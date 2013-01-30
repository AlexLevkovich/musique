#include <QtGui>
#include <QNetworkReply>
#include <qtsingleapplication.h>
#include "constants.h"
#include "mainwindow.h"
#include "utils.h"
#ifndef Q_WS_X11
#include "extra.h"
#endif
#ifdef Q_WS_MAC
#include "mac_startup.h"
#endif

#ifdef Q_WS_X11
QString getThemeName() {
    QString themeName;

    QProcess process;
    process.start("dconf",
                  QStringList() << "read" << "/org/gnome/desktop/interface/gtk-theme");
    if (process.waitForFinished()) {
        themeName = process.readAllStandardOutput();
        themeName = themeName.trimmed();
        themeName.remove('\'');
        if (!themeName.isEmpty()) return themeName;
    }

    QString rcPaths = QString::fromLocal8Bit(qgetenv("GTK2_RC_FILES"));
    if (!rcPaths.isEmpty()) {
        QStringList paths = rcPaths.split(QLatin1String(":"));
        foreach (const QString &rcPath, paths) {
            if (!rcPath.isEmpty()) {
                QFile rcFile(rcPath);
                if (rcFile.exists() && rcFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream in(&rcFile);
                    while(!in.atEnd()) {
                        QString line = in.readLine();
                        if (line.contains(QLatin1String("gtk-theme-name"))) {
                            line = line.right(line.length() - line.indexOf(QLatin1Char('=')) - 1);
                            line.remove(QLatin1Char('\"'));
                            line = line.trimmed();
                            themeName = line;
                            break;
                        }
                    }
                }
            }
            if (!themeName.isEmpty())
                break;
        }
    }

    // Fall back to gconf
    if (themeName.isEmpty())
        themeName = QGtkStyle::getGConfString(QLatin1String("/desktop/gnome/interface/gtk_theme"));

    return themeName;
}
#endif

int main(int argc, char **argv) {

#ifdef QT_MAC_USE_COCOA
    mac::MacMain();
#endif

    QtSingleApplication app(argc, argv);
    QString message = app.arguments().size() > 1 ? app.arguments().at(1) : "";
    if (message == "--help") {
        MainWindow::printHelp();
        return 0;
    }
    if (app.sendMessage(message))
        return 0;

    app.setApplicationName(Constants::NAME);
    app.setOrganizationName(Constants::ORG_NAME);
    app.setOrganizationDomain(Constants::ORG_DOMAIN);
    app.setWheelScrollLines(1);
    app.setAttribute(Qt::AA_DontShowIconsInMenus);

#ifndef Q_WS_X11
    Extra::appSetup(&app);
#else
    bool isGtk = app.style()->metaObject()->className() == QLatin1String("QGtkStyle");
    if (isGtk) {
        app.setProperty("gtk", isGtk);
        QString themeName = getThemeName();
        app.setProperty("style", themeName);
    }
    QFile cssFile(":/style.css");
    cssFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(cssFile.readAll());
    app.setStyleSheet(styleSheet);
#endif

    const QString locale = QLocale::system().name();

    // qt translations
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + locale,
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    // qWarning() << "Qt translations:" << QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    app.installTranslator(&qtTranslator);

    // app translations
#ifdef PKGDATADIR
    QString dataDir = QLatin1String(PKGDATADIR);
#else
    QString dataDir = "";
#endif
    QString localeDir = qApp->applicationDirPath() + QDir::separator() + "locale";
    if (!QDir(localeDir).exists()) {
        localeDir = dataDir + QDir::separator() + "locale";
    }
    QTranslator translator;
    translator.load(QLocale::system(), localeDir);
    app.installTranslator(&translator);
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));

    MainWindow* mainWin = MainWindow::instance();
    mainWin->setWindowTitle(Constants::NAME);

#ifndef Q_WS_X11
    Extra::windowSetup(mainWin);
#else
    mainWin->setProperty("style", app.property("style"));
#endif

#ifndef APP_MAC
    QIcon appIcon;
    if (QDir(dataDir).exists()) {
        appIcon = Utils::icon(Constants::UNIX_NAME);
    } else {
        dataDir = qApp->applicationDirPath() + "/data";
        const int iconSizes [] = { 16, 22, 32, 48, 64, 128, 256, 512 };
        for (int i = 0; i < 8; i++) {
            QString size = QString::number(iconSizes[i]);
            QString png = dataDir + "/" + size + "x" + size + "/" + Constants::UNIX_NAME + ".png";
            appIcon.addFile(png, QSize(iconSizes[i], iconSizes[i]));
        }
    }
    if (appIcon.isNull()) {
        appIcon.addFile(":/images/app.png");
    }
    mainWin->setWindowIcon(appIcon);
#endif

    mainWin->show();

    mainWin->connect(&app, SIGNAL(messageReceived(const QString &)), mainWin, SLOT(messageReceived(const QString &)));
    app.setActivationWindow(mainWin, true);

    // all string literals are UTF-8
    // QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    // This is required in order to use QNetworkReply::NetworkError in QueuedConnetions
    qRegisterMetaType<QNetworkReply::NetworkError>("QNetworkReply::NetworkError");

    // Seed random number generator
    qsrand(QDateTime::currentDateTime().toTime_t());

    return app.exec();
}
