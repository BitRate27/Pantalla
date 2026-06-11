; Inno Setup Script for Pantalla
; Place this file in the repository root and compile with Inno Setup Compiler (ISCC.exe)

#define MyProjectName "Pantalla"
#define MyAppName "Pantalla"
#define MyAppVersion "0.1.0"
#define MyAppPublisher "BitRate27"
#define MyAppURL "https://github.com/BitRate27/Pantalla"
#define MyAppId "d66f2371-0ddd-4bb8-a888-c2f4cf827a30"

[Setup]
AppId={#MyAppId}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
; Default installation directory
DefaultDirName={pf64}\Pantalla
DefaultGroupName=BitRate27
; Output installer location (relative to script)
OutputDir=.
OutputBaseFilename=PantallaInstaller
Compression=lzma
SolidCompression=yes
; Require admin privileges to install device driver helper files / write Program Files
; Use PrivilegesRequired for compatibility with older Inno Setup versions
PrivilegesRequired=admin
; Use the provided ICO file as the installer icon (place P_red_circle_icon.ico next to this .iss)
SetupIconFile=P_red_circle_icon.ico
; Use the installed ICO for the Add/Remove Programs (Uninstall) entry
CloseApplications=yes
UninstallDisplayIcon={app}\P_red_circle_icon.ico

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
; Copy everything from the Release folder of the Pantalla project into the app folder
; Adjust the Source path if you place the script elsewhere. This assumes the 'installer' directory is at repo root
Source: "Release\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; Include the ICO file so it is installed into the application folder and can be referenced for shortcuts and the uninstaller
Source: "P_red_circle_icon.ico"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
; Start Menu shortcut - use the installed ICO file
Name: "{group}\Pantalla"; Filename: "{app}\Pantalla.exe"; WorkingDir: "{app}"; IconFilename: "{app}\P_red_circle_icon.ico"
; Desktop shortcut
Name: "{commondesktop}\Pantalla"; Filename: "{app}\Pantalla.exe"; Tasks: desktopicon; IconFilename: "{app}\P_red_circle_icon.ico"

[Tasks]
Name: desktopicon; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"; Flags: unchecked
; Optional installation of the NDI Runtime — selected by default (omit 'unchecked' so checkbox is checked)
Name: install_ndi_runtime; Description: "Install NDI Runtime (recommended)"; GroupDescription: "Additional components:";
Name: install_parsec_vdd; Description: "Install Parsec VDD (recommended)"; GroupDescription: "Additional components:";

[Run]
; If user selected the task, attempt to install the NDI Runtime and Parsec VDD. We accept agreements to avoid interactive prompts.
Filename: "cmd.exe"; Parameters: "/C winget install --exact --id NDI.NDIRuntime --accept-package-agreements --accept-source-agreements"; Description: "Install NDI Runtime"; StatusMsg: "Installing NDI Runtime..."; Flags: runhidden waituntilterminated postinstall skipifsilent; Check: IsTaskSelected('install_ndi_runtime')
Filename: "cmd.exe"; Parameters: "/C winget install --exact --id Parsec.ParsecVDD --accept-package-agreements --accept-source-agreements"; Description: "Install Parsec VDD"; StatusMsg: "Installing Parsec VDD..."; Flags: runhidden waituntilterminated postinstall skipifsilent; Check: IsTaskSelected('install_parsec_vdd')

; Run the tray app after installation
Filename: "{app}\Pantalla.exe"; Description: "Launch Pantalla"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
; Remove settings file on uninstall
Type: files; Name: "{commonappdata}\Pantalla\settings.json"


