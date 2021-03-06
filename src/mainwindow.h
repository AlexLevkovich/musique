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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#ifdef USE_PHONON
#include <phonon/audiooutput.h>
#include <phonon/volumeslider.h>
#include <phonon/mediaobject.h>
#include <phonon/seekslider.h>
#endif

#ifdef USE_LIBMPV
#include "mpvaudiooutput.h"
#include "mpvvolumeslider.h"
#include "mpvmediaobject.h"
#include "mpvseekslider.h"
#define Phonon Libmpv
#endif


class MediaView;
class CollectionScannerView;
class ContextualView;
class SearchLineEdit;
class Track;
class UpdateChecker;
class Suggestion;

class MainWindow : public QMainWindow {

    Q_OBJECT

public:
    static MainWindow* instance();
    ~MainWindow();
    Phonon::SeekSlider* getSeekSlider() { return seekSlider; }
    Phonon::AudioOutput *getAudioOutput() { return audioOutput; }
    Phonon::VolumeSlider *getVolumeSlider() { return volumeSlider; }
    Phonon::MediaObject *getMediaObject() { return mediaObject; }
    void readSettings();
    SearchLineEdit *getToolbarSearch() { return toolbarSearch; }
    QToolBar* getStatusToolbar() { return statusToolBar; }

    QHash<QString, QAction*> &getActionMap() { return actionMap; }
    QHash<QString, QMenu*> &getMenuMap() { return menuMap; }

    static void printHelp();

public slots:
    void showMediaView(bool transition = true);
    void showChooseFolderView(bool transition = true);
    void toggleContextualView();
    void hideContextualView();
    void updateContextualView(Track *track);
    void showInitialView();
    void quit();
    void showMessage(QString message);
    void handleError(QString message);
    void restore();
    void messageReceived(const QString &message);
    void goBack();
#ifdef APP_ACTIVATION
    void showActivationView(bool transition = true);
    void showActivationDialog();
    void buy();
    void hideBuyAction();
    void showDemoDialog(QString message);
#endif

signals:
    void currentTimeChanged(const QString &s);

protected:
    void closeEvent(QCloseEvent *);
    bool eventFilter(QObject *obj, QEvent *event);
    void resizeEvent(QResizeEvent *);

private slots:
    void lazyInit();
    void visitSite();
    void about();
    void toggleFullscreen();
    void updateUIForFullscreen();
    void setShuffle(bool enabled);
    void setRepeat(bool enabled);
    void checkForUpdate();
    void gotNewVersion(QString version);

    // Phonon related logic
    void stop();
    void stateChanged(Phonon::State newState, Phonon::State oldState);
    void searchFocus();
    void tick(qint64 time);
    void totalTimeChanged(qint64 time);

    // volume shortcuts
    void volumeUp();
    void volumeDown();
    void volumeMute();
    void volumeChanged(qreal newVolume);
    void volumeMutedChanged(bool muted);

    // app logic
    void startFullScan(QString dir);
    void fullScanFinished(const QVariantMap &stats);
    void startIncrementalScan();
    void incrementalScanProgress(int percent);
    void incrementalScanFinished(const QVariantMap &stats);
    void startImageDownload();
    void imageDownloadFinished();
    void search(QString query);
    void suggestionAccepted(Suggestion *suggestion);
    void searchCleared();

    void showActionInStatusBar(QAction*, bool show);
    void showStopAfterThisInStatusBar(bool show);

    void toggleScrobbling(bool enable);
    void enableScrobbling();
    void disableScrobbling();
    void lastFmLogout();

    void savePlaylist();
    void loadPlaylist();

#ifdef APP_MAC_STORE
    void rateOnAppStore();
#endif

    void runFinetune();
    void runFinetune(const QVariantMap &stats);
    void runFinetune(const QString &filename);

private:
    MainWindow();
    void showWidget(QWidget*, bool transition = false);
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void writeSettings();
    void initPhonon();
    static QString formatTime(qint64 duration);
    QString playlistPath();
    void simpleUpdateDialog(QString version);
    void showFinetuneDialog(const QVariantMap &stats);

    // view mechanism
    QStackedWidget *views;
    QStack<QWidget*> *history;

    // view widgets
    QWidget *chooseFolderView;
    CollectionScannerView *collectionScannerView;
    MediaView *mediaView;
    ContextualView *contextualView;
    QWidget *aboutView;

    QHash<QString, QAction*> actionMap;
    QHash<QString, QMenu*> menuMap;

    // actions
    QAction *contextualAct;
    QAction *backAct;
    QAction *quitAct;
    QAction *siteAct;
    QAction *aboutAct;
    QAction *searchFocusAct;
    QAction *chooseFolderAct;

    // media actions
    QAction *skipBackwardAct;
    QAction *skipForwardAct;
    QAction *playAct;
    QAction *fullscreenAct;
    QAction *volumeUpAct;
    QAction *volumeDownAct;
    QAction *volumeMuteAct;

    // playlist actions
    QAction *removeAct;
    QAction *moveDownAct;
    QAction *moveUpAct;

    // menus
    QMenu *fileMenu;
    QMenu *playlistMenu;
    QMenu *playbackMenu;
    QMenu *helpMenu;

    // toolbar
    QToolBar *mainToolBar;
    SearchLineEdit *toolbarSearch;
    QToolBar *statusToolBar;

    // phonon
    Phonon::SeekSlider *seekSlider;
    Phonon::VolumeSlider *volumeSlider;
    Phonon::MediaObject *mediaObject;
    Phonon::AudioOutput *audioOutput;
    QLabel *currentTime;
    // QLabel *totalTime;
    qreal volume;

    // fullscreen
    bool m_fullscreen;
    bool m_maximized;

    // update checker
    UpdateChecker *updateChecker;
};

#endif
