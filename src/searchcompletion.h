#ifndef GOOGLESUGGEST_H
#define GOOGLESUGGEST_H

#include <QtGui>

class SearchCompletion : public QObject {
    Q_OBJECT

public:
    SearchCompletion(QWidget *parent, QLineEdit *editor);
    ~SearchCompletion();
    bool eventFilter(QObject *obj, QEvent *ev);
    void showCompletion(const QStringList &choices);

public slots:
    void doneCompletion();
    void preventSuggest();
    void enableSuggest();
    void autoSuggest();
    void handleNetworkData(QByteArray response);
    void currentItemChanged(QListWidgetItem *current);

private:
    QWidget *buddy;
    QLineEdit *editor;
    QString originalText;
    QListWidget *popup;
    QTimer *timer;
    bool enabled;

};

#endif // GOOGLESUGGEST_H
