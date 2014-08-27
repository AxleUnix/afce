RequestExecutionLevel admin
SetCompressor /SOLID lzma
Name "�������� ����-���� ����������"
!define VERSION 0.9.7
!define QTDIR C:\Qt\5.2.1\mingw48_32\bin
!define BUILDDIR ..\build-afce-new-Desktop_Qt_5_2_1_MinGW_32bit-Release
OutFile "..\afce-${VERSION}-win32.exe"
InstallDir "$PROGRAMFILES\afce"

AutoCloseWindow true
ShowInstDetails show

XPStyle on

LoadLanguageFile "${NSISDIR}\Contrib\Language files\Russian.nlf"

Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section "Program"
  SetOutPath "$INSTDIR"
  File "${BUILDDIR}\release\afce.exe"
  File "README.RU.txt"
  File "afc.ico"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\afce" "DisplayName" "�������� ����-����"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\afce" "DisplayIcon" "$INSTDIR\afce.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\afce" "DisplayVersion" "${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\afce" "Publisher" "������ ��������"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\afce" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\afce" "URLInfoAbout" "http://viktor-zin.blogspot.ru/2011/09/blog-post_5556.html"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\afce" "InstallLocation" "$INSTDIR"

  WriteRegStr HKCR ".afc" "" "afcfile"
  WriteRegStr HKCR ".afc" "Content Type" "application/x-afce"
  WriteRegStr HKCR "afcfile" "" "����-�����"
  WriteRegStr HKCR "afcfile\DefaultIcon" "" '$INSTDIR\afc.ico'
  WriteRegStr HKCR "afcfile\shell\open\command" "" '"$INSTDIR\afce.exe" "%1"'

  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v (0x08000000, 0, 0, 0)'

  WriteUninstaller "$INSTDIR\uninst.exe"
SectionEnd

Section "Translations"
  SetOutPath "$INSTDIR\locale"
  File "locale\*.qm"
  File "${BUILDDIR}\locale\*.qm"
SectionEnd

Section "Qt"
  SetOutPath "$INSTDIR"
  File "${QTDIR}\libstdc++-6.dll"
  File "${QTDIR}\libgcc_s_dw2-1.dll"
  File "${QTDIR}\Qt5Core.dll"
  File "${QTDIR}\Qt5Gui.dll"
  File "${QTDIR}\Qt5PrintSupport.dll"
  File "${QTDIR}\Qt5Svg.dll"
  File "${QTDIR}\Qt5Xml.dll"
  File "${QTDIR}\Qt5Widgets.dll"
  File "${QTDIR}\libwinpthread-1.dll"
  File "${QTDIR}\icuin51.dll"
  File "${QTDIR}\icuuc51.dll"
  File "${QTDIR}\icudt51.dll"

SectionEnd

Section "Help"
  SetOutPath "$INSTDIR\help"
  File /r "help\*.*"
SectionEnd

Section "Generators"
  SetOutPath "$INSTDIR\generators"
  File /r "generators\*.json"
SectionEnd


Section "Shortcuts"
  SetOutPath "$INSTDIR"
  CreateDirectory "$SMPROGRAMS\�������� ����-����"
  CreateShortCut "$SMPROGRAMS\�������� ����-����\�������� ����-����.lnk" "$INSTDIR\afce.exe"
  CreateShortCut "$SMPROGRAMS\�������� ����-����\������� ���������.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section "Uninstall"
  RMDIR /r "$INSTDIR"
  RMDIR /r "$SMPROGRAMS\�������� ����-����"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\afce"

  DeleteRegKey HKCR ".afc"
  DeleteRegKey HKCR "afcfile"
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v (0x08000000, 0, 0, 0)'
SectionEnd
