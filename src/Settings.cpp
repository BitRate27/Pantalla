#include "Settings.h"
#include "global.h"
#include <filesystem>
#include <fstream>
#include <windows.h>

bool SaveSettings(const std::vector<DisplayInfo *> &displays, const std::vector<DisplayInfo *> &virtualDisplays, bool startOnUserLogin)
{
    try
    {
        std::filesystem::create_directories(SETTINGS_DIR);
        ::nlohmann::json j;
        j["displays"] = ::nlohmann::json::array();
        for (const auto &d : displays)
        {
            ::nlohmann::json jd;
            jd["vddIndex"] = d->vddIndex;
            jd["deviceName"] = std::string(d->deviceName.begin(), d->deviceName.end());
            jd["screenName"] = std::string(d->screenName.begin(), d->screenName.end());
            jd["sendNDI"] = d->sendNDI;
            j["displays"].push_back(jd);
        }
        j["virtualDisplays"] = ::nlohmann::json::array();
        for (const auto &d : virtualDisplays)
        {
            ::nlohmann::json jd;
            jd["vddIndex"] = d->vddIndex;
            jd["deviceName"] = std::string(d->deviceName.begin(), d->deviceName.end());
            jd["screenName"] = std::string(d->screenName.begin(), d->screenName.end());
            jd["sendNDI"] = d->sendNDI;
            j["virtualDisplays"].push_back(jd);
        }
        j["startOnUserLogin"] = startOnUserLogin;
        std::ofstream ofs(SETTINGS_FILE);
        ofs << j.dump(4);
        ofs.close();
        log_file("Saved settings to %ls\n", SETTINGS_FILE);
        return true;
    }
    catch (std::exception &e)
    {
        log_file("Failed to save settings: %s\n", e.what());
        return false;
    }
}

bool LoadSettings(std::vector<DisplayInfo *> &displays, std::vector<DisplayInfo *> &virtualDisplays, bool &startOnUserLogin)
{
    try
    {
        if (!std::filesystem::exists(SETTINGS_FILE))
        {
            log_file("No settings file found\n");
            startOnUserLogin = false;
            return false;
        }
        std::ifstream ifs(SETTINGS_FILE);
        ::nlohmann::json j;
        ifs >> j;
        ifs.close();

        startOnUserLogin = j.value("startOnUserLogin", false);
        getDisplaysFromSettings(j, "displays", displays);
        getDisplaysFromSettings(j, "virtualDisplays", virtualDisplays);

        log_file("Loaded settings from %ls\n", SETTINGS_FILE);
        return true;
    }
    catch (std::exception &e)
    {
        log_file("Failed to load settings: %s\n", e.what());
        return false;
    }
}

void getDisplaysFromSettings(::nlohmann::json &j, const std::string &displayType, std::vector<DisplayInfo *> &displays)
{
    if (j.contains(displayType))
    {
        for (const auto &jd : j[displayType])
        {
            int vddIndex = jd.value("vddIndex", -1);
            std::wstring deviceName;
            if (jd.contains("deviceName"))
            {
                std::string dn = jd["deviceName"].get<std::string>();
                deviceName.assign(dn.begin(), dn.end());
            }
            std::string screenName = jd.value("screenName", std::string());
            bool sendNDI = jd.value("sendNDI", false);
            NDISender *ndiSender = nullptr;
            DisplayInfo *info = nullptr;
            for (const auto display : displays)
            {
                if (display->deviceName == deviceName)
                {
                    display->sendNDI = sendNDI;
                    display->vddIndex = vddIndex;
                    info = display;
                    break;
                }
            }
            if (!info)
            {
                info = new DisplayInfo{vddIndex, L"", screenName, ndiSender, sendNDI};
                displays.push_back(info);
            }
        }
    }
}

// Add/remove Run on startup entry for current user
bool SetRunOnStartup(bool enable)
{
    HKEY hKey = NULL;
    LONG res = RegOpenKeyExW(HKEY_CURRENT_USER, LR"(Software\Microsoft\Windows\CurrentVersion\Run)", 0, KEY_WRITE, &hKey);
    if (res != ERROR_SUCCESS)
    {
        log_file("Failed to open Run key: %ld\n", res);
        return false;
    }

    std::wstring valueName = L"Pantalla";
    if (enable)
    {
        WCHAR modulePath[MAX_PATH] = {0};
        if (GetModuleFileNameW(NULL, modulePath, MAX_PATH) == 0)
        {
            log_file("GetModuleFileNameW failed when setting startup: %ld\n", GetLastError());
            RegCloseKey(hKey);
            return false;
        }
        std::wstring cmd = L"\"" + std::wstring(modulePath) + L"\"";
        DWORD dataSize = static_cast<DWORD>((cmd.size() + 1) * sizeof(WCHAR));
        res = RegSetValueExW(hKey, valueName.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE *>(cmd.c_str()), dataSize);
        if (res != ERROR_SUCCESS)
        {
            log_file("Failed to set Run value: %ld\n", res);
            RegCloseKey(hKey);
            return false;
        }
        RegCloseKey(hKey);
        return true;
    }
    else
    {
        res = RegDeleteValueW(hKey, valueName.c_str());
        if (res != ERROR_SUCCESS && res != ERROR_FILE_NOT_FOUND)
        {
            log_file("Failed to delete Run value: %ld\n", res);
            RegCloseKey(hKey);
            return false;
        }
        RegCloseKey(hKey);
        return true;
    }
}
