; SPDX-License-Identifier: Zlib
; Synthetic extraction fixture. Tests inspect this package without running it.
#ifndef CaseId
  #error CaseId must be supplied by the fixture builder
#endif
#ifndef CompilerSeries
  #error CompilerSeries must be supplied by the fixture builder
#endif
#ifndef Span
  #define Span 0
#endif
#ifndef SetupArch
  #define SetupArch "x86"
#endif

[Setup]
AppId=InnoExtract.FormatFixture.{#CaseId}
AppName=Format Regression Sample
AppVersion=1.0
AppPublisher=Example
DefaultDirName={localappdata}\InnoExtractFormatSample\{#CaseId}
DefaultGroupName=Format Regression Sample
PrivilegesRequired=lowest
Uninstallable=yes
UsePreviousAppDir=no
UsePreviousGroup=no
UsePreviousSetupType=no
UsePreviousTasks=no
UsePreviousUserInfo=no
DisableProgramGroupPage=yes
DisableStartupPrompt=yes
WizardStyle=modern
CloseApplications=no
OutputBaseFilename=setup
SourceDir=.
#if Span
DiskSpanning=yes
DiskSliceSize=4194304
SlicesPerDisk=1
Compression=none
SolidCompression=no
#else
Compression=lzma2
SolidCompression=yes
#endif
#if CompilerSeries >= 60402
CloseApplicationsFilterExcludes=*.skip
#endif
#if CompilerSeries >= 60502
WizardImageBackColor=clWhite
WizardSmallImageBackColor=clBlack
#endif
#if CompilerSeries >= 60601
WizardImageOpacity=231
#endif
#if CompilerSeries >= 70000
SetupArchitecture={#SetupArch}
#endif

[Types]
Name: "full"; Description: "Complete"

[Components]
Name: "core"; Description: "Core"; Types: full; Flags: fixed
Name: "core\child"; Description: "Nested component"; Types: full

[Tasks]
Name: "sampletask"; Description: "Sample task"; Flags: unchecked
Name: "sampletask\child"; Description: "Nested task"; Flags: unchecked

[Dirs]
Name: "{app}\empty"; Components: core

[Files]
Source: "payload\hello.txt"; DestDir: "{app}"; Flags: ignoreversion; Components: core
Source: "payload\data.bin"; DestDir: "{app}\nested"; Flags: ignoreversion; Components: core\child
Source: "payload\unicode.txt"; DestDir: "{app}\unicodé"; DestName: "naïve.txt"; Flags: ignoreversion
Source: "payload\hello.txt"; DestDir: "{app}"; DestName: "architecture32.txt"; Flags: ignoreversion 32bit
Source: "payload\hello.txt"; DestDir: "{app}"; DestName: "architecture64.txt"; Flags: ignoreversion 64bit; Check: IsWin64

[Icons]
Name: "{group}\Sample document"; Filename: "{app}\hello.txt"

[INI]
Filename: "{app}\sample.ini"; Section: "sample"; Key: "format"; String: "regression"; Flags: uninsdeleteentry uninsdeletesectionifempty

[Registry]
Root: HKCU32; Subkey: "Software\InnoExtract\FormatFixture\{#CaseId}"; ValueType: string; ValueName: "Sample32"; ValueData: "sample"; Flags: uninsdeletekey
Root: HKCU64; Subkey: "Software\InnoExtract\FormatFixture\{#CaseId}"; ValueType: string; ValueName: "Sample64"; ValueData: "sample"; Flags: uninsdeletekey; Check: IsWin64

[InstallDelete]
Type: files; Name: "{app}\old.tmp"

[UninstallDelete]
Type: filesandordirs; Name: "{app}\generated"

[Run]
#if CompilerSeries >= 70000
Filename: "{cmd}"; Parameters: "/C exit 0"; Flags: runhidden 32bit logoutput; Check: NeverRun; OnLog: LogLine
#else
Filename: "{cmd}"; Parameters: "/C exit 0"; Flags: runhidden 32bit; Check: NeverRun
#endif

[UninstallRun]
Filename: "{cmd}"; Parameters: "/C exit 0"; Flags: runhidden 64bit; Check: NeverRun

[Code]
function NeverRun: Boolean;
begin
  Result := False;
end;

#if CompilerSeries >= 70000
procedure LogLine(const S: String; const Error, FirstLine: Boolean);
begin
end;
#endif
