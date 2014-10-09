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
#include "autocomplete.h"
#include "suggester.h"
#ifdef APP_MAC
#include "searchlineedit_mac.h"
#else
#include "searchlineedit.h"
#endif

AutoComplete::AutoComplete(SearchLineEdit *buddy, QLineEdit *lineEdit):
    QObject(buddy), buddy(buddy), lineEdit(lineEdit), suggester(0) {

    enabled = true;

    popup = new QListWidget();
    popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    popup->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    popup->setMouseTracking(true);
    popup->setWindowFlags(Qt::Popup);
    popup->setAttribute(Qt::WA_ShowWithoutActivating);
    popup->setFocusPolicy(Qt::NoFocus);
    popup->setFocusProxy(buddy);
    popup->installEventFilter(this);

    popup->setWindowOpacity(.9);
    popup->setProperty("suggest", true);
    popup->setFrameShape(QFrame::NoFrame);
    popup->setAttribute(Qt::WA_TranslucentBackground);
    popup->viewport()->setStyleSheet("border:0; border-radius:5px; background:palette(base)");

    connect(popup, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(doneCompletion()));
    connect(popup, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
        SLOT(currentItemChanged(QListWidgetItem*)));
    connect(popup, SIGNAL(itemEntered(QListWidgetItem*)), SLOT(itemEntered(QListWidgetItem *)));

    timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(500);
    connect(timer, SIGNAL(timeout()), SLOT(autoSuggest()));
    connect(buddy, SIGNAL(textEdited(QString)), timer, SLOT(start()));
}

AutoComplete::~AutoComplete() {
    delete popup;
}

bool AutoComplete::eventFilter(QObject *obj, QEvent *ev) {
    if (obj != popup) return false;

    if (ev->type() == QEvent::Leave) {
        popup->setCurrentItem(0);
        popup->clearSelection();
        buddy->setText(originalText);
        return true;
    }

    if (ev->type() == QEvent::FocusOut) {
        popup->hide();
        buddy->setText(originalText);
        buddy->setFocus();
        return true;
    }

    if (ev->type() == QEvent::KeyPress) {
        bool consumed = false;
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(ev);
        // qWarning() << keyEvent->text();
        switch (keyEvent->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            if (popup->currentItem()) {
                doneCompletion();
                consumed = true;
            } else {
                buddy->setFocus();
                lineEdit->event(ev);
                popup->hide();
            }
            break;

        case Qt::Key_Escape:
            popup->hide();
            popup->clear();
            buddy->setText(originalText);
            buddy->setFocus();
            consumed = true;
            break;

        case Qt::Key_Up:
            if (popup->currentRow() == 0) {
                popup->setCurrentItem(0);
                popup->clearSelection();
                buddy->setText(originalText);
                buddy->setFocus();
                consumed = true;
            }
            break;

        case Qt::Key_Down:
        case Qt::Key_Home:
        case Qt::Key_End:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
            // qDebug() << key;
            break;

        default:
            // qDebug() << keyEvent->text();
            lineEdit->event(ev);
            consumed = true;
            break;
        }

        return consumed;
    }

    return false;
}

void AutoComplete::showCompletion(const QList<Suggestion *> &suggestions) {
    if (suggestions.isEmpty()) {
        popup->clear();
        popup->hide();
        return;
    }
    popup->setUpdatesEnabled(false);
    popup->clear();
    for (int i = 0; i < suggestions.count(); ++i) {
        QListWidgetItem * item;
        item = new QListWidgetItem(popup);
        Suggestion *s = suggestions[i];
        item->setText(s->value);
        if (!s->type.isEmpty())
            item->setIcon(QIcon(":/images/" + s->type + ".png"));
    }
    popup->setCurrentItem(0);
    int h = 0;
    for (int i = 0; i < suggestions.count(); ++i)
        h += popup->sizeHintForRow(i);
    popup->resize(buddy->width(), h);
    popup->move(buddy->mapToGlobal(QPoint(0, buddy->height())));
    popup->setFocus();

    if (popup->isHidden()) popup->show();
    popup->setUpdatesEnabled(true);
}

void AutoComplete::doneCompletion() {
    timer->stop();
    popup->hide();
    buddy->setFocus();
    int index = popup->currentIndex().row();
    if (index >= 0 && index < suggestions.size()) {
        Suggestion* suggestion = suggestions.at(index);
        buddy->setText(suggestion->value);
        emit suggestionAccepted(suggestion);
        popup->clear();
    } else qWarning() << "No suggestion for index" << index;
}

void AutoComplete::preventSuggest() {
    timer->stop();
    enabled = false;
    popup->hide();
    popup->setFrameShape(QFrame::NoFrame);
}

void AutoComplete::enableSuggest() {
    enabled = true;
}

void AutoComplete::setSuggester(Suggester* suggester) {
    if (this->suggester) this->suggester->disconnect();
    this->suggester = suggester;
    connect(suggester, SIGNAL(ready(QList<Suggestion*>)), SLOT(suggestionsReady(QList<Suggestion*>)));
}

void AutoComplete::autoSuggest() {
    if (!enabled) return;
    if (!buddy->hasFocus()) return;

    popup->setCurrentItem(0);
    popup->clearSelection();

    QString query = buddy->text();
    originalText = query;
    if (query.isEmpty()) {
        popup->hide();
        buddy->setFocus();
        return;
    }

    if (suggester) suggester->suggest(query);
}

void AutoComplete::suggestionsReady(const QList<Suggestion *> &suggestions) {
    qDeleteAll(this->suggestions);
    this->suggestions = suggestions;
    if (!enabled) return;
    if (!buddy->hasFocus()) return;
    showCompletion(suggestions);
}

void AutoComplete::itemEntered(QListWidgetItem *item) {
    if (!item) return;
    item->setSelected(true);
    popup->setCurrentItem(item);
}

void AutoComplete::currentItemChanged(QListWidgetItem *item) {
    if (!item) return;
    buddy->setText(item->text());
    // lineEdit->setSelection(originalText.length(), editor->text().length());
}
