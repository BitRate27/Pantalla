#include "TrayApp.h"
#include "DisplayManager.h"
#include "List.h"
#include <QIcon>
#include <QPainter>
#include <QPixmap>
#include <QApplication>
#include <QLabel>
#include <QWidgetAction>
#include <QPushButton>
#include <QHBoxLayout>
#include <QDialog>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QDialogButtonBox>

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
TrayApp::TrayApp(QObject *parent) : QObject(parent)
{
	m_displayManager = new DisplayManager();
	buildIcon();
	buildMenu();
}

TrayApp::~TrayApp()
{
	log_file("Destroying TrayApp\n");
	delete m_displayManager;

	close_log();
}

// ---------------------------------------------------------------------------
// Public
// ---------------------------------------------------------------------------
void TrayApp::show()
{
	m_trayIcon->show();
}

// ---------------------------------------------------------------------------
// Private slots
// ---------------------------------------------------------------------------
void TrayApp::onAddVirtualDisplay()
{
	bool added = m_displayManager->addVirtualDisplay();

	log_file("Add virtual display result = %d\n", added);
}

void TrayApp::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
	if (reason == QSystemTrayIcon::Context) {
		// Rebuild the menu contents so it includes any recently added screens
		updateMenuContents();

		// Try to obtain the tray icon geometry (global coordinates)
		QRect iconGeom = m_trayIcon->geometry();

		// Fallback to cursor pos if geometry is empty
		QPoint popupPos;
		if (iconGeom.isValid() && !iconGeom.isNull()) {
			// Left-justify the menu to the icon, and place it above the icon
			QSize menuSize = m_menu->sizeHint();
			popupPos = iconGeom.topLeft();
			popupPos.setY(iconGeom.top() - menuSize.height());
		} else {
			popupPos = QCursor::pos();
		}

		// Ensure the menu is shown on the screen
		m_menu->popup(popupPos);
	}
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------
void TrayApp::buildIcon()
{
	QPixmap pm(32, 32);
	pm.fill(Qt::transparent);

	QPainter p(&pm);
	p.setRenderHint(QPainter::Antialiasing);

	// Outer circle — red
	p.setBrush(QColor(0xC8,0x20,0x27));
	p.setPen(Qt::NoPen);
	p.drawEllipse(0,0,31,31);

	QRect textRect(0,0,31,31);

	QFont f = p.font();
	f.setBold(true);
	// Pick a pixel size that fits the available rect. Start large and decrease until it fits.
	int chosenPixelSize =32;
	for (int sz = chosenPixelSize; sz >0; --sz) {
		f.setPixelSize(sz);
		QFontMetrics fm(f);
		QRect br = fm.boundingRect(textRect, Qt::AlignCenter, QStringLiteral("P"));
		if (br.width() <= textRect.width() && br.height() <= textRect.height()) {
			chosenPixelSize = sz;
			break;
		}
	}
	f.setPixelSize(chosenPixelSize);
	p.setFont(f);
	p.setPen(Qt::white);
	p.drawText(textRect, Qt::AlignCenter, QStringLiteral("P"));
	p.end();

	m_trayIcon = new QSystemTrayIcon(QIcon(pm), this);
	m_trayIcon->setToolTip(QStringLiteral("Pantalla"));
}

void TrayApp::buildMenu()
{
	// QMenu without a parent: we manage its lifetime manually in the d-tor.
	m_menu = new QMenu();

	m_addAction = new QAction(QStringLiteral("Add virtual display"), this);
	connect(m_addAction, &QAction::triggered, this, &TrayApp::onAddVirtualDisplay);

	// Settings action
	m_settingsAction = new QAction(QStringLiteral("Settings"), this);
	connect(m_settingsAction, &QAction::triggered, this, &TrayApp::onOpenSettings);

	// Exit action — quits the application/event loop
	m_exitAction = new QAction(QStringLiteral("Exit"), this);
	connect(m_exitAction, &QAction::triggered, qApp, &QApplication::quit);

	// Make menu show a left icon area equal to the NDI button width so bottom actions align with labels
	const int indentWidth = 51; // matches NDI button width used in updateMenuContents
	QPixmap indentPix(indentWidth, 18);
	indentPix.fill(Qt::transparent);
	QIcon indentIcon(indentPix);
	m_addAction->setIcon(indentIcon);
	m_settingsAction->setIcon(indentIcon);
	m_exitAction->setIcon(indentIcon);

	// Build initial contents
	updateMenuContents();

	// Instead of letting Qt show the menu at the cursor, handle activation
	// and popup the menu above and left-justified to the tray icon.
	connect(m_trayIcon, &QSystemTrayIcon::activated, this, &TrayApp::onTrayIconActivated);
}

void TrayApp::onOpenSettings()
{
	QDialog dlg;
	dlg.setWindowTitle(QStringLiteral("Settings"));
	QVBoxLayout *v = new QVBoxLayout(&dlg);
	QCheckBox *cb = new QCheckBox(QStringLiteral("Start on User Login"));
	if (m_displayManager)
		cb->setChecked(m_displayManager->getStartOnUserLogin());
	v->addWidget(cb);

	// Repository link (clickable) - added before the NDI trademark line
	QLabel *repoLink = new QLabel();
	repoLink->setTextFormat(Qt::RichText);
	repoLink->setText(QStringLiteral("<a href=\"https://github.com/BitRate27/Pantalla\">https://github.com/BitRate27/Pantalla</a>"));
	repoLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
	repoLink->setOpenExternalLinks(true);
	v->addWidget(repoLink);

	// Trademark / link line above buttons
	QLabel *note = new QLabel();
	note->setTextFormat(Qt::RichText);
	note->setText(QStringLiteral(
		"NDI\u00AE is a registered trademark of Vizrt NDI AB - <a href=\"https://ndi.video\">https://ndi.video</a>"));
	note->setTextInteractionFlags(Qt::TextBrowserInteraction);
	note->setOpenExternalLinks(true);
	v->addWidget(note);

	// Parsec VDA link below the disclaimer
	QLabel *note2 = new QLabel();
	note2->setTextFormat(Qt::RichText);
	note2->setText(QStringLiteral(
		"<a href=\"https://github.com/nomi-san/parsec-vdd\">Parsec VDD</a> - Copyright (c)2023 Nguyen Duy <a href=\"mailto:wuuyi123@gmail.com\">wuuyi123@gmail.com</a>"));
	note2->setTextInteractionFlags(Qt::TextBrowserInteraction);
	note2->setOpenExternalLinks(true);
	v->addWidget(note2);

	QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
	connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
	v->addWidget(buttons);

	if (dlg.exec() == QDialog::Accepted) {
		bool startOnBoot = cb->isChecked();
		log_file("Settings changed: StartOnUserLogin=%d\n", startOnBoot);
		if (m_displayManager->getStartOnUserLogin() != startOnBoot) {
			m_displayManager->setStartOnUserLogin(startOnBoot);
		}
	}
}

void TrayApp::updateMenuContents()
{
	if (!m_menu)
		return;

	// If the Add action is already present in the menu, remove it so it won't be deleted by clear().
	if (m_addAction) {
		m_menu->removeAction(m_addAction);
	}
	// If the Settings action is present, remove it so it won't be deleted by clear().
	if (m_settingsAction) {
		m_menu->removeAction(m_settingsAction);
	}
	// If the Exit action is present, remove it so it won't be deleted by clear().
	if (m_exitAction) {
		m_menu->removeAction(m_exitAction);
	}

	m_menu->clear();

	// Add monitor entries as non-interactive left-justified widgets
	auto snaps = m_displayManager->getDisplaySnapshots();

	// Find last Parsec snapshot index (we only show the X for the last one)
	int lastParsecIndex = -1;
	for (int i = (int)snaps.size() -1; i >=0; --i) {
		if (snaps[i].screenName.find("Parsec") != std::string::npos) {
			lastParsecIndex = i;
			break;
		}
	}

	for (size_t idx =0; idx < snaps.size(); ++idx) {
		const auto &snap = snaps[idx];
		// Skip empty screen names
		if (snap.screenName.empty())
			continue;
		QWidgetAction *screenAction = new QWidgetAction(m_menu);

		// Create a QWidget with QHBoxLayout holding a QPushButton and QLabel
		QWidget *widget = new QWidget();
		QHBoxLayout *layout = new QHBoxLayout(widget);
		// Tight layout: small spacing and minimal margins to reduce vertical gaps between menu items
		layout->setSpacing(3);
		layout->setContentsMargins(4, 1, 4, 1);
		widget->setContentsMargins(0, 0, 0, 0);
		widget->setFixedHeight(22);

		// NDI button
		QPushButton *ndiButton = new QPushButton();
		ndiButton->setText(QStringLiteral("NDI"));
		ndiButton->setFixedSize(29, 18);
		// Green if hasSender, otherwise black
	 bool hasSender = snap.hasSender;
		ndiButton->setStyleSheet(
			QStringLiteral("background-color: %1; color: white; border: none; padding:4px;")
				.arg(hasSender ? QStringLiteral("green") : QStringLiteral("black")));
		layout->addWidget(ndiButton);

		// Connect button click to add/remove actions depending on current state
		// Capture the identifying information required to operate on the display later
		std::string screenName = snap.screenName;
		std::wstring deviceName = snap.deviceName;
		if (hasSender) {
			connect(ndiButton, &QPushButton::clicked, this, [this, screenName, deviceName]() {
				// Remove NDI sender by names (thread-safe)
				this->m_displayManager->removeNDISenderByNames(deviceName, screenName);
				this->updateMenuContents();
			});
		} else {
			connect(ndiButton, &QPushButton::clicked, this, [this, screenName, deviceName]() {
				// Add NDI sender by names (thread-safe)
				this->m_displayManager->addNDISender(deviceName, screenName);
				this->updateMenuContents();
			});
		}

		// Screen name label
		QLabel *label = new QLabel(QString::fromStdString(snap.screenName));
		label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		label->setMargin(0);
		layout->addWidget(label);

		// If this is the last Parsec display, add a right-justified 'X' button
		if ((int)idx == lastParsecIndex) {
			layout->addStretch();
			QPushButton *closeBtn = new QPushButton(QStringLiteral("X"));
			closeBtn->setFixedSize(18,18);
			closeBtn->setStyleSheet(
				QStringLiteral("background-color: transparent; color: red; border: none;"));

			std::string screenNameCopy = snap.screenName;
			std::wstring deviceNameCopy = snap.deviceName;
			connect(closeBtn, &QPushButton::clicked, this, [this, screenNameCopy, deviceNameCopy]() {
				// Remove virtual display by names (thread-safe)
				this->m_displayManager->removeVirtualDisplayByNames(deviceNameCopy, screenNameCopy);
				this->updateMenuContents();
			});
			layout->addWidget(closeBtn);
		}

		widget->setLayout(layout);
		screenAction->setDefaultWidget(widget);
		m_menu->addAction(screenAction);
	}

	if (m_addAction)
		m_menu->addAction(m_addAction);
	if (m_settingsAction)
		m_menu->addAction(m_settingsAction);
	if (m_exitAction)
		m_menu->addAction(m_exitAction);
}

void TrayApp::onAddNDISender(DisplayInfo *info)
{
	if (!info)
		return;
	log_file("Request to add NDI sender for %s\n", info->screenName.c_str());
	m_displayManager->addNDISender(info->deviceName, info->screenName);
	updateMenuContents();
}

void TrayApp::onRemoveNDISender(DisplayInfo *info)
{
	if (!info)
		return;
	log_file("Request to remove NDI sender for %s\n", info->screenName.c_str());
	m_displayManager->removeNDISender(info);
	updateMenuContents();
}

void TrayApp::onRemoveVirtualDisplay(DisplayInfo *info)
{
	if (!info)
		return;
	log_file("Request to remove virtual display for %s\n", info->screenName.c_str());
	m_displayManager->removeVirtualDisplay(info);
	updateMenuContents();
}
