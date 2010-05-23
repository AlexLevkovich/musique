#ifndef GLOBAL_H
#define GLOBAL_H

#include <QtGui>
#include <QNetworkAccessManager>
#include "networkaccess.h"

namespace The {

    static QMap<QString, QAction*> *g_actions = 0;

    QMap<QString, QAction*>* globalActions() {
        if (!g_actions)
            g_actions = new QMap<QString, QAction*>;
        return g_actions;
    }

    static QMap<QString, QMenu*> *g_menus = 0;

    QMap<QString, QMenu*>* globalMenus() {
        if (!g_menus)
            g_menus = new QMap<QString, QMenu*>;
        return g_menus;
    }

    static QNetworkAccessManager *nam = 0;

    QNetworkAccessManager* networkAccessManager() {
        if (!nam) {
            nam = new QNetworkAccessManager();

            /*
            // A simple disk based cache
            QNetworkDiskCache *cache = new QNetworkDiskCache();
            QString cacheLocation = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
            qDebug() << cacheLocation;
            cache->setCacheDirectory(cacheLocation);
            nam->setCache(cache);
            */

        }
        return nam;
    }

    // static NetworkAccess *g_http = 0;
    // key is thread itself
    QHash<QThread*, NetworkAccess *> g_http;

    NetworkAccess* http() {
        // const QString threadName = QThread::currentThread()->objectName();
        // qDebug() << "threadName" << threadName;
        if (g_http.contains(QThread::currentThread())) {
            return g_http.value(QThread::currentThread());
        } else {
            // qDebug() << "NetworkAccess for thread" << QThread::currentThread();
            NetworkAccess *http = new NetworkAccess();
            g_http.insert(QThread::currentThread(), http);
            return http;
        }
    }

}

#endif // GLOBAL_H
