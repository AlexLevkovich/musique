#include "updatechecker.h"
#include "networkaccess.h"
#include "constants.h"

namespace The {
    NetworkAccess* http();
}

UpdateChecker::UpdateChecker() {
    m_needUpdate = false;
}

void UpdateChecker::checkForUpdate() {
    QUrl updateUrl(QString(Constants::WEBSITE) + "-ws/release.xml");
    updateUrl.addQueryItem("v", Constants::VERSION);

#ifdef APP_MAC
    updateUrl.addQueryItem("os", "mac");
#endif
#ifdef APP_WIN
    updateUrl.addQueryItem("os", "win");
#endif
#ifdef APP_DEMO
    updateUrl.addQueryItem("t", "demo");
#endif
#ifdef APP_MAC_STORE
    updateUrl.addQueryItem("store", "mac");
#endif


    QObject *reply = The::http()->get(updateUrl);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(requestFinished(QByteArray)));
}

void UpdateChecker::requestFinished(QByteArray data) {
        UpdateCheckerStreamReader reader;
        reader.read(data);
        m_needUpdate = reader.needUpdate();
        m_remoteVersion = reader.remoteVersion();
        if (m_needUpdate) emit newVersion(m_remoteVersion);
}

QString UpdateChecker::remoteVersion() {
    return m_remoteVersion;
}

// --- Reader ---

bool UpdateCheckerStreamReader::read(QByteArray data) {
    addData(data);

    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == "release") {
                while (!atEnd()) {
                    readNext();
                    if (isStartElement() && name() == "version") {
                        QString remoteVersion = readElementText();
                        // qDebug() << remoteVersion << QString(Constants::VERSION);
                        m_needUpdate = remoteVersion != QString(Constants::VERSION);
                        m_remoteVersion = remoteVersion;
                        break;
                    }
                }
            }
        }
    }

    return !error();
}

QString UpdateCheckerStreamReader::remoteVersion() {
    return m_remoteVersion;
}
