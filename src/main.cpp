#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QStringLiteral>

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

    QQmlApplicationEngine engine;

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
