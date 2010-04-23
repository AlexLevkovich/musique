#ifndef NETWORKACCESS_H
#define NETWORKACCESS_H

#include <QtNetwork>

namespace The {
    QNetworkAccessManager* networkAccessManager();
}

class NetworkReply : public QObject {

    Q_OBJECT

public:
    NetworkReply(QNetworkReply* networkReply);
    bool followRedirects;

public slots:
    void finished();
    void requestError(QNetworkReply::NetworkError);

signals:
    void data(QByteArray);
    void error(QNetworkReply*);
    void finished(QNetworkReply*);

private:
    QNetworkReply *networkReply;

};


class NetworkAccess : public QObject {

    Q_OBJECT

public:
    NetworkAccess( QObject* parent=0);
    QNetworkReply* simpleGet(QUrl url, int operation = QNetworkAccessManager::GetOperation);
    NetworkReply* get(QUrl url);
    NetworkReply* head(QUrl url);
    QNetworkReply* syncGet(QUrl url);
    QByteArray syncGetBytes(QUrl url);
    QString syncGetString(QUrl url);

private slots:
    void error(QNetworkReply::NetworkError);
    void syncMetaDataChanged();
    void syncFinished();

private:
    QNetworkReply *networkReply;
    bool working;

};

typedef QPointer<QObject> ObjectPointer;
Q_DECLARE_METATYPE(ObjectPointer)

#endif // NETWORKACCESS_H
