#include "mpvvolumeslider.h"

namespace Libmpv {

VolumeSlider::VolumeSlider(QWidget * parent) : QWidget(parent),
                                 layout(QBoxLayout::LeftToRight, this),
                                 slider(Qt::Horizontal, parent),
                                 muteButton(parent),
                                 volumeIcon(QIcon::fromTheme(QLatin1String("player-volume"))),
                                 mutedIcon(QIcon::fromTheme(QLatin1String("player-volume-muted"))),
                                 output(NULL),
                                 ignoreVolumeChangeAction(false),
                                 ignoreVolumeChangeObserve(true),
                                 sliderPressed(false) {
    init();
}

VolumeSlider::VolumeSlider(AudioOutput * output,QWidget * parent) : QWidget(parent),
                                layout(QBoxLayout::LeftToRight, this),
                                slider(Qt::Horizontal, parent),
                                muteButton(parent),
                                volumeIcon(QIcon::fromTheme(QLatin1String("player-volume"))),
                                mutedIcon(QIcon::fromTheme(QLatin1String("player-volume-muted"))),
                                output(NULL),
                                ignoreVolumeChangeAction(false),
                                ignoreVolumeChangeObserve(true),
                                sliderPressed(false) {
    init();
    if (output != NULL) setAudioOutput(output);
}

void VolumeSlider::init() {
    slider.setRange(0,100);
    slider.setPageStep(5);
    slider.setSingleStep(1);

    muteButton.setIcon(volumeIcon);
    muteButton.setAutoRaise(true);
    layout.setMargin(0);
    layout.setSpacing(2);
    layout.addWidget(&muteButton,0,Qt::AlignVCenter);
    layout.addWidget(&slider,0,Qt::AlignVCenter);

    slider.setEnabled(false);
    muteButton.setEnabled(false);

    if (volumeIcon.isNull()) {
        muteButton.setVisible(false);
    }

    setToolTip(tr("Volume: %1%").arg(100));
    setWhatsThis(tr("Use this slider to adjust the volume. The leftmost position is 0%, the rightmost is %1%").arg(100));

    connect(&slider,SIGNAL(valueChanged(int)),SLOT(on_sliderChanged(int)));
    connect(&slider,SIGNAL(sliderPressed()),this,SLOT(on_sliderPressed()));
    connect(&slider,SIGNAL(sliderReleased()),this,SLOT(on_sliderReleased()));
    connect(&muteButton,SIGNAL(clicked()),SLOT(on_buttonClicked()));

    setFocusProxy(&slider);
}

void VolumeSlider::setAudioOutput(AudioOutput * output) {
    if (this->output != NULL) {
        disconnect(this->output, 0, this, 0);
    }
    this->output = output;
    if (output) {
        slider.setValue((int)output->volume());

        on_volumeChanged(output->volume());
        on_mutedChanged(output->isMuted());

        connect(output,SIGNAL(volumeChanged(qreal)),SLOT(on_volumeChanged(qreal)));
        connect(output,SIGNAL(mutedChanged(bool)),SLOT(on_mutedChanged(bool)));
        connect(output,SIGNAL(stateChanged(bool)),&muteButton,SLOT(setEnabled(bool)));
        connect(output,SIGNAL(stateChanged(bool)),&slider,SLOT(setEnabled(bool)));
    } else {
        slider.setValue(100);
        slider.setEnabled(false);
        muteButton.setEnabled(false);
    }
}

bool VolumeSlider::isMuteVisible() const {
    return !muteButton.isHidden();
}

void VolumeSlider::setMuteVisible(bool visible) {
    muteButton.setVisible(visible);
}

QSize VolumeSlider::iconSize() const {
    return muteButton.iconSize();
}

void VolumeSlider::setIconSize(const QSize &iconSize) {
    muteButton.setIconSize(iconSize);
}

qreal VolumeSlider::maximumVolume() const {
    return slider.maximum();
}

void VolumeSlider::setMaximumVolume(qreal volume) {
    slider.setMaximum((int)volume);
}

Qt::Orientation VolumeSlider::orientation() const {
    return slider.orientation();
}

void VolumeSlider::setOrientation(Qt::Orientation o) {
    Qt::Alignment align = (o == Qt::Horizontal ? Qt::AlignVCenter : Qt::AlignHCenter);
    layout.setAlignment(&muteButton, align);
    layout.setAlignment(&slider, align);
    layout.setDirection(o == Qt::Horizontal ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom);
    slider.setOrientation(o);
}

AudioOutput * VolumeSlider::audioOutput() const {
    return output;
}

void VolumeSlider::on_buttonClicked() {
    if (output) {
        output->setMuted(!output->isMuted());
    }
    else {
        slider.setEnabled(false);
        muteButton.setEnabled(false);
    }
}

void VolumeSlider::on_sliderPressed() {
    sliderPressed = true;
}

void VolumeSlider::on_sliderReleased() {
    sliderPressed = false;
    if (output) {
        on_volumeChanged(output->volume());
    }
}

void VolumeSlider::on_mutedChanged(bool muted) {
    if (muted) {
        setToolTip(tr("Muted"));
        muteButton.setIcon(mutedIcon);
    }
    else {
        setToolTip(tr("Volume: %1%").arg(output->volume()));
        muteButton.setIcon(volumeIcon);
    }
}

void VolumeSlider::on_sliderChanged(int value) {
    if (output) {
        if (!output->isMuted()) {
           setToolTip(tr("Volume: %1%").arg(value));
        }

        if (!ignoreVolumeChangeObserve && output->volume() != (qreal)value) {
          ignoreVolumeChangeAction = true;
          output->setVolume(value);
        }
    }
    else {
        slider.setEnabled(false);
        muteButton.setEnabled(false);
    }

    ignoreVolumeChangeObserve = false;
}

void VolumeSlider::on_volumeChanged(qreal value) {
    if (sliderPressed) return;

    int newslidervalue = (int)value;
    if (!ignoreVolumeChangeAction && slider.value() != newslidervalue) {
        ignoreVolumeChangeObserve = true;
        slider.setValue(newslidervalue);
    }

    ignoreVolumeChangeAction = false;
}

bool VolumeSlider::hasTracking() const {
    return slider.hasTracking();
}

void VolumeSlider::setTracking(bool tracking) {
    slider.setTracking(tracking);
}

int VolumeSlider::pageStep() const {
    return slider.pageStep();
}

void VolumeSlider::setPageStep(int milliseconds) {
    slider.setPageStep(milliseconds);
}

int VolumeSlider::singleStep() const {
    return slider.singleStep();
}

void VolumeSlider::setSingleStep(int milliseconds) {
    slider.setSingleStep(milliseconds);
}

}
