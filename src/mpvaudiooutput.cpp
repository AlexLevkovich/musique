#include "mpvaudiooutput.h"
#include <math.h>

static const qreal log10over20 = qreal(0.1151292546497022842);
static const qreal LOUDNESS_TO_VOLTAGE_EXPONENT = qreal(0.67);
static const qreal VOLTAGE_TO_LOUDNESS_EXPONENT = qreal(1.0/LOUDNESS_TO_VOLTAGE_EXPONENT);

namespace Libmpv {

AudioOutput::AudioOutput(QObject * parent): QObject(parent) {
    m_mo = NULL;
}

AudioOutput::AudioOutput(MediaObject * mo,QObject * parent): QObject(parent)  {
    m_mo = NULL;
    setMediaObject(mo);
}

void AudioOutput::setMediaObject(MediaObject * mo) {
    if (mo == NULL || mo == m_mo) return;

    if (m_mo != NULL) disconnect(m_mo,0,this,0);
    m_mo = mo;

    connect(m_mo,SIGNAL(volumeChanged(qint64)),this,SLOT(on_volumeChanged(qint64)));
    connect(m_mo,SIGNAL(muteStateChanged(bool)),this,SIGNAL(mutedChanged(bool)));
    connect(m_mo,SIGNAL(stateChanged(Libmpv::State,Libmpv::State)),this,SLOT(on_stateChanged(Libmpv::State)));
}

void AudioOutput::on_volumeChanged(qint64 value) {
    emit volumeChanged((qreal)value);
}

qreal AudioOutput::volume() const {
    if (m_mo == NULL) return 0;
    return m_mo->volume();
}

bool AudioOutput::isMuted() const {
    if (m_mo == NULL) return true;
    return m_mo->isMuted();
}

void AudioOutput::setMuted(bool mute) {
    if (m_mo == NULL) return;
    return m_mo->setMuted(mute);
}

void AudioOutput::setVolume(qreal newVolume) {
    if (m_mo == NULL) return;
    return m_mo->setVolume(newVolume);
}

void AudioOutput::on_stateChanged(Libmpv::State state) {
    emit stateChanged(state == PlayingState || state == PausedState);
}

}
