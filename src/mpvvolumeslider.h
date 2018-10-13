#ifndef MPVVOLUMESLIDER_H
#define MPVVOLUMESLIDER_H

#include <QWidget>
#include <QPointer>
#include <QBoxLayout>
#include <QSlider>
#include <QToolButton>
#include <QIcon>
#include "libmpv_def.h"
#include "mpvaudiooutput.h"

namespace Libmpv {

class VolumeSlider: public QWidget {
    Q_OBJECT
public:
    VolumeSlider(QWidget * parent = NULL);
    VolumeSlider(AudioOutput * out,QWidget * parent = NULL);

    bool hasTracking() const;
    void setTracking(bool tracking);
    int pageStep() const;
    void setPageStep(int milliseconds);
    int singleStep() const;
    void setSingleStep(int milliseconds);
    bool isMuteVisible() const;
    QSize iconSize() const;
    qreal maximumVolume() const;
    Qt::Orientation orientation() const;
    AudioOutput *audioOutput() const;

public slots:
    void setAudioOutput(AudioOutput * output);
    void setMaximumVolume(qreal);
    void setOrientation(Qt::Orientation);
    void setMuteVisible(bool);
    void setIconSize(const QSize &size);

private slots:
    void on_sliderChanged(int);
    void on_volumeChanged(qreal);
    void on_mutedChanged(bool);
    void on_buttonClicked();
    void on_sliderPressed();
    void on_sliderReleased();

private:
    void init();

    QPointer<AudioOutput> output;
    QBoxLayout layout;
    QSlider slider;
    QToolButton muteButton;
    QIcon volumeIcon;
    QIcon mutedIcon;

    bool ignoreVolumeChangeAction;
    bool ignoreVolumeChangeObserve;
    bool sliderPressed;
};

}

#endif // MPVVOLUMESLIDER_H
