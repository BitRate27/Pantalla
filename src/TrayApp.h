#pragma once

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include "global.h"
#include "DisplayManager.h"

class DisplayInfo; // forward

// ---------------------------------------------------------------------------
// TrayApp
//
// Owns the system-tray icon and its right-click context menu.
// Lifetime is tied to QApplication — no parent needed.
// ---------------------------------------------------------------------------
class TrayApp : public QObject {
	Q_OBJECT

public:
	explicit TrayApp(QObject *parent = nullptr);
	~TrayApp() override;

	// Call once after construction to make the icon visible.
	void show();

private slots:
	void onAddVirtualDisplay();
	void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
	void onAddNDISender(DisplayInfo *info);
	void onRemoveNDISender(DisplayInfo *info);
	void onRemoveVirtualDisplay(DisplayInfo *info);
	void onOpenSettings();

private:
	void buildIcon();
	void buildMenu();
	void updateMenuContents();

	QSystemTrayIcon *m_trayIcon = nullptr;
	QMenu *m_menu = nullptr;
	QAction *m_addAction = nullptr;
	QAction *m_settingsAction = nullptr;
	QAction *m_exitAction = nullptr;

	DisplayManager *m_displayManager = nullptr;
};
