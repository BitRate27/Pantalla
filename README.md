# Pantalla
Output real and virtual displays over NDI

## User instructions
### Downloading
1. Go to the [release page](https://github.com/BitRate27/Pantalla/releases)
2. Expand the Assets list for the most recent release (first one)
3. Download PantallaInstaller.exe

### Installation
1. Run the PantallaInstaller.exe
2. May have to turn off Windows Defender Screen or any protection from exes from unknown publishers
3. Wait for a few moments after installation wizard finishes for the Parsec Driver and NDI Runtime to install

### Usage
1. Launch Pantalla from the Start menu or Desktop (white P in a red circle)
2. Left click the ^ "Show hidden icons" on the right side of task bar

<img width="165" height="95" alt="image" src="https://github.com/user-attachments/assets/bf839f05-51f7-4ffc-b6e9-a9e0d0250db2" />

3. Right click on the Pantalla icon (white P in red circle)

<img width="213" height="154" alt="image" src="https://github.com/user-attachments/assets/dc1a56b7-bf44-4560-b750-be81d0cdb401" />

4. Use this menu to add/remove displays and turn on/off NDI

<img width="308" height="210" alt="image" src="https://github.com/user-attachments/assets/a8562814-3b7d-4c76-b1ae-bbeeeba20a6a" />

5. Menu options
- Click on the NDI with a black background to turn NDI on for that display
- Click on the NDI with a green background to turn NDI off for that display
- Click on Add virtual display to add a new Parsec Virtual Display
- Click on the red x to remove the last virtual display added
- Click on Settings for saved settings and to see links and copyrights
- Click on Exit to terminate all NDI senders and remove any virtual displays

## Developer Notes
### Building
1. git clone https://github.com/BitRate27/Pantalla.git
2. cd Pantalla
3. cmake .
4. open Pantalla.sln

### Creating Installer
1. Change Pantalla configuration to Release
2. Build Pantalla
3. In a Command Shell: ISCC.exe Pantalla.iss


