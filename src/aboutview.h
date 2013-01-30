#ifndef ABOUTVIEW_H
#define ABOUTVIEW_H

#include <QtGui>
#include "view.h"
#include "constants.h"

class AboutView : public QWidget, public View {

    Q_OBJECT

public:
    AboutView(QWidget *parent);
    void appear();
    QHash<QString, QVariant> metadata() {
        QHash<QString, QVariant> metadata;
        metadata.insert("title", tr("About"));
        metadata.insert("description",
                        tr("What you always wanted to know about %1 and never dared to ask")
                        .arg(Constants::NAME));
        return metadata;
    }

protected:
    void paintEvent(QPaintEvent *);

};
#endif
