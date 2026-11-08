#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QStringLiteral>
#include <QStyleHints>
#include <QTranslator>

#include "core/AppSettings.h"
#include "gui/AppController.h"
#include "services/IconProvider.h"
#include "services/JsonStorage.h"
#include "services/Logger.h"
#include "services/SingleInstance.h"

#include <cstdlib>

#ifdef Q_OS_WIN
#  include <windows.h>
#endif

namespace {
void showAlreadyRunningMessage()
{
#ifdef Q_OS_WIN
    MessageBoxW(
        nullptr,
        L"Desktop Hub is already running, but its window could not be brought to the front.\n\n"
        L"Desktop Hub は既に起動していますが、既存のウィンドウを前面に表示できませんでした。",
        L"Desktop Hub",
        MB_OK | MB_ICONWARNING);
#endif
}
} // namespace

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // TODO: Replace these temporary identifiers once the final product name
    // and organization name are decided. On Windows they determine the
    // directories used by QStandardPaths for data and logs.
    QCoreApplication::setOrganizationName(QStringLiteral("Valcyle"));
    QCoreApplication::setApplicationName(QStringLiteral("DesktopHub"));
    QCoreApplication::setApplicationVersion(QStringLiteral(DESKTOPHUB_VERSION));

    Logger::initialize();
    Logger::info(QStringLiteral("Starting Desktop Hub %1 (Qt %2)")
                     .arg(QCoreApplication::applicationVersion(),
                          QString::fromLatin1(qVersion())));

    // A second instance must not run concurrently with the first one.
    SingleInstance singleInstance;
    QString singleInstanceError;
    if (!singleInstance.acquire(&singleInstanceError)) {
        if (!singleInstance.notifyExisting()) {
            Logger::warning(QStringLiteral("Second instance could not reach the "
                                           "existing window: %1")
                                .arg(singleInstanceError));
            showAlreadyRunningMessage();
        }
        return 0;
    }

    // Fusion behaves predictably across Windows versions and can follow the
    // dark color scheme requested below.
    QQuickStyle::setStyle(QStringLiteral("Fusion"));
    QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Dark);

    QQmlApplicationEngine engine;

    // Use the same singleton instances that QML receives.
    AppSettings *settings = engine.singletonInstance<AppSettings *>(
        QStringLiteral("DesktopHub"), QStringLiteral("AppSettings"));
    AppController *controller = engine.singletonInstance<AppController *>(
        QStringLiteral("DesktopHub"), QStringLiteral("App"));
    if (!settings || !controller) {
        return EXIT_FAILURE;
    }

    // Install the stored UI language before loading data so that any startup
    // warning is generated in the correct language.
    const QString storedLanguage = JsonStorage::storedLanguage();
    if (!storedLanguage.isEmpty()) {
        settings->setLanguage(storedLanguage);
    }

    // Translation files are compiled into the executable by CMake.
    // If a translation is missing, the English source strings are used.
    QTranslator translator;
    const auto applyLanguage = [&translator, settings]() {
        QCoreApplication::removeTranslator(&translator);
        if (translator.load(settings->translationFile())) {
            QCoreApplication::installTranslator(&translator);
        }
    };
    applyLanguage();

    // Loads data.json and applies the stored language before QML starts.
    controller->initialize(settings);

    // Switch the UI language immediately without restarting the application.
    QObject::connect(settings, &AppSettings::languageChanged, &engine,
                     [&engine, &applyLanguage, controller]() {
                         applyLanguage();
                         engine.retranslate();
                         controller->refreshTranslations();
                     });

    // Shell icons are requested from QML as image://itemicon/<id>/<revision>.
    engine.addImageProvider(QStringLiteral("itemicon"),
                            new IconProvider(controller->items()));

    // Raise the window when a second instance asks for it.
    bool showPending = false;
    const auto showMainWindow = [&engine]() {
        if (engine.rootObjects().isEmpty()) {
            return;
        }
        if (auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst())) {
            window->show();
            window->raise();
            window->requestActivate();
        }
    };

    QObject::connect(&singleInstance, &SingleInstance::showRequested, &app,
                     [&engine, &showPending, &showMainWindow]() {
                         if (engine.rootObjects().isEmpty()) {
                             showPending = true;
                             return;
                         }
                         showMainWindow();
                     });

    QObject::connect(&app, &QCoreApplication::aboutToQuit,
                     []() { Logger::info(QStringLiteral("Shutting down.")); });

    // Exit with an error instead of silently running without a window
    // when the QML entry point cannot be created.
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(EXIT_FAILURE); },
        Qt::QueuedConnection);

    // "DesktopHub" is the QML module URI declared in CMakeLists.txt.
    engine.loadFromModule(QStringLiteral("DesktopHub"), QStringLiteral("Main"));

    if (showPending) {
        showMainWindow();
    }

    return app.exec();
}
