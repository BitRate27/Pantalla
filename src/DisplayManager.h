#pragma once
#include <string>
#include <vector>
#include <mutex>
#include "global.h"
#include "List.h"
#include "NDISender.h"
#include "parsec-vdd.h"
#include "json.hpp"

struct DisplayInfo
{
	int vddIndex;
	std::wstring deviceName;
	std::string screenName;
	NDISender *ndiSender;
	bool sendNDI;
};

// Read-only snapshot safe to pass between threads without locking
struct DisplaySnapshot
{
	int vddIndex;
	std::wstring deviceName;
	std::string screenName;
	bool sendNDI;
	bool hasSender;
};

class DisplayManager
{
public:
	DisplayManager();
	~DisplayManager();
	std::vector<DisplayInfo *> getDisplays();
	// New thread-safe snapshot API returning copies of display state
	std::vector<DisplaySnapshot> getDisplaySnapshots();
	DisplayInfo *getDisplayInfo(std::vector<DisplayInfo *> displays, std::string screenName);
	bool addVirtualDisplay();
	bool addNDISender(std::wstring deviceName, std::string ndiName);

	bool removeNDISender(DisplayInfo *display);
	// Removal helpers that accept names (safe for UI callers)
	bool removeNDISenderByNames(const std::wstring &deviceName, const std::string &screenName);
	bool removeVirtualDisplay(DisplayInfo *display);
	bool removeVirtualDisplayByNames(const std::wstring &deviceName, const std::string &screenName);

	void updateVirtualScreenNames();
	void changeScreenName(DisplayInfo *display, std::string screenName);

	// helper queries
	bool nameExists(std::vector<std::wstring> list, std::wstring name);
	bool displayExists(DisplayInfo *display);

	// Start on User Login setting
	void setStartOnUserLogin(bool v);
	bool getStartOnUserLogin() const { return m_startOnUserLogin; }

	// load helper
	void getDisplaysFromSettings(::nlohmann::json &j, const std::string &displayType, std::vector<DisplayInfo *> &displays);
	bool setMonitorResolution(const std::wstring &deviceName,
							  DWORD width, DWORD height,
							  DWORD refreshRate,
							  DWORD bitDepth);
	bool setMonitorPosition(const std::wstring &deviceName, LONG x, LONG y);

private:
	void displayMonitorThread();

	bool m_displayMonitorRunning;
	std::vector<std::string> m_screenNames;
	std::vector<std::wstring> m_deviceNames;
	std::thread m_displayMonitorThread;
	std::vector<DisplayInfo *> m_displays;
	std::vector<DisplayInfo *> m_virtualDisplays;
	int m_newVddIndexAdded;
	HANDLE m_vdd;
	bool m_update_running = false;
	std::thread m_update_thread;

	mutable std::recursive_mutex m_mutex; // protects m_displays and related state, recursive for reentrancy

	// persisted setting
	bool m_startOnUserLogin = false;
};
