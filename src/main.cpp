#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QStringLiteral>
#include <QStyleHints>
#include <QTranslator>

#include "core/AppSettings.h"

#include <cstdlib>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // TODO: Replace these temporary identifiers once the final product name
    // and organization name are decided. On Windows they determine the
    // directories used by QStandardPaths for data and logs.
    QCoreApplication::setOrganizationName(QStringLiteral("Valcyle"));
    QCoreApplication::setApplicationName(QStringLiteral("DesktopHub"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));

    // Fusion behaves predictably across Windows versions and can follow the
    // dark color scheme requested below.
    QQuickStyle::setStyle(QStringLiteral("Fusion"));
    QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Dark);

    QQmlApplicationEngine engine;

    // Use the same singleton instance that QML receives. This guarantees that
    // the translation setup and the settings page operate on one object.
    AppSettings *settings = engine.singletonInstance<AppSettings *>(
        QStringLiteral("DesktopHub"), QStringLiteral("AppSettings"));
    if (!settings) {
        return EXIT_FAILURE;
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

    // Switch the UI language immediately without restarting the application.
    QObject::connect(settings, &AppSettings::languageChanged, &engine,
                     [&engine, &applyLanguage]() {
                         applyLanguage();
                         engine.retranslate();
                     });

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

    return app.exec();
}
