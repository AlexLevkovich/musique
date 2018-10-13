#ifndef MPVMEDIAOBJECT_H
#define MPVMEDIAOBJECT_H

#include <QObject>
#include <QStringList>
#include <QMultiMap>
#include "libmpv_def.h"
#include "mpvmediasource.h"
#include "mpv/client.h"

namespace Libmpv {

class MediaObject: public QObject {
    Q_OBJECT
public:
    MediaObject(const QString & client_name = QString(),QObject * parent = NULL);
    ~MediaObject();
    void clearQueue();
    MediaSource currentSource() const;
    qint64 currentTime() const;
    void enqueue(const MediaSource & source);
    void enqueue(const QList<MediaSource> & sources);
    void enqueue(const QList<QUrl> & urls);
    void enqueue(const QStringList & names);
    QString errorString() const;
    ErrorType errorType() const;
    bool isSeekable() const;
    QList<MediaSource> queue() const;
    qint64 remainingTime() const;
    void setCurrentSource(const MediaSource & source);
    void setQueue(const QList<MediaSource> & sources);
    void setQueue(const QList<QUrl> & urls);
    void setQueue(const QStringList & names);
    State state() const;
    qint32 tickInterval() const;
    qint64 totalTime() const;

    qint64 volume() const;
    void setVolume(qint64 value);
    bool isMuted() const;
    void setMuted(bool mute);

public slots:
    void clear();
    void pause();
    void play();
    void seek(qint64 time);
    void setTickInterval(qint32 newTickInterval);
    void stop();

protected:
    bool event(QEvent* event);

signals:
    void aboutToFinish();
    void currentSourceChanged(const Libmpv::MediaSource & newSource);
    void finished();
    void stateChanged(Libmpv::State newstate, Libmpv::State oldstate);
    void tick(qint64 time);
    void totalTimeChanged(qint64 newTotalTime);
    void seekableChanged(bool isSeekable);
    void volumeChanged(qint64 value);
    void muteStateChanged(bool muted);

private:
    void _enqueue(const MediaSource & source,bool doStop = true);
    void _enqueue(const QList<MediaSource> & sources,bool doStop = true);
    void _enqueue(const QList<QUrl> & urls,bool doStop = true);
    void _enqueue(const QStringList & names,bool doStop = true);
    bool _check_set_error(int status,bool is_fatal = false);
    void on_media_finished();
    void on_media_error(int code);
    void handle_mpv_property_change(mpv_event *event);
    void update_total_time();

    State m_state;
    ErrorType m_errorType;
    QString m_errorString;
    qint64 m_tickInterval;
    qint64 m_lastTick;
    qint64 m_totalTime;
    qint64 m_internal_volume;
    bool m_internal_is_mute;

    bool m_finishEmitted;
    bool m_aboutFinishEmitted;
    bool m_doCompleteStop;
    QList<MediaSource> m_mediaSources;
    int m_currMediaIndex;
    mpv_handle *m_mpv_core;
    QString m_client_name;
};

}

#endif // MPVMEDIAOBJECT_H
