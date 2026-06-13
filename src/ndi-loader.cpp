#include "global.h"
#include <ndi-loader.h>
#include <QDir>
#include <QLibrary>
#include <QString>
#define QT_TO_UTF8(str) str.toUtf8().constData()

QLibrary* loaded_lib = nullptr;

typedef const NDIlib_v6* (*NDIlib_v6_load_)(void);
const NDIlib_v6* load_ndilib()
{
	auto locations = QStringList();
	auto temp_path = QString(qgetenv(NDILIB_REDIST_FOLDER));
	if (!temp_path.isEmpty()) {
		locations << temp_path;
	}
#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
	locations << "/usr/lib";
	locations << "/usr/lib64";
	locations << "/usr/local/lib";
#endif
	auto lib_path = QString();
#if defined(Q_OS_LINUX)
	// Linux
	auto regex = QRegularExpression("libndi\\.so\\.(\\d+)");
	int max_version = 0;
#endif
	for (const auto& location : locations) {
		auto dir = QDir(location);
#if defined(Q_OS_LINUX)
		// Linux
		auto filters = QStringList("libndi.so.*");
		dir.setNameFilters(filters);
		auto file_names = dir.entryList(QDir::Files);
		for (const auto& file_name : file_names) {
			auto match = regex.match(file_name);
			if (match.hasMatch()) {
				int version = match.captured(1).toInt();
				if (version > max_version) {
					max_version = version;
					lib_path = dir.absoluteFilePath(file_name);
				}
			}
		}
#else
		// MacOS, Windows
		temp_path = QDir::cleanPath(dir.absoluteFilePath(NDILIB_LIBRARY_NAME));
		log_file( "load_ndilib: Trying '%s'\n", QT_TO_UTF8(QDir::toNativeSeparators(temp_path)));
		auto file_info = QFileInfo(temp_path);
		if (file_info.exists() && file_info.isFile()) {
			lib_path = temp_path;
			break;
		}
#endif
	}
	if (!lib_path.isEmpty()) {
		log_file( "load_ndilib: Found '%s'; attempting to load NDI library...\n",
			QT_TO_UTF8(QDir::toNativeSeparators(lib_path)));
		loaded_lib = new QLibrary(lib_path, nullptr);
		if (loaded_lib->load()) {
			log_file( "load_ndilib: NDI library loaded successfully\n");
			NDIlib_v6_load_ lib_load =
				reinterpret_cast<NDIlib_v6_load_>(loaded_lib->resolve("NDIlib_v6_load"));
			if (lib_load != nullptr) {
				log_file( "load_ndilib: NDIlib_v6_load found\n");
				return lib_load();
			}
			else {
				log_file( "Error loading the NDI Library from path: '%s'\n",
					QT_TO_UTF8(QDir::toNativeSeparators(lib_path)));
				log_file( "load_ndilib: ERROR: NDIlib_v6_load not found in loaded library\n");
				delete loaded_lib;
				loaded_lib = nullptr;
			}
		}
		else {
			log_file( "Error loading QLibrary with error: '%s'\n",
				QT_TO_UTF8(loaded_lib->errorString()));
			log_file( "load_ndilib: ERROR: QLibrary returned the following error: '%s'\n",
				QT_TO_UTF8(loaded_lib->errorString()));
			delete loaded_lib;
			loaded_lib = nullptr;
		}
	}

	log_file(
		"NDI library not found.\n");
	log_file( "load_ndilib: ERROR: Can't find the NDI library\n");
	return nullptr;
}