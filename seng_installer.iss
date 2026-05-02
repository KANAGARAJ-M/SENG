; seng - Simple English Programming Language
; Inno Setup Script v1.0.0
; Author  : NoCorps.org build by KANAGARAJ-M
; Website : https://nocorps.org/seng

#define MyAppName      "seng"
#define MyAppVersion   "1.0.0"
#define MyAppPublisher "NoCorps.org build by KANAGARAJ-M"
#define MyAppURL       "https://nocorps.org/seng"
#define MyAppExe       "seng.exe"
#define MyAppDesc      "Simple English Programming Language"

[Setup]
AppId                  ={{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}
AppName                ={#MyAppName}
AppVersion             ={#MyAppVersion}
AppVerName             ={#MyAppName} {#MyAppVersion}
AppPublisher           ={#MyAppPublisher}
AppPublisherURL        ={#MyAppURL}
AppSupportURL          ={#MyAppURL}/support
AppUpdatesURL          ={#MyAppURL}/downloads
AppCopyright           =Copyright (C) 2026 {#MyAppPublisher}
VersionInfoVersion     =1.0.0.0
VersionInfoCompany     ={#MyAppPublisher}
VersionInfoDescription ={#MyAppDesc}
VersionInfoCopyright   =Copyright (C) 2026 {#MyAppPublisher}
DefaultDirName         ={autopf}\seng
DefaultGroupName       ={#MyAppName}
AllowNoIcons           =yes
DirExistsWarning       =no
LicenseFile            =LICENSE
OutputDir              =installer
OutputBaseFilename     =seng-setup-{#MyAppVersion}-windows-x64
SetupIconFile          =assets\seng_icon.ico
Compression            =lzma2/ultra64
SolidCompression       =yes
WizardStyle            =modern
ShowLanguageDialog     =no
DisableWelcomePage     =no
DisableDirPage         =no
DisableProgramGroupPage=yes
PrivilegesRequired     =admin
ChangesEnvironment     =yes
UninstallDisplayIcon   ={app}\{#MyAppExe}
UninstallDisplayName   ={#MyAppName} {#MyAppVersion}
CloseApplications      =yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "seng.exe";             DestDir: "{app}";          Flags: ignoreversion
Source: "src\*";                DestDir: "{app}\src";      Flags: ignoreversion recursesubdirs createallsubdirs
Source: "Makefile";             DestDir: "{app}";          Flags: ignoreversion
Source: "examples\*";          DestDir: "{app}\examples"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "README.md";            DestDir: "{app}";          Flags: ignoreversion
Source: "LICENSE";              DestDir: "{app}";          Flags: ignoreversion
Source: "CHANGELOG.md";         DestDir: "{app}";          Flags: ignoreversion
Source: "assets\seng_icon.ico"; DestDir: "{app}\assets";   Flags: ignoreversion
Source: "build.bat";            DestDir: "{app}";          Flags: ignoreversion

[Registry]
; Add seng to system PATH
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; ValueType: expandsz; ValueName: "Path"; ValueData: "{olddata};{app}"

; .se file association
Root: HKCU; Subkey: "Software\Classes\.se";                                ValueType: string; ValueName: ""; ValueData: "seng.SourceFile"
Root: HKCU; Subkey: "Software\Classes\seng.SourceFile";                    ValueType: string; ValueName: ""; ValueData: "seng Source File"
Root: HKCU; Subkey: "Software\Classes\seng.SourceFile\DefaultIcon";        ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExe},0"
Root: HKCU; Subkey: "Software\Classes\seng.SourceFile\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExe}"" ""%1"""

; .sec file association
Root: HKCU; Subkey: "Software\Classes\.sec";                                 ValueType: string; ValueName: ""; ValueData: "seng.CompiledFile"
Root: HKCU; Subkey: "Software\Classes\seng.CompiledFile";                    ValueType: string; ValueName: ""; ValueData: "seng Compiled Bytecode"
Root: HKCU; Subkey: "Software\Classes\seng.CompiledFile\DefaultIcon";        ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExe},0"
Root: HKCU; Subkey: "Software\Classes\seng.CompiledFile\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExe}"" run ""%1"""

; Install info
Root: HKLM; Subkey: "Software\NoCorps.org\seng"; ValueType: string; ValueName: "Version";     ValueData: "{#MyAppVersion}"
Root: HKLM; Subkey: "Software\NoCorps.org\seng"; ValueType: string; ValueName: "InstallPath"; ValueData: "{app}"

[Run]
Filename: "{app}\{#MyAppExe}"; Parameters: "help"; WorkingDir: "{app}"; StatusMsg: "Verifying seng installation..."; Flags: runhidden nowait

[UninstallRun]
Filename: "powershell.exe"; Parameters: "-ExecutionPolicy Bypass -Command ""$p=[Environment]::GetEnvironmentVariable('Path','Machine');$p=($p -split ';' | Where-Object {{$_ -ne '{app}'}}) -join ';';[Environment]::SetEnvironmentVariable('Path',$p,'Machine')"""; Flags: runhidden
