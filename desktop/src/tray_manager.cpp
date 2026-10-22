#include "tray_manager.hpp"

#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QCursor>
#include <QApplication>
#include <QStyle>

namespace banglakey::desktop {

TrayManager::TrayManager(IpcManager* ipcManager, QObject* parent)
    : QObject(parent),
      ipc_manager_(ipcManager),
      tray_icon_(std::make_unique<QSystemTrayIcon>(this)),
      tray_menu_(std::make_unique<QMenu>()) {
    buildMenu();

    connect(tray_icon_.get(), &QSystemTrayIcon::activated,
            this, &TrayManager::onTrayActivated);

    if (ipc_manager_) {
        connect(ipc_manager_, &IpcManager::modeChanged,
                this, &TrayManager::updateMode);
        updateMode(ipc_manager_->currentMode());
    } else {
        updateMode(AppMode::English);
    }
}

void TrayManager::show() {
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

QIcon TrayManager::createSquareIcon(const QColor& color, bool rounded) {
    QPixmap pixmap(18, 18);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    if (rounded) {
        painter.setBrush(color);
        painter.setPen(color.darker(120));
        painter.drawRoundedRect(2, 2, 14, 14, 3, 3);
    } else {
        painter.fillRect(2, 2, 14, 14, color);
    }

    return QIcon(pixmap);
}

QIcon TrayManager::createAppIcon() {
    QPixmap pixmap(18, 18);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Green circular background
    painter.setBrush(QColor("#006a4e"));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(1, 1, 16, 16);

    // Red inner circle
    painter.setBrush(QColor("#f42a41"));
    painter.drawEllipse(4, 4, 10, 10);

    // Bengali "ক"
    painter.setPen(Qt::white);
    QFont f = painter.font();
    f.setPixelSize(10);
    f.setBold(true);
    painter.setFont(f);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, "ক");

    return QIcon(pixmap);
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

QIcon TrayManager::createTrayIcon(AppMode mode) {
    QPixmap pixmap(24, 24);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background tile based on mode
    QColor bgColor;
    QString label;
    switch (mode) {
        case AppMode::English:
            bgColor = QColor("#1e3c72");
            label = "E";
            break;
        case AppMode::BijoyClassic:
            bgColor = QColor("#0288d1");
            label = "বি";
            break;
        case AppMode::BijoyUnicode:
            bgColor = QColor("#7b1fa2");
            label = "বু";
            break;
        case AppMode::Unicode:
            bgColor = QColor("#2e7d32");
            label = "ইউ";
            break;
    }

    painter.setBrush(bgColor);
    painter.setPen(QColor(255, 255, 255, 180));
    painter.drawRoundedRect(1, 1, 22, 22, 5, 5);

    // Text label
    painter.setPen(Qt::white);
    QFont f = painter.font();
    f.setPixelSize(mode == AppMode::Unicode ? 10 : 12);
    f.setBold(true);
    painter.setFont(f);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, label);

    return QIcon(pixmap);
}

void TrayManager::buildMenu() {
    tray_menu_->clear();

    // Menu styling: compact, light background, dark text, clean thin border matching reference
    tray_menu_->setStyleSheet(
        "QMenu {"
        "  background-color: #ffffff;"
        "  color: #1a1a1a;"
        "  border: 1px solid #c0c0c0;"
        "  padding: 4px;"
        "  font-size: 13px;"
        "}"
        "QMenu::item {"
        "  padding: 4px 24px 4px 8px;"
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

    // 1. Unicode
    action_unicode_ = tray_menu_->addAction(createSquareIcon(QColor("#2e7d32")), tr("Unicode"));
    action_unicode_->setCheckable(true);
    connect(action_unicode_, &QAction::triggered, this, [this]() {
        if (ipc_manager_) ipc_manager_->setMode(AppMode::Unicode);
    });

    // 2. Bijoy Unicode
    action_bijoy_unicode_ = tray_menu_->addAction(createSquareIcon(QColor("#7b1fa2")), tr("Bijoy Unicode"));
    action_bijoy_unicode_->setCheckable(true);
    connect(action_bijoy_unicode_, &QAction::triggered, this, [this]() {
        if (ipc_manager_) ipc_manager_->setMode(AppMode::BijoyUnicode);
    });

    // 3. Bijoy Classic
    action_bijoy_classic_ = tray_menu_->addAction(createSquareIcon(QColor("#0288d1")), tr("Bijoy Classic"));
    action_bijoy_classic_->setCheckable(true);
    connect(action_bijoy_classic_, &QAction::triggered, this, [this]() {
        if (ipc_manager_) ipc_manager_->setMode(AppMode::BijoyClassic);
    });

    // 4. English
    action_english_ = tray_menu_->addAction(createAppIcon(), tr("English"));
    action_english_->setCheckable(true);
    connect(action_english_, &QAction::triggered, this, [this]() {
        if (ipc_manager_) ipc_manager_->setMode(AppMode::English);
    });

    // 5. Horizontal Separator
    tray_menu_->addSeparator();

    // 6. Show as Topbar
    action_show_topbar_ = tray_menu_->addAction(createRestoreIcon(), tr("Show as Topbar"));
    connect(action_show_topbar_, &QAction::triggered, this, [this]() {
        emit showToolbarRequested();
    });

    // 7. Close
    action_close_ = tray_menu_->addAction(createCloseIcon(), tr("Close"));
    connect(action_close_, &QAction::triggered, this, [this]() {
        emit exitRequested();
    });

    tray_icon_->setContextMenu(tray_menu_.get());
}

void TrayManager::updateChecks() {
    if (!ipc_manager_) return;

    AppMode mode = ipc_manager_->currentMode();
    if (action_unicode_) action_unicode_->setChecked(mode == AppMode::Unicode);
    if (action_bijoy_unicode_) action_bijoy_unicode_->setChecked(mode == AppMode::BijoyUnicode);
    if (action_bijoy_classic_) action_bijoy_classic_->setChecked(mode == AppMode::BijoyClassic);
    if (action_english_) action_english_->setChecked(mode == AppMode::English);
}

void TrayManager::updateMode(AppMode mode) {
    updateChecks();

    // Update tray icon image and tooltip
    tray_icon_->setIcon(createTrayIcon(mode));
    tray_icon_->setToolTip(tr("BanglaKey: %1").arg(IpcManager::modeToString(mode)));
}

void TrayManager::onTrayActivated(QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
        case QSystemTrayIcon::Trigger:
            // Left click: cycle through modes
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
