#ifndef MPVAUDIOOUTPUT_H
#define MPVAUDIOOUTPUT_H

#include <QObject>
#include "libmpv_def.h"
#include "mpvmediaobject.h"

namespace Libmpv {

class AudioOutput: public QObject {
    Q_OBJECT
public:
    AudioOutput(QObject * parent = NULL);
    AudioOutput(MediaObject * mo,QObject * parent = NULL);

    bool isMuted() const;
    MediaObject * outputDevice() const;
    qreal volume() const;

public slots:
    void setMuted(bool mute);
    void setMediaObject(MediaObject * mo);
    void setVolume(qreal newVolume);

signals:
    void mutedChanged(bool muted);
    void volumeChanged(qreal newVolume);
    void stateChanged(bool enabled);

private slots:
    void on_volumeChanged(qint64 value);
    void on_stateChanged(Libmpv::State);

private:
    MediaObject * m_mo;
};

}

#endif // MPVAUDIOOUTPUT_H
