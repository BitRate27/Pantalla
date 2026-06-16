// DisplayManager.cpp
#include <windows.h>
#include "global.h"
#include "DisplayManager.h"
#include "Settings.h"
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <vector>
#include "parsec-vdd.h"
#include "json.hpp"

DisplayManager::DisplayManager() : m_displays(), m_virtualDisplays(), m_displayMonitorRunning(false), m_displayMonitorThread()
{
	// Check driver status.
	parsec_vdd::DeviceStatus status =
		parsec_vdd::QueryDeviceStatus(&parsec_vdd::VDD_CLASS_GUID, parsec_vdd::VDD_HARDWARE_ID);
	if (status != parsec_vdd::DEVICE_OK)
	{
		log_file("Parsec VDD device is not OK, got status %d.\n", status);
		return;
	}

	// Obtain device handle.
	m_vdd = parsec_vdd::OpenDeviceHandle(&parsec_vdd::VDD_ADAPTER_GUID);
	if (m_vdd == NULL || m_vdd == INVALID_HANDLE_VALUE)
	{
		log_file("Failed to obtain the device handle.\n");
		return;
	}

	// Side thread for updating vdd.
	// The thread will be stopped in the d-tor.
	m_update_running = true;
	m_update_thread = std::thread([this]
								  {
		log_file("Starting VDD update thread...\n");
		while (this->m_update_running) {
			parsec_vdd::VddUpdate(this->m_vdd);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		log_file("Update not running\n"); });
	m_update_thread.detach();

	m_screenNames = ListOBSProjectorMonitorsFormatted();
	m_deviceNames = ListDisplayDevices();

	if (m_screenNames.size() == m_deviceNames.size())
	{
		for (int i = 0; i < m_screenNames.size(); i++)
		{
			DisplayInfo *info = new DisplayInfo{-1, m_deviceNames[i], m_screenNames[i], nullptr, false};
			m_displays.push_back(info);
		}
	}

	bool startOnUserLogin = false;
	LoadSettings(m_displays, m_virtualDisplays, startOnUserLogin);

	// Create NDI senders for displays that have sendNDI set to true
	for (const auto &d : m_displays)
	{
		if (displayExists(d) && d->sendNDI)
		{
			d->ndiSender = new NDISender(d->deviceName, d->screenName);
		}
	}

	// Add virtual displays to VDD and log their indices
	for (const auto &d : m_virtualDisplays)
	{
		auto index = parsec_vdd::VddAddDisplay(m_vdd);
		log_file("Added virtual display for %s with index %d\n", d->screenName.c_str(), index);
		if (d->vddIndex != index)
			log_file("Warning: VDD index mismatch for %s. Expected %d, got %d\n",
					 d->screenName.c_str(), d->vddIndex, index);
		d->vddIndex = index;
	}
	m_displayMonitorRunning = true;
	m_displayMonitorThread = std::thread(&DisplayManager::displayMonitorThread, this);
	// Do not detach: keep the thread joinable so the destructor can stop and join it safely.
};

DisplayManager::~DisplayManager()
{
	log_file("Destroying DisplayManager\n");
	// Stop display monitor thread if running
	if (m_displayMonitorRunning)
	{
		m_displayMonitorRunning = false;
		std::this_thread::sleep_for(std::chrono::milliseconds(101));
		if (m_displayMonitorThread.joinable())
		{
			m_displayMonitorThread.join();
		}
	}
	for (auto display : getDisplays())
	{
		if (display->ndiSender)
		{
			log_file("Deleting NDISender for display %s\n", display->screenName.c_str());
			delete display->ndiSender;
		}
		if (display->vddIndex != -1)
		{
			log_file("Removing VDD display for %s with index %d\n", display->screenName.c_str(), display->vddIndex);
			parsec_vdd::VddRemoveDisplay(m_vdd, display->vddIndex);
		}
		delete display;
	}
	m_displays.clear();
	m_update_running = false;
	std::this_thread::sleep_for(std::chrono::milliseconds(101));
	if (m_update_thread.joinable())
		m_update_thread.join();
	parsec_vdd::CloseDeviceHandle(m_vdd);
};

bool DisplayManager::nameExists(std::vector<std::wstring> list, std::wstring name)
{
	return std::find(list.begin(), list.end(), name) != list.end();
}

bool DisplayManager::displayExists(DisplayInfo *display)
{
	auto current_deviceNames = ListDisplayDevices();
	return nameExists(current_deviceNames, display->deviceName);
}

DisplayInfo *DisplayManager::getDisplayInfo(std::vector<DisplayInfo *> displays, std::string screenName)
{
	for (auto display : displays)
	{
		if (display->screenName == screenName)
		{
			return display;
		}
	}
	return nullptr;
};
std::vector<DisplayInfo *> DisplayManager::getDisplays()
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	std::vector<DisplayInfo *> allDisplays;
	for (const auto d : m_displays)
	{
		if (displayExists(d))
			allDisplays.push_back(d);
	}
	for (const auto d : m_virtualDisplays)
	{
		if (displayExists(d))
			allDisplays.push_back(d);
	}
	return allDisplays;
}

// Thread-safe snapshot API
std::vector<DisplaySnapshot> DisplayManager::getDisplaySnapshots()
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	std::vector<DisplaySnapshot> snaps;
	for (const auto d : m_displays)
	{
		if (!displayExists(d))
			continue;
		DisplaySnapshot s;
		s.vddIndex = d->vddIndex;
		s.deviceName = d->deviceName;
		s.screenName = d->screenName;
		s.sendNDI = d->sendNDI;
		s.hasSender = (d->ndiSender != nullptr);
		snaps.push_back(std::move(s));
	}
	for (const auto d : m_virtualDisplays)
	{
		if (!displayExists(d))
			continue;
		DisplaySnapshot s;
		s.vddIndex = d->vddIndex;
		s.deviceName = d->deviceName;
		s.screenName = d->screenName;
		s.sendNDI = d->sendNDI;
		s.hasSender = (d->ndiSender != nullptr);
		snaps.push_back(std::move(s));
	}
	return snaps;
}

// Remove NDI sender by matching names (thread-safe)
bool DisplayManager::removeNDISenderByNames(const std::wstring &deviceName, const std::string &screenName)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	for (const auto &display : m_displays)
	{
		if (display->deviceName == deviceName && display->screenName == screenName)
		{
			return removeNDISender(display);
		}
	}
	for (const auto &display : m_virtualDisplays)
	{
		if (display->deviceName == deviceName && display->screenName == screenName)
		{
			return removeNDISender(display);
		}
	}
	return false;
}

// Remove virtual display by matching names (thread-safe)
bool DisplayManager::removeVirtualDisplayByNames(const std::wstring &deviceName, const std::string &screenName)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	for (const auto &display : m_virtualDisplays)
	{
		if (display->deviceName == deviceName && display->screenName == screenName)
		{
			// call existing removal logic which will handle ndiSender deletion and RemoveDisplay
			removeVirtualDisplay(display);
			return true;
		}
	}
	for (const auto &display : m_displays)
	{
		if (display->deviceName == deviceName && display->screenName == screenName)
		{
			// If it's a non-virtual display, fall back to removing NDI sender if present
			if (display->ndiSender)
			{
				return removeNDISender(display);
			}
			break;
		}
	}
	return false;
}

bool DisplayManager::addVirtualDisplay()
{
	int vddIndex = parsec_vdd::VddAddDisplay(m_vdd);
	DisplayInfo *info = new DisplayInfo{vddIndex, L"", "", nullptr, true};
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		m_virtualDisplays.push_back(info);
		// persist the updated list
		SaveSettings(m_displays, m_virtualDisplays, m_startOnUserLogin);
	}
	return true;
}

bool DisplayManager::addNDISender(std::wstring deviceName, std::string ndiName)
{
	log_file("addNDISender(names): Creating DisplayInfo for %s with device %ls\n", ndiName.c_str(),
			 deviceName.c_str());
	bool found = false;
	NDISender *ndiSender = new NDISender(deviceName, ndiName);
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		for (const auto &display : m_displays)
		{
			if (display->deviceName == deviceName)
			{
				display->ndiSender = ndiSender;
				display->sendNDI = true;
				found = true;
				break;
			}
		}
		if (!found)
		{
			for (const auto &display : m_virtualDisplays)
			{
				if (display->deviceName == deviceName)
				{
					display->ndiSender = ndiSender;
					display->sendNDI = true;
					found = true;
					break;
				}
			}
		}
		if (found)
		{
			// persist the change
			SaveSettings(m_displays, m_virtualDisplays, m_startOnUserLogin);
		}
	}
	if (!found)
	{
		// if nothing matched, delete the created sender to avoid leak
		delete ndiSender;
	}
	return found;
}

bool DisplayManager::removeNDISender(DisplayInfo *display)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	// If the DisplayInfo has an ndiSender, delete it
	if (display->ndiSender)
	{
		delete display->ndiSender;
		display->ndiSender = nullptr;
		display->sendNDI = false;
		SaveSettings(m_displays, m_virtualDisplays, m_startOnUserLogin);
		return true;
	}
	return false;
}

void DisplayManager::updateVirtualScreenNames()
{
	auto current_deviceNames = ListDisplayDevices();
	auto current_screenNames = ListOBSProjectorMonitorsFormatted();

	if (current_deviceNames.size() != current_screenNames.size())
		return;

	for (int i = 0; i < current_deviceNames.size(); i++)
	{
		for (auto &d : m_virtualDisplays)
		{
			if ((d->deviceName == current_deviceNames[i]) && (d->screenName != current_screenNames[i]))
			{
				changeScreenName(d, current_screenNames[i]);
				break;
			}
		}
	}
}

void DisplayManager::changeScreenName(DisplayInfo *display, std::string screenName)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	display->screenName = screenName;
	if (display->ndiSender != nullptr)
	{
		delete display->ndiSender;
		display->ndiSender = new NDISender(display->deviceName, display->screenName);
	}
}

bool DisplayManager::setMonitorResolution(const std::wstring &deviceName,
										  DWORD width, DWORD height,
										  DWORD refreshRate = 60,
										  DWORD bitDepth = 32)
{
	DEVMODE dm = {};
	dm.dmSize = sizeof(DEVMODE);

	// Read current settings first so unchanged fields stay valid
	if (!EnumDisplaySettingsEx(deviceName.c_str(), ENUM_CURRENT_SETTINGS, &dm, 0))
		return false;

	dm.dmPelsWidth = width;
	dm.dmPelsHeight = height;
	dm.dmDisplayFrequency = refreshRate;
	dm.dmBitsPerPel = bitDepth;
	dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT |
				  DM_DISPLAYFREQUENCY | DM_BITSPERPEL;

	// Test first  avoids a jarring failed switch
	LONG result = ChangeDisplaySettingsEx(deviceName.c_str(), &dm,
										  nullptr, CDS_TEST, nullptr);
	if (result != DISP_CHANGE_SUCCESSFUL)
		return false;

	// Apply and persist to registry
	result = ChangeDisplaySettingsEx(deviceName.c_str(), &dm,
									 nullptr, CDS_UPDATEREGISTRY | CDS_NORESET,
									 nullptr);
	return result == DISP_CHANGE_SUCCESSFUL;
}

bool DisplayManager::setMonitorPosition(const std::wstring &deviceName, LONG x, LONG y)
{
	DEVMODE dm = {};
	dm.dmSize = sizeof(DEVMODE);

	if (!EnumDisplaySettingsEx(deviceName.c_str(), ENUM_CURRENT_SETTINGS, &dm, 0))
		return false;

	dm.dmPosition.x = x;
	dm.dmPosition.y = y;
	dm.dmFields = DM_POSITION;

	LONG result = ChangeDisplaySettingsEx(deviceName.c_str(), &dm,
										  nullptr,
										  CDS_UPDATEREGISTRY | CDS_NORESET,
										  nullptr);
	return result == DISP_CHANGE_SUCCESSFUL;
}

// After staging all monitors, commit the layout:
void CommitDisplayChanges()
{
	ChangeDisplaySettingsEx(nullptr, nullptr, nullptr, 0, nullptr);
}
bool DisplayManager::removeVirtualDisplay(DisplayInfo *display)
{

	// If the DisplayInfo has an ndiSender, delete it
	if (display->ndiSender)
	{
		delete display->ndiSender;
		display->ndiSender = nullptr;
		display->sendNDI = false;
	}
	if (display->vddIndex != -1)
	{
		log_file("Removing VDD display for %s with index %d\n", display->screenName.c_str(), display->vddIndex);
		parsec_vdd::VddRemoveDisplay(m_vdd, display->vddIndex);
		display->vddIndex = -1;
	}
	m_virtualDisplays.erase(std::remove(m_virtualDisplays.begin(), m_virtualDisplays.end(), display),
							m_virtualDisplays.end());
	delete display;
	SaveSettings(m_displays, m_virtualDisplays, m_startOnUserLogin);
	return false;
}

void DisplayManager::displayMonitorThread()
{
	log_file("Starting display monitor thread2...\n");
	while (m_displayMonitorRunning)
	{
		auto current_screenNames = ListOBSProjectorMonitorsFormatted();
		if (current_screenNames.size() != m_screenNames.size())
		{
			auto current_deviceNames = ListDisplayDevices();
			log_file("----Screen count changed! Previous: %d, Current: %d\n", (int)m_screenNames.size(),
					 (int)current_screenNames.size());
			for (int i = 0; i < current_screenNames.size(); i++)
			{
				log_file("Detected screen: %s\n", current_screenNames[i].c_str());
			}
			for (int i = 0; i < current_deviceNames.size(); i++)
			{
				log_file("Detected device: %ls\n", current_deviceNames[i].c_str());
			}
			log_file("----\n");
			if (current_screenNames.size() != current_deviceNames.size())
			{
				log_file(
					"Warning: screen count and device count do not match! Screens: %d, Devices: %d\n",
					(int)current_screenNames.size(), (int)current_deviceNames.size());
				break;
			}
			// Find the new screen that was added
			for (int i = 0; i < current_screenNames.size(); i++)
			{
				if (std::find(m_screenNames.begin(), m_screenNames.end(), current_screenNames[i]) ==
					m_screenNames.end())
				{
					log_file("New screen detected: %s device: %ls\n",
							 current_screenNames[i].c_str(), current_deviceNames[i].c_str());
					if (current_screenNames[i].find("Parsec") ==
						std::string::npos)
					{ // not a Parsec display
						log_file("New screen is not a Parsec display\n");
						auto display = getDisplayInfo(m_displays, current_screenNames[i]);
						if (display == nullptr)
						{
							log_file("Adding it as a non-NDI sender\n");
							DisplayInfo *info = new DisplayInfo{-1, current_deviceNames[i],
																current_screenNames[i],
																nullptr, false};
							{
								std::lock_guard<std::recursive_mutex> lock(m_mutex);
								m_displays.push_back(info);
								SaveSettings(m_displays, m_virtualDisplays, m_startOnUserLogin);
							}
						}
						else
						{
							if (display->sendNDI)
							{
								log_file("Turning on NDI for newly added display\n");
								std::lock_guard<std::recursive_mutex> lock(m_mutex);
								addNDISender(current_deviceNames[i],
											 current_screenNames[i]);
							}
						}
					}
					else
					{
						log_file("New screen is a Parsec display\n");
						for (const auto &display : m_virtualDisplays)
						{
							if (display->screenName == "" && display->vddIndex > -1)
							{
								log_file("Adding virtual screen %s\n",
										 current_screenNames[i].c_str());
								{
									std::lock_guard<std::recursive_mutex> lock(
										m_mutex);
									display->screenName = current_screenNames[i];
									display->deviceName = current_deviceNames[i];
									SaveSettings(m_displays, m_virtualDisplays, m_startOnUserLogin);
									break;
								}
							}
							else if (display->deviceName == L"" && (strncmp(display->screenName.c_str(), "ParsecVDA", 9) == 0))
							{
								log_file("Found saved virtual display %s\n",
										 current_screenNames[i].c_str());
								{
									std::lock_guard<std::recursive_mutex> lock(
										m_mutex);
									display->deviceName = current_deviceNames[i];
									display->screenName = current_screenNames[i];
									SaveSettings(m_displays, m_virtualDisplays, m_startOnUserLogin);
									break;
								}
							}
						}
					}
				}
			}
			m_screenNames = current_screenNames;
			m_deviceNames = current_deviceNames;
		}
		else
		{
			for (auto &display : m_displays)
			{
				if (display->sendNDI && display->ndiSender == nullptr && display->screenName != "" && display->deviceName != L"")
				{
					std::lock_guard<std::recursive_mutex> lock(m_mutex);
					NDISender *ndiSender = new NDISender(display->deviceName, display->screenName);
					display->ndiSender = ndiSender;
				}
			}
			for (auto &display : m_virtualDisplays)
			{
				if (display->sendNDI && display->ndiSender == nullptr && display->screenName != "" && display->deviceName != L"")
				{
					std::lock_guard<std::recursive_mutex> lock(m_mutex);
					NDISender *ndiSender = new NDISender(display->deviceName, display->screenName);
					display->ndiSender = ndiSender;
				}
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
};

void DisplayManager::setStartOnUserLogin(bool v)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	m_startOnUserLogin = v;
	// Update registry Run key
	if (!SetRunOnStartup(v))
	{
		log_file("Failed to update startup registry key\n");
	}
	SaveSettings(m_displays, m_virtualDisplays, m_startOnUserLogin);
}
