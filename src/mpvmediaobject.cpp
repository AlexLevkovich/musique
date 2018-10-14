#include "mpvmediaobject.h"
#include <QCoreApplication>
#include <QDebug>

namespace MPV {
    static const int ABOUT_TO_FINISH_TIME = 2000;
    static const int TICK_INTERVAL        = 100;
    static const int MAX_VOLUME           = 150;
}

static void wakeup(void *ctx) {
    Libmpv::MediaObject *engine = (Libmpv::MediaObject*) ctx;
    QCoreApplication::postEvent(engine, new QEvent(QEvent::User));
}

namespace Libmpv {

MediaObject::MediaObject(const QString & client_name,QObject * parent): QObject(parent) {
    m_state = ErrorState;
    m_errorType = FatalError;

#ifndef _WIN32
    setlocale(LC_NUMERIC, "C");
#endif

    m_mpv_core = mpv_create();
    if(!m_mpv_core) {
        m_errorString = tr("Cannot create MPV engine!!!");
        return;
    }

    m_client_name = client_name;

    mpv_set_option_string(m_mpv_core, "config", "no");
    mpv_set_option_string(m_mpv_core, "audio-display", "no");          /* do not show image for audio tracks */
    mpv_set_option_string(m_mpv_core, "gapless-audio", "yes") ;        /* force gapless playback */
    mpv_set_option_string(m_mpv_core, "vo", "null");                   /* disable video */
    mpv_set_option_string(m_mpv_core, "idle", "yes");                  /* mpv wait idly instead of quitting */
    mpv_set_option_string(m_mpv_core, "input-default-bindings", "no"); /* no key binding */
    mpv_set_option_string(m_mpv_core, "input-vo-keyboard", "no");      /* no keyboard inputs */
    mpv_set_option_string(m_mpv_core, "input-cursor", "no");           /* no mouse handling */
    mpv_set_option_string(m_mpv_core, "ytdl", "no");                   /* no youtube-dl */
    mpv_set_option_string(m_mpv_core, "fs", "no");                     /* no fullscreen */
    mpv_set_option_string(m_mpv_core, "osd-level", "0");               /* no OSD */
    mpv_set_option_string(m_mpv_core, "quiet", "yes");                 /* console output less verbose */
    mpv_set_option_string(m_mpv_core, "softvol", "yes");               /* use mpv internal vol */
    mpv_set_option_string(m_mpv_core, "softvol-max", (QString::number(MPV::MAX_VOLUME) + ".0").toLatin1().constData());         /* max vol */
    mpv_set_option_string(m_mpv_core, "audio-client-name",client_name.toUtf8().constData());
    mpv_request_log_messages(m_mpv_core, "info");
    mpv_set_wakeup_callback(m_mpv_core, wakeup, this);

    m_tickInterval = MPV::TICK_INTERVAL;

    /* ----- core mpv init ----- */
    if (!_check_set_error(mpv_initialize(m_mpv_core),true)) return;

    /* ----- get updates when these properties change ----- */
    if (!_check_set_error(mpv_observe_property(m_mpv_core, 1, "pause",MPV_FORMAT_FLAG),true)) return;


    double volume_in_percent = 100;
    mpv_set_property_async(m_mpv_core, 0, "volume", MPV_FORMAT_DOUBLE, &volume_in_percent);
    m_internal_volume = 100;

    m_internal_is_mute  = false;
    int m = 0;
    mpv_set_property_async(m_mpv_core, 0, "mute", MPV_FORMAT_FLAG, &m);

    m_errorType = NoError;
    m_state = LoadingState;
    m_finishEmitted = true;
    m_lastTick = 0;
    m_totalTime = 0;
    m_currMediaIndex = -1;
    m_doCompleteStop = false;
    m_aboutFinishEmitted = false;
}

MediaObject::~MediaObject() {
    switch (state()) {
        case PlayingState:
        case PausedState:
            stop();
            break;
        case ErrorState:
        case StoppedState:
        case LoadingState:
            break;
    }

    if(m_mpv_core) {
        mpv_terminate_destroy(m_mpv_core);
        m_mpv_core = NULL;
    }
}

bool MediaObject::_check_set_error(int status,bool is_fatal) {
    if (status >= 0) return true;

    m_errorString = QString::fromUtf8(mpv_error_string(status));
    m_errorType = is_fatal?FatalError:NormalError;
    State old_state = m_state;
    m_state = ErrorState;
    emit stateChanged(m_state,old_state);

    return false;
}

State MediaObject::state() const {
    return m_state;
}

void MediaObject::setCurrentSource(const MediaSource & source) {
    clear();
    m_currMediaIndex = 0;
    _enqueue(source,true);
}

void MediaObject::setQueue(const QList<MediaSource> & sources) {
    m_currMediaIndex = -1;
    m_mediaSources.clear();

    m_currMediaIndex = 0;
    _enqueue(sources,true);
}

void MediaObject::setQueue(const QList<QUrl> & urls) {
    m_currMediaIndex = -1;
    m_mediaSources.clear();

    m_currMediaIndex = 0;
    _enqueue(urls,true);
}

void MediaObject::setQueue(const QStringList & names) {
    m_currMediaIndex = -1;
    m_mediaSources.clear();

    m_currMediaIndex = 0;
    _enqueue(names,true);
}

void MediaObject::enqueue(const MediaSource & source) {
    _enqueue(source,false);
}

void MediaObject::_enqueue(const QList<MediaSource> & sources,bool doStop) {
    for (int i=0;i<sources.count();i++) {
        _enqueue(sources.at(i),doStop);
    }
}

void MediaObject::_enqueue(const QList<QUrl> & urls,bool doStop) {
    QList<MediaSource> sources;
    for (int i=0;i<urls.count();i++) {
        sources.append(MediaSource(urls.at(i)));
    }

    _enqueue(sources,doStop);
}

void MediaObject::_enqueue(const QStringList & names,bool doStop) {
    QList<MediaSource> sources;
    for (int i=0;i<names.count();i++) {
        sources.append(MediaSource(names.at(i)));
    }

    _enqueue(sources,doStop);
}

void MediaObject::enqueue(const QList<MediaSource> & sources) {
    _enqueue(sources,false);
}

void MediaObject::enqueue(const QList<QUrl> & urls) {
    _enqueue(urls,false);
}

void MediaObject::enqueue(const QStringList & names) {
    _enqueue(names,false);
}

void MediaObject::_enqueue(const MediaSource & source,bool doStop) {
    if (doStop && state() == PlayingState) m_doCompleteStop = true;

    m_mediaSources.append(source);

    if (state() == LoadingState) {
        State old_state = m_state;
        m_state = StoppedState;
        emit stateChanged(m_state,old_state);
    }
}

void MediaObject::play() {
    MediaSource source = currentSource();
    if(m_mpv_core != NULL && (state() == StoppedState || state() == ErrorState) && source.isValid()) {
        const QByteArray path_ba = ((source.type() == MediaSource::LocalFile)?source.fileName():source.url().toString()).toUtf8();
        const char *cmd[] = {"loadfile", path_ba.constData(), "replace", NULL};

        mpv_unobserve_property(m_mpv_core, 2);
        mpv_unobserve_property(m_mpv_core, 3);
        mpv_unobserve_property(m_mpv_core, 4);
        mpv_unobserve_property(m_mpv_core, 5);

        if (!_check_set_error(mpv_command(m_mpv_core, cmd))) return;

        mpv_observe_property(m_mpv_core, 2, "time-pos", MPV_FORMAT_DOUBLE);
        mpv_observe_property(m_mpv_core, 3, "duration", MPV_FORMAT_DOUBLE);
        mpv_observe_property(m_mpv_core, 4, "metadata", MPV_FORMAT_NODE);
        mpv_observe_property(m_mpv_core, 5, "volume",   MPV_FORMAT_NODE);
    }

    if(m_mpv_core != NULL && (state() == PausedState || (source.isValid() && (state() == StoppedState || state() == ErrorState)))) {
        int f = 0;
        mpv_set_property_async(m_mpv_core, 0, "pause", MPV_FORMAT_FLAG, &f);
        if (state() == PausedState) {
            State old_state = m_state;
            m_state = PlayingState;
            emit stateChanged(m_state,old_state);
        }
    }
}

void MediaObject::clear() {
    stop();
    m_currMediaIndex = -1;
    m_mediaSources.clear();
    State old_state = m_state;
    m_state = LoadingState;
    emit stateChanged(m_state,old_state);
    m_lastTick = 0;
}

void MediaObject::pause() {
    if(m_mpv_core != NULL && state() == PlayingState) {
        int f = 1;
        mpv_set_property_async(m_mpv_core, 0, "pause", MPV_FORMAT_FLAG, &f);
        State old_state = m_state;
        m_state = PausedState;
        emit stateChanged(m_state,old_state);
    }
}

void MediaObject::seek(qint64 time) {
    if (!isSeekable()) return;

    const qint64 seconds=time/1000;
    const QByteArray tmp = QString::number(seconds).toUtf8();

    const char *cmd[] = {"seek", tmp.constData(), "absolute", NULL};
    if (!_check_set_error(mpv_command(m_mpv_core, cmd))) return;
}

void MediaObject::setTickInterval(qint32 newTickInterval) {
    if (state() != PlayingState && state() != PausedState) {
        m_tickInterval = newTickInterval;
    }
}

void MediaObject::stop() {
    if(m_mpv_core != NULL && (state() == PlayingState || state() == PausedState)) {
        mpv_unobserve_property(m_mpv_core, 2);
        mpv_unobserve_property(m_mpv_core, 3);
        mpv_unobserve_property(m_mpv_core, 4);
        mpv_unobserve_property(m_mpv_core, 5);

        m_doCompleteStop = false;
        m_finishEmitted = false;
        m_aboutFinishEmitted = false;
        const char *cmd[] = {"stop", NULL};
        if (!_check_set_error(mpv_command(m_mpv_core, cmd))) return;
        State old_state = m_state;
        m_state = (m_mediaSources.count() > 0)?StoppedState:LoadingState;
        emit stateChanged(m_state,old_state);
    }
}

qint64 MediaObject::currentTime() const {
    return m_lastTick;
}

qint64 MediaObject::totalTime() const {
    return m_totalTime;
}

void MediaObject::clearQueue() {
    m_currMediaIndex = -1;
    m_mediaSources.clear();
}

MediaSource MediaObject::currentSource() const {
    if (m_currMediaIndex >= 0 && m_currMediaIndex < m_mediaSources.count()) {
        return m_mediaSources.at(m_currMediaIndex);
    }

    return MediaSource();
}

QString MediaObject::errorString() const {
    return m_errorString;
}

ErrorType MediaObject::errorType() const {
    return m_errorType;
}

bool MediaObject::isSeekable() const {
    return (m_mpv_core != NULL && m_totalTime > 0);
}

QList<MediaSource> MediaObject::queue() const {
    return m_mediaSources;
}

qint64 MediaObject::remainingTime() const {
    return totalTime() - currentTime();
}

qint32 MediaObject::tickInterval() const {
    return m_tickInterval;
}

bool MediaObject::event(QEvent* event) {
    if(event->type() == QEvent::User) {
        while(m_mpv_core) {
            mpv_event *m_event = mpv_wait_event(m_mpv_core, 0);
            if(!m_event || m_event->event_id == MPV_EVENT_NONE) break;

            if (m_event->event_id == MPV_EVENT_PROPERTY_CHANGE) {
                handle_mpv_property_change(m_event);
            }
            else if (m_event->event_id == MPV_EVENT_START_FILE) {
                m_totalTime = 0;
                m_lastTick  = 0;
                emit currentSourceChanged(m_mediaSources.at(m_currMediaIndex));
            }
            else if (m_event->event_id == MPV_EVENT_FILE_LOADED) {
                update_total_time();
            }
            else if (m_event->event_id == MPV_EVENT_END_FILE) {
#if (MPV_CLIENT_API_VERSION >= MPV_MAKE_VERSION(1,9))
                struct mpv_event_end_file *eof_event = (struct mpv_event_end_file*)m_event->data;
                if( eof_event->reason ==  MPV_END_FILE_REASON_EOF) on_media_finished();
                else if (eof_event->reason ==  MPV_END_FILE_REASON_ERROR) on_media_error(m_event->error);
#else
                on_media_finished();
#endif
            }
        }
        return true;
    }
    return QObject::event(event);
}

void MediaObject::on_media_finished() {
    if (m_mediaSources.isEmpty()) m_doCompleteStop = true;

    if (m_finishEmitted) emit finished();
    else m_finishEmitted = true;
    State prev_state = m_state;
    m_state = (m_mediaSources.count() > 0)?StoppedState:LoadingState;
    emit stateChanged(m_state,prev_state);
    m_aboutFinishEmitted = false;

    if ((m_currMediaIndex+1) < m_mediaSources.count() && !m_doCompleteStop) {
        m_currMediaIndex++;
        play();
    }
    m_doCompleteStop = false;
}

void MediaObject::on_media_error(int code) {
      _check_set_error(code);
      m_aboutFinishEmitted = false;
}

void MediaObject::handle_mpv_property_change(mpv_event *event) {
    mpv_event_property *prop = (mpv_event_property*)event->data;

    /* on time change */
    if(QString(prop->name) == "time-pos") {
       if(prop->format == MPV_FORMAT_DOUBLE) {
         qint64 time = (qint64)((*(double*)prop->data) * 1000); /* sec to mili */
         if (time + m_tickInterval >= m_lastTick || time - m_tickInterval <= m_lastTick) {
             m_lastTick = time;
             emit tick(time);
             if (m_totalTime > 0 && m_lastTick >= m_totalTime - MPV::ABOUT_TO_FINISH_TIME)
                 if (!m_aboutFinishEmitted) {
                     emit aboutToFinish();
                     m_aboutFinishEmitted = true;
                 }
         }
       }
    }
    else if(QString(prop->name) == "volume") {
        m_internal_volume = (qint64)(*((double *)prop->data));
        emit volumeChanged(m_internal_volume);
    }
    else if (QString(prop->name) == "metadata") {
        mpv_node node;
        mpv_get_property(m_mpv_core,"metadata",MPV_FORMAT_NODE,&node);
        if(node.format == MPV_FORMAT_NODE_MAP) {
            QString title,artist;
            for(int n = 0; n < node.u.list->num; n++) {
                if(node.u.list->values[n].format == MPV_FORMAT_STRING) {
                    QString s_key   =  node.u.list->keys[n];
                    s_key = s_key.toLower();
                    QString s_value =  node.u.list->values[n].u.string;
                    if (s_key == "artist") {
                        artist = s_value;
                    }
                    else if (s_key == "title") {
                        title = s_value;
                    }
                }
            }
            if (artist.isEmpty() && !m_client_name.isEmpty()) artist = m_client_name;
            if (artist.isEmpty()) artist = "libMPV";
            mpv_set_option_string(m_mpv_core,"title",(title.isEmpty()?artist:(artist + " - " + title)).toUtf8().constData());
        }
    }
}

void MediaObject::update_total_time() {
    double len;
    mpv_get_property(m_mpv_core, "duration", MPV_FORMAT_DOUBLE, &len);

    m_totalTime = ((qint64)len)*1000;

    State old_state = m_state;
    m_state = PlayingState;
    emit stateChanged(m_state,old_state);
    emit seekableChanged(isSeekable());
    emit totalTimeChanged(m_totalTime);
}

qint64 MediaObject::volume() const {
    return m_internal_volume;
}

void MediaObject::setVolume(qint64 value) {
    if(m_mpv_core != NULL && m_internal_volume != value && (state() == PlayingState || state() == PausedState)) {
        if (value > MPV::MAX_VOLUME) value = MPV::MAX_VOLUME;
        if (value < 0) value = 0;
        m_internal_volume   = value;

        double volume_in_percent = (double)m_internal_volume;
        mpv_set_property_async(m_mpv_core, 0, "volume", MPV_FORMAT_DOUBLE, &volume_in_percent);
    }
}

bool MediaObject::isMuted() const {
    return m_internal_is_mute;
}

void MediaObject::setMuted(bool mute) {
    if(m_mpv_core != NULL && m_internal_is_mute != mute && (state() == PlayingState || state() == PausedState)) {
      m_internal_is_mute = mute;

      int m = m_internal_is_mute ? 1 : 0;
      mpv_set_property_async(m_mpv_core, 0, "mute", MPV_FORMAT_FLAG, &m);

      emit muteStateChanged(m_internal_is_mute);
    }
}

}
