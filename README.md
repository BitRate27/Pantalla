# Pantalla
Output real and virtual displays over NDI

# Building
1. git clone https://github.com/BitRate27/Panalla.git
2. cd Pantalla
3. cmake .
4. open Pantalla.sln

# Creating Installer
1. Change Pantalla configuration to Release
2. Build Pantalla
3. In a Command Shell: ISCC.exe Pantalla.iss

# Pantalla
========

Overview
--------
Pantalla is a small Windows tray application that manages virtual displays and publishes them via NDI. The tray UI and behavior are implemented in `src/TrayApp.cpp` and the display logic is handled by `DisplayManager`.

Quick start — launch the app
---------------------------
- Build the project using your usual tool (Visual Studio / MSBuild / CMake) for Release.
- Run the produced executable. The application runs in the background and appears as a tray icon.

Where to find the app in the tray
--------------------------------
- After launching, look for the red circular tray icon with a white `P`.
- Hover the icon to see the tooltip `Pantalla`.
- If the icon is not visible, open the Windows notification area (system tray overflow) and locate `Pantalla`.

Showing the menu
-----------------
- Right-click the tray icon (context activation) to open the menu. Pantalla rebuilds the menu on each context activation so the contents reflect the current displays and NDI senders.
- The application attempts to position the popup above and left-justified to the tray icon; if the icon geometry cannot be determined it falls back to the cursor position.

Menu layout and operations
--------------------------
- Each display entry in the menu shows:
 - An `NDI` button at the left (29×18 px in the UI).
 - The display/screen name as a label.
 - For the last Parsec virtual display entry, a right-aligned `X` button to remove the virtual display.

- Turning NDI output on or off
 - If the `NDI` button is green the display currently has an active NDI sender.
 - If the `NDI` button is black no NDI sender exists for that display.
 - Click the `NDI` button to toggle the sender: clicking a black button adds an NDI sender, clicking a green button removes it.
 - The UI updates immediately after the operation (menu is rebuilt).

- Adding a virtual display
 - Choose `Add virtual display` from the menu to create a new virtual display.
 - This calls `DisplayManager::addVirtualDisplay()` (see `TrayApp::onAddVirtualDisplay()`).

- Removing a virtual display
 - If present, Parsec virtual displays show an `X` button on the last Parsec entry. Click the `X` to remove that virtual display.
 - Alternatively, virtual displays can be removed through any UI or code paths implemented in `DisplayManager`.

- Settings
 - Open `Settings` from the menu to toggle options such as `Start on Windows Startup`.
 - Changes take effect when you accept the dialog.

- Exit
 - Use `Exit` in the menu to quit the application and the event loop.

Developer notes
---------------
- The tray/menu code lives in `src/TrayApp.cpp` (notable functions: `buildIcon()`, `buildMenu()`, `updateMenuContents()`, and `onTrayIconActivated()`).
- Display management is performed by `DisplayManager` and the UI calls into it for actions like adding/removing virtual displays and NDI senders.

Troubleshooting
---------------
- If the menu does not appear, try opening the tray overflow or left-clicking the tray icon once to bring it into view.
- If NDI operations do not behave as expected, check the logs and the implementation in `DisplayManager`.

License / Attribution
---------------------
See the repository for license information and third-party attributions.


