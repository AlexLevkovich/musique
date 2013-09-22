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

#include "searchcompletion.h"
#include "networkaccess.h"

#define GSUGGEST_URL "http://suggestqueries.google.com/complete/search?ds=yt&output=toolbar&hl=%1&q=%2"

namespace The {
    NetworkAccess* http();
}

SearchCompletion::SearchCompletion(QWidget *parent, QLineEdit *editor):
        QObject(parent), buddy(parent), editor(editor) {

    enabled = true;

    popup = new QListWidget;
    popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    popup->installEventFilter(this);
    popup->setMouseTracking(true);

    connect(popup, SIGNAL(itemClicked(QListWidgetItem*)),
            SLOT(doneCompletion()));

    // connect(popup, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
    //    SLOT(currentItemChanged(QListWidgetItem *)));

    // mouse hover
    // connect(popup, SIGNAL(itemEntered(QListWidgetItem*)),
    //    SLOT(currentItemChanged(QListWidgetItem *)));

    popup->setWindowFlags(Qt::Popup);
    popup->setFocusPolicy(Qt::NoFocus);
    popup->setFocusProxy(parent);

    timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(300);
    connect(timer, SIGNAL(timeout()), SLOT(autoSuggest()));
    connect(editor, SIGNAL(textEdited(QString)), timer, SLOT(start()));

}

SearchCompletion::~SearchCompletion() {
    delete popup;
}

bool SearchCompletion::eventFilter(QObject *obj, QEvent *ev) {
    if (obj != popup)
        return false;

    if (ev->type() == QEvent::MouseButtonPress) {
        popup->hide();
        editor->setFocus();
        editor->setText(originalText);
        return true;
    }

    if (ev->type() == QEvent::KeyPress) {

        bool consumed = false;

        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(ev);
        int key = keyEvent->key();
        // qDebug() << keyEvent->text();
        switch (key) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            if (popup->currentItem()) {
                doneCompletion();
                consumed = true;
            } else {
                editor->setFocus();
                editor->event(ev);
                popup->hide();
            }
            break;

        case Qt::Key_Escape:
            editor->setFocus();
            editor->setText(originalText);
            popup->hide();
            consumed = true;
            break;

        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Home:
        case Qt::Key_End:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
            break;

        default:
            qDebug() << keyEvent->text();
            editor->setFocus();
            editor->event(ev);
            popup->hide();
            break;
        }

        return consumed;
    }

    return false;
}

void SearchCompletion::showCompletion(const QStringList &choices) {

    if (choices.isEmpty())
        return;

    popup->setUpdatesEnabled(false);
    popup->clear();
    for (int i = 0; i < choices.count(); ++i) {
        QListWidgetItem * item;
        item = new QListWidgetItem(popup);
        item->setText(choices[i]);
    }
    popup->setCurrentItem(0);
    popup->adjustSize();
    popup->setUpdatesEnabled(true);

    int h = popup->sizeHintForRow(0) * choices.count() + 4;
    popup->resize(buddy->width(), h);

    popup->move(buddy->mapToGlobal(QPoint(0, buddy->height())));

    popup->setFocus();
    popup->show();
}

void SearchCompletion::doneCompletion() {
    timer->stop();
    popup->hide();
    editor->setFocus();
    QListWidgetItem *item = popup->currentItem();
    if (item) {
        editor->setText(item->text());
        QKeyEvent *e;
        e = new QKeyEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
        QApplication::postEvent(editor, e);
        e = new QKeyEvent(QEvent::KeyRelease, Qt::Key_Enter, Qt::NoModifier);
        QApplication::postEvent(editor, e);
    }
}

void SearchCompletion::preventSuggest() {
    timer->stop();
    enabled = false;
    popup->hide();
}

void SearchCompletion::enableSuggest() {
    enabled = true;
}

void SearchCompletion::autoSuggest() {
    if (!enabled) return;

    QString query = editor->text();
    originalText = query;
    // qDebug() << "originalText" << originalText;
    if (query.isEmpty()) return;

    QString locale = QLocale::system().name().replace("_", "-");
    // case for system locales such as "C"
    if (locale.length() < 2) {
        locale = "en-US";
    }

    QString url = QString(GSUGGEST_URL).arg(locale, query);

    QObject *reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(handleNetworkData(QByteArray)));
}

void SearchCompletion::handleNetworkData(QByteArray response) {
    if (!enabled) return;

    QStringList choices;

    QXmlStreamReader xml(response);
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.tokenType() == QXmlStreamReader::StartElement)
            if (xml.name() == "suggestion") {
            QStringRef str = xml.attributes().value("data");
            choices << str.toString();
        }
    }

    showCompletion(choices);

}

void SearchCompletion::currentItemChanged(QListWidgetItem *current) {
    if (current) {
        // qDebug() << "current" << current->text();
        current->setSelected(true);
        editor->setText(current->text());
        editor->setSelection(originalText.length(), editor->text().length());
    }
}
