#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>

#include "settings_backend.hpp"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);

    app.setOrganizationName("BanglaKey");
    app.setOrganizationDomain("banglakey.org");
    app.setApplicationName("BanglaKey Settings");
    app.setApplicationVersion("0.1.0");

    QQmlApplicationEngine engine;

    auto backend = std::make_unique<banglakey::desktop::SettingsBackend>();
    engine.rootContext()->setContextProperty("backend", backend.get());

    const QUrl url(QStringLiteral("qrc:/qt/qml/banglakey/desktop/qml/Main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject* obj, const QUrl& objUrl) {
            if (!obj && url == objUrl) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection
    );

    // Also support loading directly from filesystem if resource is not found
    engine.load(url);
    if (engine.rootObjects().isEmpty()) {
        engine.load(QUrl::fromLocalFile(QStringLiteral(BANGLAKEY_QML_SOURCE_DIR "/qml/Main.qml")));
    }

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
