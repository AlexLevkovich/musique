/* $BEGIN_LICENSE

This file is part of Musique.
Copyright 2013, Flavio Tordini <flavio.tordini@gmail.com>

Musique is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Musique is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Musique.  If not, see <http://www.gnu.org/licenses/>.

$END_LICENSE */

#include "updatechecker.h"
#include "networkaccess.h"
#include "constants.h"
#ifdef APP_ACTIVATION
#include "activation.h"
#endif

namespace The {
    NetworkAccess* http();
}

UpdateChecker::UpdateChecker() {
    m_needUpdate = false;
}

void UpdateChecker::checkForUpdate() {
    QUrl url(QString(Constants::WEBSITE) + "-ws/release.xml");
    QUrlQuery q;
    q.addQueryItem("v", Constants::VERSION);

#ifdef APP_MAC
    q.addQueryItem("os", "mac");
#endif
#ifdef APP_WIN
    q.addQueryItem("os", "win");
#endif
#ifdef APP_ACTIVATION
    QString t = "demo";
    if (Activation::instance().isActivated()) t = "active";
    q.addQueryItem("t", t);
#endif
#ifdef APP_MAC_STORE
    q.addQueryItem("store", "mac");
#endif
    url.setQuery(q);

    QObject *reply = The::http()->get(url);
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
