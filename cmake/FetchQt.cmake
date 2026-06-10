# cmake/FetchQt.cmake
#
# Downloads a specific Qt version at CMake configure-time using aqtinstall
# (https://github.com/miurahr/aqtinstall) and caches the result inside the
# source tree under <root>/qt/<version>/msvc2019_64/.
#
# After this script runs, CMAKE_PREFIX_PATH is updated so that the normal
#   find_package(Qt6 ...)
# call in CMakeLists.txt will find the local copy without any manual setup.
#
# The download is skipped on subsequent configures if the directory already
# exists, so it only costs time once.
# -----------------------------------------------------------------------

set(FETCH_QT_VERSION      "6.7.3"          CACHE STRING "Qt version to download")
set(FETCH_QT_HOST         "windows"        CACHE STRING "aqt host (windows/linux/mac)")
set(FETCH_QT_TARGET       "desktop"        CACHE STRING "aqt target")
set(FETCH_QT_ARCH         "win64_msvc2019_64" CACHE STRING "aqt architecture/compiler kit")

# Where the downloaded Qt tree will live (committed to .gitignore)
set(FETCH_QT_BASE_DIR     "${CMAKE_SOURCE_DIR}/qt")
set(FETCH_QT_VERSIONED_DIR "${FETCH_QT_BASE_DIR}/${FETCH_QT_VERSION}")
set(FETCH_QT_PREFIX_DIR   "${FETCH_QT_VERSIONED_DIR}/msvc2019_64")

# -----------------------------------------------------------------------
# Sentinel: skip everything if Qt is already present from a previous run
# -----------------------------------------------------------------------
if(EXISTS "${FETCH_QT_PREFIX_DIR}/bin/qmake.exe")
    message(STATUS "[FetchQt] Found cached Qt ${FETCH_QT_VERSION} at ${FETCH_QT_PREFIX_DIR}")
else()
    message(STATUS "[FetchQt] Qt ${FETCH_QT_VERSION} not found — downloading via aqtinstall…")

    # --- 1. Locate Python (3.x) --------------------------------------------
    find_program(PYTHON_EXECUTABLE
        NAMES python3 python
        DOC   "Python 3 interpreter required by aqtinstall"
    )
    if(NOT PYTHON_EXECUTABLE)
        message(FATAL_ERROR
            "[FetchQt] Python 3 not found on PATH.\n"
            "Install Python 3 (https://www.python.org) and ensure it is on PATH,\n"
            "or set PYTHON_EXECUTABLE to its full path."
        )
    endif()
    message(STATUS "[FetchQt] Using Python: ${PYTHON_EXECUTABLE}")

    # --- 2. Install / upgrade aqtinstall -----------------------------------
    message(STATUS "[FetchQt] Installing/upgrading aqtinstall…")
    execute_process(
        COMMAND "${PYTHON_EXECUTABLE}" -m pip install --upgrade aqtinstall
        RESULT_VARIABLE _aqt_pip_result
        OUTPUT_VARIABLE _aqt_pip_output
        ERROR_VARIABLE  _aqt_pip_error
    )
    if(NOT _aqt_pip_result EQUAL 0)
        message(FATAL_ERROR
            "[FetchQt] Failed to install aqtinstall via pip.\n"
            "stdout: ${_aqt_pip_output}\n"
            "stderr: ${_aqt_pip_error}"
        )
    endif()

    # --- 3. Download Qt ----------------------------------------------------
    message(STATUS
        "[FetchQt] Downloading Qt ${FETCH_QT_VERSION} "
        "(${FETCH_QT_HOST} / ${FETCH_QT_TARGET} / ${FETCH_QT_ARCH})…"
    )
    message(STATUS "[FetchQt] Output directory: ${FETCH_QT_BASE_DIR}")
    message(STATUS "[FetchQt] This may take several minutes on first run.")

    execute_process(
        COMMAND "${PYTHON_EXECUTABLE}" -m aqt install-qt
                    "${FETCH_QT_HOST}"
                    "${FETCH_QT_TARGET}"
                    "${FETCH_QT_VERSION}"
                    "${FETCH_QT_ARCH}"
                    --outputdir "${FETCH_QT_BASE_DIR}"
        RESULT_VARIABLE _aqt_result
        OUTPUT_VARIABLE _aqt_output
        ERROR_VARIABLE  _aqt_error
    )
    if(NOT _aqt_result EQUAL 0)
        # Clean up any partial download so the next configure retries cleanly
        file(REMOVE_RECURSE "${FETCH_QT_VERSIONED_DIR}")
        message(FATAL_ERROR
            "[FetchQt] aqtinstall failed (exit code ${_aqt_result}).\n"
            "stdout: ${_aqt_output}\n"
            "stderr: ${_aqt_error}"
        )
    endif()

    # --- 4. Verify the result ----------------------------------------------
    if(NOT EXISTS "${FETCH_QT_PREFIX_DIR}/bin/qmake.exe")
        file(REMOVE_RECURSE "${FETCH_QT_VERSIONED_DIR}")
        message(FATAL_ERROR
            "[FetchQt] Download appeared to succeed but qmake.exe was not found "
            "at expected path:\n  ${FETCH_QT_PREFIX_DIR}/bin/qmake.exe\n"
            "Check FETCH_QT_ARCH — it must match the aqt arch name exactly."
        )
    endif()

    message(STATUS "[FetchQt] Qt ${FETCH_QT_VERSION} downloaded successfully.")
endif()

# -----------------------------------------------------------------------
# Expose the local Qt to CMake's package-search machinery
# -----------------------------------------------------------------------
list(PREPEND CMAKE_PREFIX_PATH "${FETCH_QT_PREFIX_DIR}")

# Also cache it so VS / cmake-gui can see the resolved path
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" CACHE STRING
    "Qt prefix path (auto-set by FetchQt.cmake)" FORCE)

message(STATUS "[FetchQt] CMAKE_PREFIX_PATH → ${FETCH_QT_PREFIX_DIR}")
