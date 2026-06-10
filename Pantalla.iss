; Inno Setup Script for Pantalla
; Place this file in the repository root and compile with Inno Setup Compiler (ISCC.exe)

#define MyProjectName "Pantalla"
#define MyAppName "Pantalla"
#define MyAppVersion "0.1.0"
#define MyAppPublisher "BitRate27"
#define MyAppURL "https://github.com/BitRate27/Pantalla"
#define MyAppId "d66f2371-0ddd-4bb8-a888-c2f4cf827a30"

[Setup]
AppId={{{#MyAppId}}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
; Default installation directory
DefaultDirName=\Program Files\Pantalla
DefaultGroupName=BitRate27
; Output installer location (relative to script)
OutputDir=.
OutputBaseFilename=PantallaInstaller
Compression=lzma
SolidCompression=yes
; Require admin privileges to install device driver helper files / write Program Files
; Use PrivilegesRequired for compatibility with older Inno Setup versions
PrivilegesRequired=admin

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
; Copy everything from the Release folder of the Pantalla project into the app folder
; Adjust the Source path if you place the script elsewhere. This assumes the 'installer' directory is at repo root
Source: "Release\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
; Start Menu shortcut
Name: "{group}\Pantalla"; Filename: "{app}\Pantalla.exe"; WorkingDir: "{app}"; IconFilename: "{app}\Pantalla.exe"
; Desktop shortcut
Name: "{commondesktop}\Pantalla"; Filename: "{app}\Pantalla.exe"; Tasks: desktopicon; IconFilename: "{app}\Pantalla.exe"

[Tasks]
Name: desktopicon; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"; Flags: unchecked

[Run]
; Run the tray app after installation
Filename: "{app}\Pantalla.exe"; Description: "Launch Pantalla"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
; Remove settings file on uninstall
Type: files; Name: "{commonappdata}\Pantalla\settings.json"
