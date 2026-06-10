#include <QApplication>
#include "TrayApp.h"
#include "global.h"
#include <filesystem>
#include <algorithm>
const NDIlib_v6* ndiLib = nullptr;
std::atomic<bool> g_Running = true;
std::atomic<bool> g_SessionRunning = false;
static FILE *log_fp = nullptr;
void close_log()
{
	if (log_fp) {
		fclose(log_fp);
		log_fp = nullptr;
	}
}
void log_file(const char *fmt, ...)
{
	if (!log_fp)
		return;

	// Get current local time with milliseconds
	auto now = std::chrono::system_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) %1000;
	time_t tnow = std::chrono::system_clock::to_time_t(now);
	struct tm local_tm;
#if defined(_WIN32)
	localtime_s(&local_tm, &tnow);
#else
	localtime_r(&tnow, &local_tm);
#endif

	char timebuf[32];
	snprintf(timebuf, sizeof(timebuf), "%02d:%02d:%02d.%03d: ", local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec,
		 static_cast<int>(ms.count()));

	// Write timestamp prefix
	fputs(timebuf, log_fp);

	// Write the formatted message
	va_list ap;
	va_start(ap, fmt);
	vfprintf(log_fp, fmt, ap);
	va_end(ap);
	fflush(log_fp);
}

// Open log file
static void open_log_file(std::wstring /*ignored_path*/)
{
	// Use program data folder for logs
	std::filesystem::path dirPath(L"C:/ProgramData/Pantalla");
	try {
		if (!std::filesystem::exists(dirPath)) {
			std::filesystem::create_directories(dirPath);
		}
	} catch (...) {
		// ignore
	}

	// Create timestamp
	auto now = std::chrono::system_clock::now();
	time_t tnow = std::chrono::system_clock::to_time_t(now);
	struct tm local_tm;
#if defined(_WIN32)
	localtime_s(&local_tm, &tnow);
#else
	localtime_r(&tnow, &local_tm);
#endif
	wchar_t timestr[64];
	wcsftime(timestr, sizeof(timestr), L"%Y-%m-%d %H-%M-%S", &local_tm);
	std::wstring filename = std::wstring(timestr) + L".log";

	// Rotate logs: keep only most recent10 .log files
	try {
		std::vector<std::filesystem::directory_entry> logs;
		for (auto &entry : std::filesystem::directory_iterator(dirPath)) {
			if (!entry.is_regular_file()) continue;
			if (entry.path().extension() == L".log") {
				logs.push_back(entry);
			}
		}
		std::sort(logs.begin(), logs.end(), [](const std::filesystem::directory_entry &a, const std::filesystem::directory_entry &b) {
			return std::filesystem::last_write_time(a) < std::filesystem::last_write_time(b);
		});
		while (logs.size() >=10) {
			std::error_code ec;
			std::filesystem::remove(logs.front().path(), ec);
			if (ec) break;
			logs.erase(logs.begin());
		}
	} catch (...) {
		// ignore
	}

	std::filesystem::path fullPath = dirPath / filename;
	char log_path_utf8[MAX_PATH];
	std::wstring fullPathW = fullPath.wstring();
	wcstombs_s(nullptr, log_path_utf8, sizeof(log_path_utf8), fullPathW.c_str(), _TRUNCATE);
	fopen_s(&log_fp, log_path_utf8, "a");
	if (!log_fp) {
		fprintf(stderr, "Failed to open log file: %s\n", log_path_utf8);
	}
}
int main(int argc, char *argv[])
{
	WCHAR modulePath[MAX_PATH] = {0};
	if (GetModuleFileNameW(NULL, modulePath, MAX_PATH) >0) {
		// Extract directory
		WCHAR dirPath[MAX_PATH] = {0};
		wcscpy_s(dirPath, modulePath);
		WCHAR *lastSlash = wcsrchr(dirPath, L'\\');
		if (lastSlash) {
			*lastSlash = L'\0';
		} else {
			// fallback to current directory
			wcscpy_s(dirPath, L".");
		}

		open_log_file(dirPath);
	}

	ndiLib = load_ndilib();
	if (!ndiLib) {
		log_file("Failed to load NDI library\n");
		return -1;
	}
	if (!ndiLib->initialize()) {
		log_file("Failed to initialize NDI library\n");
		return -1;
	}
	log_file("NDI library loaded successfully\n");

	QApplication app(argc, argv);

	// Prevent Qt from quitting when the last window is closed —
	// we live entirely in the system tray.
	app.setQuitOnLastWindowClosed(false);

	TrayApp tray;
	tray.show();
	

	
	return app.exec();
}
