#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>

#include <cstring>

#include "cmake.h"
#include "global.h"
#include "styletweaks.h"
#include "tui.h"

#ifdef APPIMAGE_EDITION
#include "helper.h"
#include <QDebug>
#endif

static bool isOptionPassed(int argc, char *argv[], const char *option)
{
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], option) == 0)
            return true;
    }

    return false;
}

// QApplication requires a graphical display. With --tui, --help, --version or
// without a display, the terminal interface is started over QCoreApplication.
static bool isTerminalRequested(int argc, char *argv[])
{
    for (const char *option : { "--tui", "-h", "--help", "--help-all", "-v", "--version" }) {
        if (isOptionPassed(argc, argv, option))
            return true;
    }

    return false;
}

int main(int argc, char *argv[])
{
#ifdef APPIMAGE_EDITION
    if (argc != 1) {
        if (QString::compare(argv[1], "--helper") == 0) {
            QCoreApplication a(argc, argv);
            if (Global::isRunningAsRoot()) {
                if (argc >= 3) {
                    Helper *helper = new Helper(a.arguments().at(2));
                    return helper->connectToServer() ? a.exec(): -1;
                }
                else {
                    qCritical() << "Helper id must be defined.";
                    return -1;
                }
            }
            else {
                qCritical() << "The helper must be run as superuser.";
                return -1;
            }
        }
    }

    if (Global::isRunningAsRoot()) {
        qCritical() << "You should run KDiskMark as normal user.";
        return -1;
    }
#endif
    QCoreApplication::setApplicationName(QStringLiteral(PROJECT_NAME));
    QCoreApplication::setApplicationVersion(QStringLiteral("%1.%2.%3").arg(PROJECT_VERSION_MAJOR)
                                            .arg(PROJECT_VERSION_MINOR).arg(PROJECT_VERSION_PATCH));
    QCoreApplication::setOrganizationName(QStringLiteral(PROJECT_NAME));

    const bool noDisplay = qEnvironmentVariableIsEmpty("DISPLAY")
                        && qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY");

    if (isTerminalRequested(argc, argv) || noDisplay) {
        QCoreApplication app(argc, argv);

        Tui tui;
        return tui.run(app);
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication a(argc, argv);
    QApplication::setWindowIcon(QIcon(QStringLiteral("%1/../share/icons/hicolor/256x256/apps/%2.png")
                                      .arg(qApp->applicationDirPath()).arg(PROJECT_NAME)));

    AppSettings().setupLocalization();

    a.setStyle(new StyleTweaks());

    MainWindow w;
    w.setFixedSize(w.size());
    w.show();

    return a.exec();
}
