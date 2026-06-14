# Pantalla
Output real and virtual displays over NDI

# Downloading
1. Go to the [release page](https://github.com/BitRate27/Pantalla/releases)
2. Download PanatallaInstaller.exe

# Installation
1. Run the PantallaInstaller.exe
2. May have to turn off Windows Defender Screen or any protection from exes from unknown publishers
3. Wait for a few moments after installation wizard finishes for the Parsec Driver and NDI Runtime to install

# Usage
1. Launch Pantalla from the Start menu or Desktop
2. Expand the System Tray with the ^ on the right side of 
<img width="308" height="210" alt="image" src="https://github.com/user-attachments/assets/a8562814-3b7d-4c76-b1ae-bbeeeba20a6a" />



# Overview
Pantalla is a small Windows tray application that manages virtual displays and publishes them via NDI. The tray UI and behavior are implemented in `src/TrayApp.cpp` and the display logic is handled by `DisplayManager`.
## Developer Notes

# Building
1. git clone https://github.com/BitRate27/Panalla.git
2. cd Pantalla
3. cmake .
4. open Pantalla.sln

# Creating Installer
1. Change Pantalla configuration to Release
2. Build Pantalla
3. In a Command Shell: ISCC.exe Pantalla.iss


