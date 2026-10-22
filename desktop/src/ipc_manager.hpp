#pragma once

#include <QObject>
#include <QString>
#include <QFileSystemWatcher>
#include <QLocalServer>
#include <QLocalSocket>
#include <memory>

namespace banglakey::desktop {

enum class AppMode {
    English,
    BijoyClassic,
    BijoyUnicode,
    Unicode
};

class IpcManager : public QObject {
    Q_OBJECT

public:
    explicit IpcManager(QObject* parent = nullptr);
    ~IpcManager() override;

    // Mode strings helpers
    static QString modeToString(AppMode mode);
    static AppMode stringToMode(const QString& str);
    static QString modeToDisplayName(AppMode mode);

    // Current active mode
    [[nodiscard]] AppMode currentMode() const { return current_mode_; }
    [[nodiscard]] QString currentModeString() const { return modeToString(current_mode_); }
    [[nodiscard]] QString currentDisplayName() const { return modeToDisplayName(current_mode_); }

    // Cycle through modes: English -> Unicode -> BijoyUnicode -> BijoyClassic -> English
    void cycleMode();

    // Set mode explicitly
    void setMode(AppMode mode);
    void setModeFromString(const QString& modeStr);

    // Single instance initialization
    // Returns true if this is the primary instance.
    // Returns false if another instance was found (and notified to show itself).
    bool initSingleInstance();

    // Broadcast "SHOW_TOOLBAR" signal to running instance
    static bool notifyRunningInstance();

    // Runtime state file path ($XDG_RUNTIME_DIR/banglakey/active_mode)
    static QString runtimeModeFilePath();
    static QString runtimeSocketPath();

signals:
    void modeChanged(AppMode newMode);
    void showToolbarRequested();

private slots:
    void onNewConnection();
    void onFileChanged(const QString& path);

private:
    void writeRuntimeModeFile(AppMode mode);
    AppMode readRuntimeModeFile();

    AppMode current_mode_{AppMode::English};
    std::unique_ptr<QLocalServer> local_server_{nullptr};
    std::unique_ptr<QFileSystemWatcher> file_watcher_{nullptr};
};

} // namespace banglakey::desktop
