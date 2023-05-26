; Define your application name
!define APPNAME "OpenTTD"
!define APPNAMEANDVERSION "OpenTTD 0.3.2.1"
!define APPVERSION "0.3.2.1"
!define INSTALLERVERSION 11

BrandingText "OpenTTD Installer"


; Version Info
Var AddWinPrePopulate
VIProductVersion "0.3.2.1"
VIAddVersionKey "ProductName" "OpenTTD Installer"
VIAddVersionKey "Comments" "Installs ${APPNAMEANDVERSION}"
VIAddVersionKey "CompanyName" "OpenTTD Developers"
VIAddVersionKey "FileDescription" "Installs ${APPNAMEANDVERSION}"
VIAddVersionKey "ProductVersion" "${APPVERSION}"
VIAddVersionKey "InternalName" "InstOpenTTD"
VIAddVersionKey "FileVersion" "${APPVERSION}"
VIAddVersionKey "LegalCopyright" " "
; Main Install settings
Name "${APPNAMEANDVERSION}"

; NOTE: Keep trailing backslash!
InstallDir "$PROGRAMFILES\OpenTTD\"
InstallDirRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenTTD" "Install Folder"
OutFile "openttd-${APPVERSION}.exe"

ShowInstDetails show
ShowUninstDetails show
SetCompressor LZMA

Var SHORTCUTS
Var CDDRIVE

; Modern interface settings
!include "MUI.nsh"

!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_WELCOME
LicenseForceSelection radiobuttons "I &accept this Agreement" "I &do not accept this agreement"

!insertmacro MUI_PAGE_LICENSE "gpl.txt"

;--------------------------------
; Rest of pages
!insertmacro MUI_PAGE_COMPONENTS


;--------------------------------
; New custom page for finding CD
Page custom SelectCD SelectCD2 ": TTD folder"

!insertmacro MUI_PAGE_DIRECTORY

;Start Menu Folder Page Configuration
!define MUI_STARTMENUPAGE_DEFAULTFOLDER $SHORTCUTS
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKEY_LOCAL_MACHINE"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenTTD"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Shortcut Folder"

!insertmacro MUI_PAGE_STARTMENU "OpenTTD" $SHORTCUTS

!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_RUN "$INSTDIR\openttd.exe"
!define MUI_FINISHPAGE_LINK "Visit OpenTTD's homepage"
!define MUI_FINISHPAGE_LINK_LOCATION "http://www.openttd.com/"
!define MUI_FINISHPAGE_NOREBOOTSUPPORT
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\readme.txt"

!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Set languages (first is default language)
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_RESERVEFILE_LANGDLL

;--------------------------------
; Sections
Section "!OpenTTD" Section1

	; Set Section properties
	SetOverwrite try

	; Make savegame folder
	SetOutPath "$INSTDIR\save"
	; define root variable relative to installer
	!define PATH_ROOT "..\..\..\"

	; Copy language files
  SetOutPath "$INSTDIR\lang\"
  File ${PATH_ROOT}lang\*.lng
  File ${PATH_ROOT}lang\english.txt

	; Copy data files
	SetOutPath "$INSTDIR\data\"
  File ${PATH_ROOT}data\opntitle.dat
  File ${PATH_ROOT}data\signalsw.grf
  File ${PATH_ROOT}data\openttd.grf
  File ${PATH_ROOT}data\canalsw.grf
  File ${PATH_ROOT}ReleasePNG\ttd.map

	; Copy the rest of the stuff
	SetOutPath "$INSTDIR\"

	;Copy text files
  File ${PATH_ROOT}changelog.txt
;  File "gpl.txt"
  File ${PATH_ROOT}readme.txt

	; Copy executable
	File /oname=openttd.exe        ${PATH_ROOT}ReleasePNG\ttd.exe
	File ${PATH_ROOT}strgen\Release\strgen.exe


  ; Delete old files from the main dir. they are now placed in data/ and lang/
	Delete "$INSTDIR\*.lng"
	Delete "$INSTDIR\*.grf"
	Delete "$INSTDIR\sample.cat"
	Delete "$INSTDIR\ttd.exe"


	;Creates the Registry Entries
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenTTD" "Comments" "Visit http://www.openttd.com"
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenTTD" "DisplayIcon" "$INSTDIR\setup.ico"
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenTTD" "DisplayName" "OpenTTD ${APPVERSION}"
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenTTD" "DisplayVersion" "${APPVERSION}"
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenTTD" "HelpLink" "http://www.openttd.com"
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenTTD" "Install Folder" "$INSTDIR"
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenTTD" "Publisher" "OpenTTD"
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenTTD" "Shortcut Folder" "$SHORTCUTS"
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenTTD" "UninstallString" "$INSTDIR\uninstall.exe"
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenTTD" "URLInfoAbout" "http://www.openttd.com"
	;This key sets the Version DWORD that patches will check against
	WriteRegDWORD HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenTTD" "Version" ${INSTALLERVERSION}

	!insertmacro MUI_STARTMENU_WRITE_BEGIN "OpenTTD"
	CreateShortCut "$DESKTOP\OpenTTD.lnk" "$INSTDIR\openttd.exe"
	CreateDirectory "$SMPROGRAMS\$SHORTCUTS"
	CreateShortCut "$SMPROGRAMS\$SHORTCUTS\OpenTTD.lnk" "$INSTDIR\openttd.exe"
	CreateShortCut "$SMPROGRAMS\$SHORTCUTS\Uninstall.lnk" "$INSTDIR\uninstall.exe"
	CreateShortCut "$SMPROGRAMS\$SHORTCUTS\Readme.lnk" "$INSTDIR\Readme.txt"
	CreateShortCut "$SMPROGRAMS\$SHORTCUTS\Changelog.lnk" "$INSTDIR\Changelog.txt"
	!insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section "Copy Game Graphics" Section2
	; Include files from CD
	;Let's copy the files from the CD
	SetOutPath "$INSTDIR\gm"
	CopyFiles "$CDDRIVE\gm\*.gm" "$INSTDIR\gm\"
	SetOutPath "$INSTDIR\data\"
	CopyFiles "$CDDRIVE\sample.cat" "$INSTDIR\data\sample.cat" 1566
	CopyFiles "$CDDRIVE\trg1r.grf" "$INSTDIR\data\trg1r.grf" 2365
	CopyFiles "$CDDRIVE\trgcr.grf" "$INSTDIR\data\trgcr.grf" 260
	CopyFiles "$CDDRIVE\trghr.grf" "$INSTDIR\data\trghr.grf" 400
	CopyFiles "$CDDRIVE\trgir.grf" "$INSTDIR\data\trgir.grf" 334
	CopyFiles "$CDDRIVE\trgtr.grf" "$INSTDIR\data\trgtr.grf" 546
	SetOutPath "$INSTDIR\"
SectionEnd

;----------------------
Section -FinishSection
	WriteUninstaller "$INSTDIR\uninstall.exe"
SectionEnd

; Modern install component descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${Section1} "OpenTTD is a fully functional clone of TTD and is very playable."
	!insertmacro MUI_DESCRIPTION_TEXT ${Section2} "Copies the game graphics. Requires TTD for Windows."
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;!undef SHORTCUTS
;Uninstall section

Section Uninstall
	MessageBox MB_YESNOCANCEL|MB_ICONQUESTION "Would you like to remove the Saved Game Folder located at '$INSTDIR\Save?'  If you choose Yes, your Saved Games will be removed." IDYES RemoveSavedGames IDNO NoRemoveSavedGames
	RemoveSavedGames:
		Delete "$INSTDIR\Save\*.*"
		Delete "$INSTDIR\AutoSave\*.*"
		RMDir "$INSTDIR\Save"
		RMDir "$INSTDIR\AutoSave"
	NoRemoveSavedGames:

	;Remove from registry...
	!insertmacro MUI_STARTMENU_GETFOLDER "OpenTTD" $SHORTCUTS
	ReadRegStr $SHORTCUTS HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenTTD" "Shortcut Folder"

	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenTTD"

	; Delete self
	Delete "$INSTDIR\uninstall.exe"

	; Delete Shortcuts
	Delete "$DESKTOP\OpenTTD.lnk"
	Delete "$SMPROGRAMS\$SHORTCUTS\OpenTTD.lnk"
	Delete "$SMPROGRAMS\$SHORTCUTS\Uninstall.lnk"
	Delete "$SMPROGRAMS\$SHORTCUTS\Readme.lnk"
	Delete "$SMPROGRAMS\$SHORTCUTS\Changelog.lnk"

	; Clean up OpenTTD dir
	Delete "$INSTDIR\changelog.txt"
	Delete "$INSTDIR\readme.txt"
	Delete "$INSTDIR\openttd.exe"
	Delete "$INSTDIR\strgen.exe"
	Delete "$INSTDIR\data\ttd.map"
	Delete "$INSTDIR\gpl.txt"
	Delete "$INSTDIR\INSTALL.LOG"
	Delete "$INSTDIR\crash.log"
	Delete "$INSTDIR\openttd.cfg"

	; Data files
	Delete "$INSTDIR\data\opntitle.dat"
	Delete "$INSTDIR\data\signalsw.grf"
	Delete "$INSTDIR\data\openttd.grf"
	Delete "$INSTDIR\data\canalsw.grf"
	Delete "$INSTDIR\data\trg1r.grf"
	Delete "$INSTDIR\data\trghr.grf"
	Delete "$INSTDIR\data\trgtr.grf"
	Delete "$INSTDIR\data\sample.cat"
	Delete "$INSTDIR\data\trgcr.grf"
	Delete "$INSTDIR\data\trgir.grf"

	;Music
	Delete "$INSTDIR\gm\*.gm"

	;Language files
	Delete "$INSTDIR\lang\*.lng"
	Delete "$INSTDIR\lang\english.txt"

	; Remove remaining directories
	RMDir "$SMPROGRAMS\$SHORTCUTS\Extras\"
	RMDir "$SMPROGRAMS\$SHORTCUTS"
	RMDir "$INSTDIR\gm"
	RMDir "$INSTDIR\lang"
	RMDir "$INSTDIR\data"
	RMDir "$INSTDIR"
SectionEnd

Function SelectCD
	SectionGetFlags ${Section2} $0
	IntOp $0 $0 & 1
	IntCmp $0 1 NoAbort
		Abort
NoAbort:

	GetTempFileName $R0
	!insertmacro MUI_HEADER_TEXT "Locate TTD" "Setup needs the location of Transport Tycoon Deluxe in order to continue."
	File /oname=$R0 CDFinder.ini
	ClearErrors
	;Now, let's populate $CDDRIVE
	ReadRegStr $CDDRIVE HKLM "SOFTWARE\Fish Technology Group\Transport Tycoon Deluxe" "HDPath"
	IfErrors NoTTD
	StrCpy $AddWinPrePopulate "Setup has detected your TTD folder.  Don't change the folder.  Simply press Next."
	Goto TruFinish
NoTTD:
	StrCpy $CDDRIVE ""
	StrCpy $AddWinPrePopulate "Setup couldn't find TTD.  Please enter the path where the graphics files from TTD for Windows are stored and press Next to continue."
TruFinish:
	ClearErrors
	WriteINIStr $R0 "Field 2" "State" $CDDRIVE
	WriteINIStr $R0 "Field 3" "Text" $AddWinPrePopulate

	InstallOptions::dialog $R0
	Pop $R1
FunctionEnd

; Exit from page function
Function SelectCD2
	ReadINIStr $CDDRIVE $R0 "Field 2" "State"
	IfFileExists $CDDRIVE\trg1r.grf "" NoCD
	IfFileExists $CDDRIVE\sample.cat "" NoCD
	IfFileExists $CDDRIVE\trgir.grf hasCD ""
NoCD:
  MessageBox MB_OK "Setup cannot continue without the Transport Tycoon Deluxe Location!"
  Abort
hasCD:
FunctionEnd


Var OLDVERSION
Var UninstallString

Function .onInit
	StrCpy $SHORTCUTS "OpenTTD"

	SectionSetFlags 0 17

	;Want to have a splash BMP?  Uncomment these lines - CAREFUL WITH FILE SIZE

;	        # the plugins dir is automatically deleted when the installer exits
;        InitPluginsDir
;				File /oname=$PLUGINSDIR\splash.bmp "C:\Documents and Settings\Administrator\My Documents\My Pictures\OpenTTD Splash.bmp"
;        #optional
;        #File /oname=$PLUGINSDIR\splash.wav "C:\myprog\sound.wav"
;
;        ;MessageBox MB_OK "Fading"
;
;       advsplash::show 3000 600 400 -1 $PLUGINSDIR\splash
;
;      Pop $0          ; $0 has '1' if the user closed the splash screen early,
;                        ; '0' if everything closed normal, and '-1' if some error occured.
;End Splash Area
		;Starts Setup - let's look for an older version of OpenTTD
	ReadRegDWORD $R8 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenTTD" "Version"

	IfErrors ShowWelcomeMessage ShowUpgradeMessage
ShowWelcomeMessage:
	ReadRegStr $R8 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenTTD" "Version"
	;In the event someone still has OpenTTD 0.1, this will detect that (that installer used a string instead of dword entry)
	IfErrors FinishCallback

ShowUpgradeMessage:
	IntCmp ${INSTALLERVERSION} $R8 VersionsAreEqual InstallerIsOlder  WelcomeToSetup
WelcomeToSetup:
	;An older version was found.  Let's let the user know there's an upgrade that will take plce.
	ReadRegStr $OLDVERSION HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenTTD" "DisplayVersion"
	;Gets the older version then displays it in a message box
	MessageBox MB_OK|MB_ICONINFORMATION "Welcome to ${APPNAMEANDVERSION} Setup.$\n$\nThis will allow you to upgrade from version $OLDVERSION."
	Goto FinishCallback

VersionsAreEqual:
	ReadRegStr $UninstallString HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenTTD" "UninstallString"
	IfFileExists "$UninstallString" "" FinishCallback
	MessageBox MB_YESNO|MB_ICONQUESTION "Setup detected ${APPNAMEANDVERSION} on your system. That's the version this program will install.$\n$\nAre you trying to uninstall it?" IDYES DoUninstall IDNO FinishCallback
DoUninstall: ;You have the same version as this installer.  This allows you to uninstall.
	Exec "$UninstallString"
	Quit

InstallerIsOlder:
	MessageBox MB_OK|MB_ICONSTOP "You have a newer version of ${APPNAME}.$\n$\nSetup will now exit."
	Quit

FinishCallback:
	ClearErrors
FunctionEnd
; eof

