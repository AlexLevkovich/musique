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
#ifndef AUTOCOMPLETE_H
#define AUTOCOMPLETE_H

#include <QtGui>

class Suggester;
class Suggestion;
class SearchLineEdit;

class AutoComplete : public QObject {

    Q_OBJECT

public:
    AutoComplete(SearchLineEdit *buddy, QLineEdit *lineEdit);
    ~AutoComplete();
    bool eventFilter(QObject *obj, QEvent *ev);
    void showCompletion(const QList<Suggestion*> &suggestions);
    void setSuggester(Suggester* suggester);
    QListWidget* getPopup() { return popup; }

public slots:
    void doneCompletion();
    void preventSuggest();
    void enableSuggest();
    void autoSuggest();
    void itemEntered(QListWidgetItem *item);
    void currentItemChanged(QListWidgetItem *item);
    void suggestionsReady(const QList<Suggestion*> &suggestions);

signals:
    void suggestionAccepted(Suggestion *suggestion);
    void suggestionAccepted(const QString &value);

private:
    SearchLineEdit *buddy;
    QLineEdit *lineEdit;
    QString originalText;
    QListWidget *popup;
    QTimer *timer;
    bool enabled;
    Suggester *suggester;
    QList<Suggestion*> suggestions;

};

#endif // AUTOCOMPLETE_H
