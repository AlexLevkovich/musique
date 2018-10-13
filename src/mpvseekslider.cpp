#include "mpvseekslider.h"
#include <QMouseEvent>
#include <QApplication>
#include <QDebug>

namespace Libmpv {

SeekSlider::SeekSlider(QWidget * parent):QWidget(parent),
                             layout(QBoxLayout::LeftToRight, this),
                             slider(Qt::Horizontal, parent),
                             ticking(false) {
    init();
}

SeekSlider::SeekSlider(MediaObject * mo,QWidget * parent):QWidget(parent),
                             layout(QBoxLayout::LeftToRight, this),
                             slider(Qt::Horizontal, parent),
                             ticking(false) {
    init();
    setMediaObject(mo);
}

void SeekSlider::init() {
    slider.setPageStep(5000); // 5 sec
    slider.setSingleStep(500); // 0.5 sec
    layout.setMargin(0);
    layout.setSpacing(2);
    layout.addWidget(&slider,0,Qt::AlignVCenter);
    setEnabled(false);

    connect(&slider,SIGNAL(valueChanged(int)),SLOT(on_seek(int)));
}

void SeekSlider::setMediaObject(MediaObject * media) {
    if (media != NULL) disconnect(media,0,this,0);

    this->media = media;
    if (media) {
        connect(media,SIGNAL(stateChanged(Libmpv::State,Libmpv::State)),SLOT(on_stateChanged(Libmpv::State)));
        connect(media,SIGNAL(totalTimeChanged(qint64)),SLOT(on_length(qint64)));
        connect(media,SIGNAL(tick(qint64)),SLOT(on_tick(qint64)));
        connect(media,SIGNAL(seekableChanged(bool)),SLOT(on_seekableChanged(bool)));
        connect(media,SIGNAL(currentSourceChanged(const Libmpv::MediaSource&)),SLOT(on_currentSourceChanged()));
        on_stateChanged(media->state());
        on_seekableChanged(media->isSeekable());
        on_length(media->totalTime());
    } else {
        on_stateChanged(StoppedState);
        on_seekableChanged(false);
    }
}

void SeekSlider::on_currentSourceChanged() {
    QMouseEvent event(QEvent::MouseButtonRelease,QPoint(),Qt::LeftButton,0,0);
    QApplication::sendEvent(&slider,&event);
}

void SeekSlider::on_seekableChanged(bool isSeekable) {
    if (media == NULL || !isSeekable) {
        setEnabled(false);
    }
    else {
        switch (media->state()) {
        case PlayingState:
            if (media->tickInterval() == 0) {
                media->setTickInterval(350);
            }
        case PausedState:
            setEnabled(true);
            break;
        case StoppedState:
        case LoadingState:
        case ErrorState:
            setEnabled(false);
            ticking = true;
            slider.setValue(0);
            ticking = false;
            break;
        }
    }
}

void SeekSlider::on_stateChanged(Libmpv::State newstate) {
    if (media == NULL || !media->isSeekable()) {
        setEnabled(false);
        return;
    }
    switch (newstate) {
    case PlayingState:
        if (media->tickInterval() == 0) {
            media->setTickInterval(350);
        }
    case PausedState:
        setEnabled(true);
        break;
    case StoppedState:
    case LoadingState:
    case ErrorState:
        setEnabled(false);
        ticking = true;
        slider.setValue(0);
        ticking = false;
        break;
    }
}

void SeekSlider::on_length(qint64 msec) {
    ticking = true;
    slider.setRange(0, msec);
    ticking = false;
}

void SeekSlider::on_tick(qint64 msec) {
    ticking = true;
    slider.setValue(msec);
    ticking = false;
}

void SeekSlider::on_seek(int msec) {
    if (!ticking && media) {
        media->seek(msec);
    }
}

bool SeekSlider::hasTracking() const {
    return slider.hasTracking();
}

void SeekSlider::setTracking(bool tracking) {
    slider.setTracking(tracking);
}

int SeekSlider::pageStep() const {
    return slider.pageStep();
}

void SeekSlider::setPageStep(int milliseconds) {
    slider.setPageStep(milliseconds);
}

int SeekSlider::singleStep() const {
    return slider.singleStep();
}

void SeekSlider::setSingleStep(int milliseconds) {
    slider.setSingleStep(milliseconds);
}

Qt::Orientation SeekSlider::orientation() const {
    return slider.orientation();
}

void SeekSlider::setOrientation(Qt::Orientation o) {
    Qt::Alignment align = (o == Qt::Horizontal ? Qt::AlignVCenter : Qt::AlignHCenter);
    layout.setAlignment(&slider, align);
    layout.setDirection(o == Qt::Horizontal ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom);
    slider.setOrientation(o);
}

}
