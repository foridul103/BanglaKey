#include "ipc_manager.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QDebug>

namespace banglakey::desktop {

namespace {

QString getRuntimeDirectory() {
    QString runtimeDir = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    if (runtimeDir.isEmpty()) {
        runtimeDir = QDir::tempPath() + "/banglakey-" + QString::fromLocal8Bit(qgetenv("USER"));
    }
    QString appDir = runtimeDir + "/banglakey";
    QDir().mkpath(appDir);
    return appDir;
}

} // namespace

QString IpcManager::runtimeModeFilePath() {
    return getRuntimeDirectory() + "/active_mode";
}

QString IpcManager::runtimeSocketPath() {
    return getRuntimeDirectory() + "/ipc.sock";
}

QString IpcManager::modeToString(AppMode mode) {
    switch (mode) {
        case AppMode::English:
            return "English";
        case AppMode::BijoyClassic:
            return "Bijoy Classic";
        case AppMode::BijoyUnicode:
            return "Bijoy Unicode";
        case AppMode::Unicode:
            return "Unicode";
    }
    return "English";
}

AppMode IpcManager::stringToMode(const QString& str) {
    QString trimmed = str.trimmed();
    if (trimmed.compare("Bijoy Classic", Qt::CaseInsensitive) == 0 ||
        trimmed.compare("BijoyClassic", Qt::CaseInsensitive) == 0) {
        return AppMode::BijoyClassic;
    }
    if (trimmed.compare("Bijoy Unicode", Qt::CaseInsensitive) == 0 ||
        trimmed.compare("BijoyUnicode", Qt::CaseInsensitive) == 0) {
        return AppMode::BijoyUnicode;
    }
    if (trimmed.compare("Unicode", Qt::CaseInsensitive) == 0 ||
        trimmed.compare("Phonetic", Qt::CaseInsensitive) == 0) {
        return AppMode::Unicode;
    }
    return AppMode::English;
}

QString IpcManager::modeToDisplayName(AppMode mode) {
    switch (mode) {
        case AppMode::English:
            return "English";
        case AppMode::BijoyClassic:
            return "বিজয় ক্লাসিক";
        case AppMode::BijoyUnicode:
            return "বিজয় ইউনিকোড";
        case AppMode::Unicode:
            return "ইউনিকোড";
    }
    return "English";
}

IpcManager::IpcManager(QObject* parent)
    : QObject(parent) {
    // 1. Load saved mode from persistent settings
    QSettings settings("BanglaKey", "BanglaKey Settings");
    QString saved = settings.value("General/TypingMode", "English").toString();
    current_mode_ = stringToMode(saved);

    // 2. Sync to runtime mode file
    writeRuntimeModeFile(current_mode_);

    // 3. Set up file watcher to listen for external mode changes (e.g. from Fcitx5 addon hotkeys)
    file_watcher_ = std::make_unique<QFileSystemWatcher>(this);
    const QString modePath = runtimeModeFilePath();
    if (QFile::exists(modePath)) {
        file_watcher_->addPath(modePath);
    }

    connect(file_watcher_.get(), &QFileSystemWatcher::fileChanged,
            this, &IpcManager::onFileChanged);
}

IpcManager::~IpcManager() {
    if (local_server_ && local_server_->isListening()) {
        local_server_->close();
        QLocalServer::removeServer(runtimeSocketPath());
    }
}

void IpcManager::cycleMode() {
    AppMode nextMode = AppMode::English;
    switch (current_mode_) {
        case AppMode::English:
            nextMode = AppMode::Unicode;
            break;
        case AppMode::Unicode:
            nextMode = AppMode::BijoyUnicode;
            break;
        case AppMode::BijoyUnicode:
            nextMode = AppMode::BijoyClassic;
            break;
        case AppMode::BijoyClassic:
            nextMode = AppMode::English;
            break;
    }
    setMode(nextMode);
}

void IpcManager::setMode(AppMode mode) {
    if (current_mode_ == mode) {
        return;
    }

    current_mode_ = mode;
    writeRuntimeModeFile(mode);

    // Persist to user settings
    QSettings settings("BanglaKey", "BanglaKey Settings");
    settings.setValue("General/TypingMode", modeToString(mode));
    settings.sync();

    emit modeChanged(mode);
}

void IpcManager::setModeFromString(const QString& modeStr) {
    setMode(stringToMode(modeStr));
}

void IpcManager::writeRuntimeModeFile(AppMode mode) {
    const QString modePath = runtimeModeFilePath();
    QFile file(modePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        // Set permissions to 0600 (user read/write only)
        file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
        QTextStream out(&file);
        out << modeToString(mode) << "\n";
        file.flush();
    }
}

AppMode IpcManager::readRuntimeModeFile() {
    const QString modePath = runtimeModeFilePath();
    QFile file(modePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString line = in.readLine();
        return stringToMode(line);
    }
    return current_mode_;
}

void IpcManager::onFileChanged(const QString& path) {
    // If file was replaced or rewritten by Fcitx5, re-add to watcher
    if (QFile::exists(path) && !file_watcher_->files().contains(path)) {
        file_watcher_->addPath(path);
    }

    AppMode newMode = readRuntimeModeFile();
    if (newMode != current_mode_) {
        current_mode_ = newMode;
        // Update persistent settings
        QSettings settings("BanglaKey", "BanglaKey Settings");
        settings.setValue("General/TypingMode", modeToString(newMode));
        settings.sync();

        emit modeChanged(newMode);
    }
}

bool IpcManager::initSingleInstance() {
    const QString sockPath = runtimeSocketPath();

    // 1. Try connecting to check if another instance is already running
    QLocalSocket testSocket;
    testSocket.connectToServer(sockPath);
    if (testSocket.waitForConnected(200)) {
        // Another instance is running! Send command to restore toolbar
        testSocket.write("SHOW_TOOLBAR\n");
        testSocket.flush();
        testSocket.waitForBytesWritten(300);
        testSocket.disconnectFromServer();
        qInfo() << "BanglaKey: Existing instance detected. Requested toolbar restore.";
        return false;
    }

    // 2. Remove stale socket if any
    QLocalServer::removeServer(sockPath);

    // 3. Start local server
    local_server_ = std::make_unique<QLocalServer>(this);
    if (!local_server_->listen(sockPath)) {
        qWarning() << "BanglaKey: Unable to start IPC server at" << sockPath << ":" << local_server_->errorString();
    } else {
        // Restrict socket file permissions
        QFile::setPermissions(sockPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
        connect(local_server_.get(), &QLocalServer::newConnection,
                this, &IpcManager::onNewConnection);
    }

    return true;
}

bool IpcManager::notifyRunningInstance() {
    const QString sockPath = runtimeSocketPath();
    QLocalSocket socket;
    socket.connectToServer(sockPath);
    if (socket.waitForConnected(200)) {
        socket.write("SHOW_TOOLBAR\n");
        socket.flush();
        socket.waitForBytesWritten(300);
        return true;
    }
    return false;
}

void IpcManager::onNewConnection() {
    while (local_server_->hasPendingConnections()) {
        QLocalSocket* client = local_server_->nextPendingConnection();
        connect(client, &QLocalSocket::readyRead, this, [this, client]() {
            QByteArray data = client->readAll();
            QString cmd = QString::fromUtf8(data).trimmed();
            if (cmd == "SHOW_TOOLBAR") {
                emit showToolbarRequested();
            } else if (cmd.startsWith("MODE:")) {
                QString modeStr = cmd.mid(5).trimmed();
                setMode(stringToMode(modeStr));
            }
            client->deleteLater();
        });
    }
}

} // namespace banglakey::desktop
