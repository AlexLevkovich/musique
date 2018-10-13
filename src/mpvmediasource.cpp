#include "mpvmediasource.h"
#include <QFileInfo>
#include <QIODevice>

namespace Libmpv {

MediaSource::MediaSource() {
    m_type = Invalid;
}

MediaSource::MediaSource(const QString & filename) {
    m_type = LocalFile;
    const QFileInfo fileInfo(filename);
    if (fileInfo.exists()) {
        if (fileInfo.isFile() && !filename.startsWith(QLatin1String(":/")) && !filename.startsWith(QLatin1String("qrc://"))) {
            m_url = QUrl::fromLocalFile(fileInfo.absoluteFilePath());
        }
    } else {
        m_url = filename;
        if (m_url.isValid()) m_type = Url;
    }
}

MediaSource::MediaSource(const QUrl & url) {
    m_type = Invalid;
    if (url.isValid()) {
        m_type = Url;
        m_url = url;
    }
}

MediaSource::MediaSource(const MediaSource & other) {
    *this = other;
}

bool MediaSource::isValid() const {
    return (m_type != Invalid);
}

QString MediaSource::fileName() const {
    return m_url.toLocalFile()
#ifdef _WIN32
          .replace('/','\\')
#endif
    ;
}

MediaSource::Type MediaSource::type() const {
    return m_type;
}

QUrl MediaSource::url() const {
    return m_url;
}

MediaSource & MediaSource::operator=(const MediaSource & other) {
    m_type = other.m_type;
    m_url = other.m_url;

    return *this;
}

bool MediaSource::operator==(const MediaSource & other) const {
    return (m_type == other.m_type && m_url == other.m_url);
}

}
