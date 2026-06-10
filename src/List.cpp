// VirtualDisplay
#include <windows.h>
#include <iostream>
#include <setupapi.h>
#include <devguid.h>
#include <cfgmgr32.h>
#include "global.h"
#include "FrameReader.h"
#include "DXGICapture.h"
#include "ndi-loader.h"
#include "parsec-vdd.h"
#include "List.h"
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <conio.h>
#include <string>
#include <vector>
#include <cwctype>
#include <QGuiApplication>
#include <QScreen>
#include <QList>
#include <QString>
#include <QTimer>
#include <QMetaObject>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "cfgmgr32.lib")
QString FormatScreenName(QScreen *screen);

// Helper to find MONITORINFOEX::szDevice for a given DISPLAY_DEVICE name
struct MonitorFindCtx {
	const WCHAR *targetName;
	WCHAR foundDevice[32];
	bool found;
};

static BOOL CALLBACK MonitorFindProc(HMONITOR hMon, HDC, LPRECT, LPARAM dwData)
{
	MonitorFindCtx *ctx = reinterpret_cast<MonitorFindCtx *>(dwData);
	MONITORINFOEXW mi;
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	if (GetMonitorInfoW(hMon, &mi)) {
		// Match if monitor's mi.szDevice is substring of targetName or vice versa
		if ((ctx->targetName && wcsstr(ctx->targetName, mi.szDevice) != nullptr) ||
		    (mi.szDevice[0] && wcsstr(mi.szDevice, ctx->targetName) != nullptr)) {
			wcscpy_s(ctx->foundDevice, mi.szDevice);
			ctx->found = true;
			return FALSE; // stop enumeration
		}
	}
	return TRUE; // continue
}
static std::wstring GetMonitorInfoDeviceNameForDisplayDevice(const WCHAR *displayDeviceName)
{
	MonitorFindCtx ctx = {};
	ctx.targetName = displayDeviceName;
	ctx.found = false;
	ctx.foundDevice[0] = L'\0';
	EnumDisplayMonitors(NULL, NULL, MonitorFindProc, reinterpret_cast<LPARAM>(&ctx));
	if (ctx.found)
		return std::wstring(ctx.foundDevice);
	return std::wstring();
};

// Return list of monitor DeviceName strings for Parsec monitors
std::vector<std::wstring> ListDisplayDevices()
{
	std::vector<std::wstring> activeList;

	DISPLAY_DEVICE adapterDevice = {};
	adapterDevice.cb = sizeof(DISPLAY_DEVICE);

	// --- Pass1: Enumerate adapters ---
	for (DWORD adapterIndex = 0; EnumDisplayDevices(NULL, adapterIndex, &adapterDevice, 0); ++adapterIndex) {
		// --- Pass2: Enumerate monitors on this adapter ---
		DISPLAY_DEVICE monitorDevice = {};
		monitorDevice.cb = sizeof(DISPLAY_DEVICE);

		for (DWORD monitorIndex = 0;
		     EnumDisplayDevices(adapterDevice.DeviceName, monitorIndex, &monitorDevice, 0); ++monitorIndex) {
			// Check for Parsec VDD adapter by DeviceID
			if (adapterDevice.DeviceID[0] != L'\0') {
				if (!(wcsstr(adapterDevice.DeviceID, L"Root\\Parsec\\VDA") != nullptr ||
				      _wcsicmp(adapterDevice.DeviceID, L"Root\\Parsec\\VDA") == 0)) {
					//continue; // Only want Parsec VDAs
				}
			}
			if (monitorDevice.StateFlags & DISPLAY_DEVICE_ACTIVE) {
				// Get MONITORINFOEX::szDevice for this monitorDevice
				std::wstring miDevice =
					GetMonitorInfoDeviceNameForDisplayDevice(monitorDevice.DeviceName);

				// Add monitor DeviceName to active list
				activeList.push_back(std::wstring(miDevice));
			}
			// reset monitorDevice for next iteration
			ZeroMemory(&monitorDevice, sizeof(monitorDevice));
			monitorDevice.cb = sizeof(DISPLAY_DEVICE);
		}

		// reset adapterDevice for next adapter (EnumDisplayDevices will overwrite fields)
		ZeroMemory(&adapterDevice, sizeof(adapterDevice));
		adapterDevice.cb = sizeof(DISPLAY_DEVICE);
	}

	return activeList;
}

QString FormatScreenName(QScreen *screen) 
{
	QRect screenGeometry = screen->geometry();
	qreal screenPixelRatio = screen->devicePixelRatio();
	QString name = "";
#if defined(__APPLE__) || defined(_WIN32)
	name = screen->name();
	// Remove any backslash characters that may appear in the screen name
	name.remove('\\');
#else
	name = screen->model().simplified();

	if (name.length() > 1 && name.endsWith("-"))
		name.chop(1);
#endif
	name = name.simplified();

	int screenPixelWidth = std::round((screenGeometry.width() * screenPixelRatio) * 0.5f) * 2;
	int screenPixelHeight = std::round((screenGeometry.height() * screenPixelRatio) * 0.5f) * 2;

	QString str = QString("%1: %2x%3 @ %4,%5")
			      .arg(name, QString::number(screenPixelWidth), QString::number(screenPixelHeight),
				   QString::number(screenGeometry.x()), QString::number(screenGeometry.y()));
	return str;
}

std::vector<std::string> ListOBSProjectorMonitorsFormatted()
{
	std::vector<std::string> projectorsFormatted;
	QList<QScreen *> screens = QGuiApplication::screens();
	for (int i = 0; i < screens.size(); i++) {
		QScreen *screen = screens[i];
		QString str = FormatScreenName(screen);
		projectorsFormatted.push_back(str.toStdString());
	}
	return projectorsFormatted;
}







