#ifndef MPVMEDIASOURCE_H
#define MPVMEDIASOURCE_H

#include <QObject>
#include <QUrl>
#include <QString>
#include "libmpv_def.h"

class QIODevice;

namespace Libmpv {

class MediaSource: public QObject {
public:
    enum Type {
        Invalid = -1,
        LocalFile,
        Url
    };

    MediaSource();
    MediaSource(const QString & fileName);
    MediaSource(const QUrl & url);
    MediaSource(const MediaSource & other);

    QString fileName() const;
    Type type() const;
    QUrl url() const;

    bool isValid() const;

    MediaSource & operator=(const MediaSource & other);
    bool operator==(const MediaSource & other) const;

private:
    Type m_type;
    QUrl m_url;
};

}

#endif // MPVMEDIASOURCE_H
