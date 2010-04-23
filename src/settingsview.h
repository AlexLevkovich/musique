#ifndef SETTINGSVIEW_H
#define SETTINGSVIEW_H

#include <QtGui>
#include "view.h"

class SettingsView : public QWidget, public View {

    Q_OBJECT

public:
    SettingsView(QWidget *parent);
    void appear() {}
    void disappear() {}
    QMap<QString, QVariant> metadata() {
        QMap<QString, QVariant> metadata;
        metadata.insert("title", tr("Preferences"));
        metadata.insert("description", tr(""));
        return metadata;
    }

private slots:
    void storeSettings();

private:

};
#endif
