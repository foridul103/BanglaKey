#include "tray_manager.hpp"

#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QCursor>
#include <QApplication>
#include <QStyle>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QDebug>

namespace banglakey::desktop {

bool TrayManager::isStatusNotifierWatcherAvailable() {
    auto bus = QDBusConnection::sessionBus();
    if (!bus.isConnected() || !bus.interface()) {
        return false;
    }
    return bus.interface()->isServiceRegistered("org.kde.StatusNotifierWatcher");
}

bool TrayManager::isAppIndicatorExtensionInstalled() {
    const QString sysPath = QStringLiteral("/usr/share/gnome-shell/extensions/appindicatorsupport@rgcjonas.gmail.com/metadata.json");
    const QString userPath = QDir::homePath() + QStringLiteral("/.local/share/gnome-shell/extensions/appindicatorsupport@rgcjonas.gmail.com/metadata.json");
    return QFile::exists(sysPath) || QFile::exists(userPath);
}

bool TrayManager::isAppIndicatorExtensionEnabled() {
    if (isStatusNotifierWatcherAvailable()) {
        return true;
    }
    if (!isGnome()) {
        return false;
    }
    QProcess proc;
    proc.start(QStringLiteral("gsettings"), {QStringLiteral("get"), QStringLiteral("org.gnome.shell"), QStringLiteral("enabled-extensions")});
    if (proc.waitForFinished(500)) {
        QString out = QString::fromUtf8(proc.readAllStandardOutput());
        return out.contains(QStringLiteral("appindicatorsupport@rgcjonas.gmail.com"));
    }
    return false;
}

bool TrayManager::isKdePlasma() {
    const QString desktop = desktopEnvironment().toUpper();
    return desktop.contains("KDE") || desktop.contains("PLASMA");
}

bool TrayManager::isGnome() {
    const QString desktop = desktopEnvironment().toUpper();
    return desktop.contains("GNOME");
}

QString TrayManager::desktopSessionType() {
    return QString::fromLocal8Bit(qgetenv("XDG_SESSION_TYPE")).toLower();
}

QString TrayManager::desktopEnvironment() {
    return QString::fromLocal8Bit(qgetenv("XDG_CURRENT_DESKTOP"));
}

QString TrayManager::trayStatusSummary() {
    if (isStatusNotifierWatcherAvailable()) {
        return QStringLiteral("StatusNotifierWatcher is active on session D-Bus. System tray integration is fully operational.");
    }
    if (isKdePlasma()) {
        return QStringLiteral("KDE Plasma session detected. StatusNotifierItem is natively supported.");
    }
    if (isGnome()) {
        if (!isAppIndicatorExtensionInstalled()) {
            return QStringLiteral("GNOME Wayland detected without AppIndicator extension. Install 'gnome-shell-extension-appindicator' to enable top-bar tray icons.");
        }
        if (!isAppIndicatorExtensionEnabled()) {
            return QStringLiteral("AppIndicator extension is installed but not active. Log out and back in, or run 'gnome-extensions enable appindicatorsupport@rgcjonas.gmail.com'.");
        }
        return QStringLiteral("GNOME Wayland detected. StatusNotifierWatcher service is waiting to be initialized.");
    }
    return QStringLiteral("System tray service (StatusNotifierWatcher) is not currently active in this desktop session.");
}

bool TrayManager::isTrayAvailable() const {
    return isStatusNotifierWatcherAvailable() || QSystemTrayIcon::isSystemTrayAvailable();
}

TrayManager::TrayManager(IpcManager* ipcManager, QObject* parent)
    : QObject(parent),
      ipc_manager_(ipcManager),
      tray_icon_(std::make_unique<QSystemTrayIcon>(this)),
      tray_menu_(std::make_unique<QMenu>()) {

    buildMenu();

    connect(tray_icon_.get(), &QSystemTrayIcon::activated,
            this, &TrayManager::onTrayActivated);

    // Watch for StatusNotifierWatcher appearing/disappearing dynamically
    service_watcher_ = new QDBusServiceWatcher(
        QStringLiteral("org.kde.StatusNotifierWatcher"),
        QDBusConnection::sessionBus(),
        QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration,
        this);

    connect(service_watcher_, &QDBusServiceWatcher::serviceRegistered, this, [this](const QString&) {
        qInfo() << "BanglaKey: StatusNotifierWatcher appeared on session D-Bus. Initializing tray icon.";
        emit trayAvailabilityChanged(true);
        if (tray_icon_) {
            tray_icon_->show();
        }
    });

    connect(service_watcher_, &QDBusServiceWatcher::serviceUnregistered, this, [this](const QString&) {
        qInfo() << "BanglaKey: StatusNotifierWatcher unregistered from session D-Bus.";
        emit trayAvailabilityChanged(false);
    });

    if (ipc_manager_) {
        connect(ipc_manager_, &IpcManager::modeChanged,
                this, &TrayManager::updateMode);
        updateMode(ipc_manager_->currentMode());
    } else {
        updateMode(AppMode::English);
    }
}

void TrayManager::show() {
    if (!isTrayAvailable()) {
        qWarning() << "BanglaKey:" << trayStatusSummary()
                   << "The floating toolbar remains active and can be restored anytime by running 'banglakey-settings'.";
        return;
    }
    if (tray_icon_) {
        tray_icon_->show();
    }
}

void TrayManager::hide() {
    if (tray_icon_) {
        tray_icon_->hide();
    }
}

bool TrayManager::isVisible() const {
    return tray_icon_ && tray_icon_->isVisible();
}

QIcon TrayManager::createModeIcon(AppMode mode) {
    QIcon icon;
    const std::vector<int> sizes = {16, 22, 24, 32, 48, 64};

    QString label;
    QString bgColor;
    switch (mode) {
        case AppMode::English:
            label = QStringLiteral("EN");
            bgColor = QStringLiteral("#37474f"); // Dark graphite
            break;
        case AppMode::BijoyUnicode:
            label = QString::fromUtf8("বি-ইউ");
            bgColor = QStringLiteral("#006a4e"); // National emerald green
            break;
        case AppMode::BijoyClassic:
            label = QString::fromUtf8("বিজয়");
            bgColor = QStringLiteral("#1565c0"); // Deep royal blue
            break;
        case AppMode::AvroPhonetic:
            label = QString::fromUtf8("অভ্র");
            bgColor = QStringLiteral("#6a1b9a"); // Rich purple
            break;
    }

    for (int sz : sizes) {
        QPixmap pixmap(sz, sz);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);

        // Rounded badge container with subtle border
        int radius = std::max(2, sz * 4 / 22);
        painter.setBrush(QColor(bgColor));
        painter.setPen(QPen(QColor(0, 0, 0, 80), 1.0));
        painter.drawRoundedRect(1, 1, sz - 2, sz - 2, radius, radius);

        // Font calculation with Bengali font fallback
        QFont font(QStringLiteral("Noto Sans Bengali"));
        font.setStyleHint(QFont::SansSerif);
        font.setBold(true);

        int len = static_cast<int>(label.length());
        int px = sz * 11 / 22;
        if (len > 3) px = sz * 8 / 22;
        else if (len > 2) px = sz * 9 / 22;

        font.setPixelSize(std::max(7, px));
        painter.setFont(font);
        painter.setPen(Qt::white);
        painter.drawText(pixmap.rect(), Qt::AlignCenter, label);
        painter.end();

        icon.addPixmap(pixmap);
    }
    return icon;
}

QIcon TrayManager::createAppIcon() {
    return createModeIcon(ipc_manager_ ? ipc_manager_->currentMode() : AppMode::BijoyUnicode);
}

QIcon TrayManager::createRestoreIcon() {
    QPixmap pixmap(18, 18);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Green restore topbar window icon
    painter.setPen(QPen(QColor("#1b5e20"), 1.8));
    painter.setBrush(QColor("#e8f5e9"));
    painter.drawRoundedRect(2, 4, 14, 10, 2, 2);

    // Topbar header bar
    painter.fillRect(2, 4, 14, 3, QColor("#2e7d32"));

    return QIcon(pixmap);
}

QIcon TrayManager::createCloseIcon() {
    QPixmap pixmap(18, 18);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Red cross
    painter.setPen(QPen(QColor("#c62828"), 2.2, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(4, 4, 14, 14);
    painter.drawLine(14, 4, 4, 14);

    return QIcon(pixmap);
}

void TrayManager::buildMenu() {
    tray_menu_->clear();

    // Menu styling: native clean light appearance with clear checkmark indicators
    tray_menu_->setStyleSheet(
        "QMenu {"
        "  background-color: #ffffff;"
        "  color: #1a1a1a;"
        "  border: 1px solid #c0c0c0;"
        "  padding: 4px;"
        "  font-size: 13px;"
        "}"
        "QMenu::item {"
        "  padding: 4px 28px 4px 10px;"
        "  border-radius: 3px;"
        "}"
        "QMenu::item:selected {"
        "  background-color: #2b78e4;"
        "  color: #ffffff;"
        "}"
        "QMenu::separator {"
        "  height: 1px;"
        "  background-color: #e0e0e0;"
        "  margin: 4px 2px;"
        "}"
    );

    mode_group_ = new QActionGroup(this);
    mode_group_->setExclusive(true);

    // 1. Bijoy Unicode
    action_bijoy_unicode_ = tray_menu_->addAction(tr("Bijoy Unicode"));
    action_bijoy_unicode_->setCheckable(true);
    mode_group_->addAction(action_bijoy_unicode_);
    connect(action_bijoy_unicode_, &QAction::triggered, this, [this]() {
        if (ipc_manager_) ipc_manager_->setMode(AppMode::BijoyUnicode);
    });

    // 2. Bijoy Classic
    action_bijoy_classic_ = tray_menu_->addAction(tr("Bijoy Classic"));
    action_bijoy_classic_->setCheckable(true);
    mode_group_->addAction(action_bijoy_classic_);
    connect(action_bijoy_classic_, &QAction::triggered, this, [this]() {
        if (ipc_manager_) ipc_manager_->setMode(AppMode::BijoyClassic);
    });

    // 3. Avro Phonetic
    action_avro_phonetic_ = tray_menu_->addAction(tr("Avro Phonetic"));
    action_avro_phonetic_->setCheckable(true);
    mode_group_->addAction(action_avro_phonetic_);
    action_unicode_ = action_avro_phonetic_;
    connect(action_avro_phonetic_, &QAction::triggered, this, [this]() {
        if (ipc_manager_) ipc_manager_->setMode(AppMode::AvroPhonetic);
    });

    // 4. English
    action_english_ = tray_menu_->addAction(tr("English"));
    action_english_->setCheckable(true);
    mode_group_->addAction(action_english_);
    connect(action_english_, &QAction::triggered, this, [this]() {
        if (ipc_manager_) ipc_manager_->setMode(AppMode::English);
    });

    // 5. Separator
    tray_menu_->addSeparator();

    // 6. Show as Topbar
    action_show_topbar_ = tray_menu_->addAction(createRestoreIcon(), tr("Show as Topbar"));
    connect(action_show_topbar_, &QAction::triggered, this, [this]() {
        emit showToolbarRequested();
    });

    // 7. Settings
    action_settings_ = tray_menu_->addAction(tr("Settings..."));
    connect(action_settings_, &QAction::triggered, this, [this]() {
        emit openSettingsRequested();
    });

    // 8. Close
    action_close_ = tray_menu_->addAction(createCloseIcon(), tr("Close"));
    connect(action_close_, &QAction::triggered, this, [this]() {
        emit exitRequested();
    });

    tray_icon_->setContextMenu(tray_menu_.get());
}

void TrayManager::updateChecks() {
    if (!ipc_manager_) return;

    AppMode mode = ipc_manager_->currentMode();
    switch (mode) {
        case AppMode::BijoyUnicode:
            if (action_bijoy_unicode_) action_bijoy_unicode_->setChecked(true);
            break;
        case AppMode::BijoyClassic:
            if (action_bijoy_classic_) action_bijoy_classic_->setChecked(true);
            break;
        case AppMode::AvroPhonetic:
            if (action_avro_phonetic_) action_avro_phonetic_->setChecked(true);
            break;
        case AppMode::English:
            if (action_english_) action_english_->setChecked(true);
            break;
    }
}

void TrayManager::updateMode(AppMode mode) {
    updateChecks();

    // Update tray icon image with active mode badge and tooltip
    tray_icon_->setIcon(createModeIcon(mode));
    tray_icon_->setToolTip(tr("BanglaKey (%1)").arg(IpcManager::modeToString(mode)));
}

void TrayManager::onTrayActivated(QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
        case QSystemTrayIcon::Trigger:
            // Single left click: cycle typing mode
            if (ipc_manager_) {
                ipc_manager_->cycleMode();
            }
            break;

        case QSystemTrayIcon::DoubleClick:
            // Double click: restore floating toolbar
            emit showToolbarRequested();
            break;

        case QSystemTrayIcon::Context:
            // Right click: show context menu at cursor
            if (tray_menu_) {
                updateChecks();
                tray_menu_->popup(QCursor::pos());
            }
            break;

        default:
            break;
    }
}

} // namespace banglakey::desktop
