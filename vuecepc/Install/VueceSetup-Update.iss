[Setup]
AppID={code:GetAppID|''}
AppName=AgileTrack
AppVerName=AgileTrack Upgrade {code:GetAppMajorVersion|''}{code:GetAppMinorVersion|''} build {code:GetAppCurrentVersion|''}
AppPublisher=Adam Lane
AppPublisherURL=http://agiletrack.org
AppSupportURL=http://agiletrack.org
AppUpdatesURL=http://agiletrack.org
DefaultDirName={pf}\AgileTrack
DefaultGroupName=AgileTrack
AllowNoIcons=yes
OutputDir=dist
OutputBaseFilename=agiletrack-update-setup
Compression=lzma
SolidCompression=yes
InfoBeforeFile=doc\readme.rtf

[Languages]
Name: eng; MessagesFile: compiler:Default.isl

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked

[Files]
Source: dist\client\agiletrack-windows\agiletrack.exe; DestDir: {app}; Flags: ignoreversion
Source: dist\client\agiletrack-windows\*; DestDir: {app}; Flags: ignoreversion recursesubdirs createallsubdirs; Excludes: jre
; NOTE: Don't use "Flags: ignoreversion" on any shared system files
Source: major.dat; DestDir: {tmp}; Flags: dontcopy
Source: minor.dat; DestDir: {tmp}; Flags: dontcopy
Source: build.dat; DestDir: {tmp}; Flags: dontcopy

[INI]
Filename: {app}\agiletrack.url; Section: InternetShortcut; Key: URL; String: http://agiletrack.org

[Icons]
Name: {group}\AgileTrack; Filename: {app}\agiletrack.exe; WorkingDir: "{app}"
Name: {group}\{cm:ProgramOnTheWeb,agiletrack.org}; Filename: {app}\agiletrack.url
Name: {userdesktop}\AgileTrack; Filename: {app}\agiletrack.exe; Tasks: desktopicon; WorkingDir: "{app}"
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\AgileTrack; Filename: {app}\agiletrack.exe; Tasks: quicklaunchicon; WorkingDir: "{app}"
Name: {group}\{cm:UninstallProgram,AgileTrack}; Filename: "{uninstallexe}"

[Run]
Filename: {app}\agiletrack.exe; Description: {cm:LaunchProgram,AgileTrack}; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: files; Name: {app}\agiletrack.url

[Registry]
Root: HKLM; Subkey: Software\AgileTrack; ValueType: string; ValueName: Version; ValueData: {code:GetAppMajorVersion|''}; Flags: uninsdeletekey createvalueifdoesntexist
Root: HKLM; Subkey: Software\AgileTrack; ValueType: string; ValueName: CurrentVersion; ValueData: {code:GetAppCurrentVersion|''}; Flags: uninsdeletekey

[Code]
function GetAppMajorVersion(param: String): String;
	var
		AppVersion: String;
	begin
		ExtractTemporaryFile('major.dat');
		LoadStringFromFile(ExpandConstant('{tmp}\major.dat'), AppVersion);
		Result := AppVersion;
	end;

function GetAppMinorVersion(param: String): String;
	var
		AppMinorVersion: String;
	begin
		ExtractTemporaryFile('minor.dat');
		LoadStringFromFile(ExpandConstant('{tmp}\minor.dat'), AppMinorVersion);
		Result := AppMinorVersion;
	end;

function GetAppCurrentVersion(param: String): String;
	var
		BuildVersion: String;
	begin
		ExtractTemporaryFile('build.dat');
		LoadStringFromFile(ExpandConstant('{tmp}\build.dat'), BuildVersion);
		Result := BuildVersion;
	end;

function GetAppID(param: String): String;
	begin
		Result := 'AgileTrack';
	end;

function GetPathInstalled(AppID: String): String;
	var
		PrevPath: String;
	begin
		PrevPath := '';
		if not RegQueryStringValue(HKLM, 'Software\Microsoft\Windows\CurrentVersion\Uninstall\'+AppID+'_is1', 'Inno Setup: App Path', PrevPath) then begin
			RegQueryStringValue(HKCU, 'Software\Microsoft\Windows\CurrentVersion\Uninstall\'+AppID+'_is1', 'Inno Setup: App Path', PrevPath);
		end;
		Result := PrevPath;
	end;

function GetInstalledVersion(): String;
	var
		InstalledVersion: String;
	begin
		InstalledVersion := '';
		RegQueryStringValue(HKLM, 'Software\AgileTrack', 'Version', InstalledVersion);
		Result := InstalledVersion;
	end;

function GetInstalledCurrentVersion(): String;
	var
		InstalledCurrentVersion: String;
	begin
		InstalledCurrentVersion := '';
		RegQueryStringValue(HKLM, 'Software\AgileTrack', 'CurrentVersion', InstalledCurrentVersion);
		Result := InstalledCurrentVersion;
	end;

function InitializeSetup(): Boolean;
	var
		Response: Integer;
		PrevDir: String;
		InstalledVersion: String;
		InstalledCurrentVersion: String;
		VersionError: String;
	begin
		Result := true;

		// read the installtion folder
		PrevDir := GetPathInstalled(getAppID(''));

		if length(Prevdir) > 0 then begin
			// I found the folder so it's an upgrade
			InstalledVersion := GetInstalledVersion();
			// compare versions
			if InstalledVersion = GetAppMajorVersion('') then begin
				InstalledCurrentVersion := GetInstalledCurrentVersion();
				if (InstalledCurrentVersion < GetAppCurrentVersion('')) then begin
					Result := True;
				end else if (InstalledCurrentVersion = GetAppCurrentVersion('')) then begin
					Response := MsgBox(
						'It appears that the existing AgileTrack installation is already current.' + #13#13 +
						'Do you want to continue with the update installation?', mbError, MB_YESNO
					);
					Result := (Response = IDYES);
				end else begin
					Response := MsgBox(
						'It appears that the existing AgileTrack installation newer than this update.' + #13#13 +
						'The existing installation is build '+ InstalledCurrentVersion +'.  This update will change the installation to build '+ GetAppCurrentVersion('') + #13#13 +
						'Do you want to continue with the update installation?', mbError, MB_YESNO
					);
					Result := (Response = IDYES);
				end;
			end else begin
				if length(InstalledVersion) = 0 then begin
					VersionError := 'Setup was unable to determine the version of the existing AgileTrack installation.';
				end else begin
					VersionError := 'Setup has detected an installation of AgileTrack ' + InstalledVersion + '.';
				end;
				MsgBox(
					VersionError + #13#13 +
					'This update installer requires AgileTrack ' + GetAppMajorVersion('') +' to ' +
					'already be installed.' + #13 + 'Please install AgileTrack ' + GetAppMajorVersion('') +' before running this update.' + #13#13 +
					'Setup/Upgrade aborted.', mbError, MB_OK
				);
				Result := false;
			end;
		end else begin
			MsgBox(
				'This update installer requires an existing installation of AgileTrack ' + GetAppMajorVersion('') +' to ' +
				'already be installed.' + #13 + 'Please install AgileTrack ' + GetAppMajorVersion('') +' before running this update.' + #13#13 +
				'Setup/Upgrade aborted.', mbError, MB_OK
			);
			Result := false;
		end;
    end;

function ShouldSkipPage(PageID: Integer): Boolean;
	begin
		// skip selectdir if It's an upgrade
		if (PageID = wpSelectDir) then begin
			Result := true;
		end else if (PageID = wpSelectProgramGroup) then begin
			Result := true;
		end else if (PageID = wpSelectTasks) then begin
			Result := true;
		end else begin
			Result := false;
		end;
	end;