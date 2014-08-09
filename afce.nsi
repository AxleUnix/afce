RequestExecutionLevel admin
SetCompressor /SOLID lzma
Name "�������� ����-���� ����������"
!define VERSION 0.9.6
!define QTDIR C:\Qt\5.2.1\mingw48_32\bin
!define BUILDDIR ..\build-afce-Desktop_Qt_5_2_1_MinGW_32bit-Debug\release
OutFile "..\afce-${VERSION}-win32.exe"
InstallDir "$PROGRAMFILES\afce"
;BGGradient 0000FF 000000 FFFFFF
;MiscButtonText "< �����" "����� >" "������" "�������"
;InstallButtonText "����������"
;DirText "������� �����, ���� ����� ���������� ������." "������� ����������:" "�����..."
AutoCloseWindow true
ShowInstDetails show
;CompletedText "������"
XPStyle on

LoadLanguageFile "${NSISDIR}\Contrib\Language files\Russian.nlf"

Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section "Program"
  SetOutPath "$INSTDIR"
  File "${BUILDDIR}\afce.exe"
  File "afce_ru_RU.qm"
  File "afce_en_US.qm"
  File "README.RU.txt"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\afce" "DisplayName" "�������� ����-����"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\afce" "DisplayIcon" "$INSTDIR\afce.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\afce" "DisplayVersion" "${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\afce" "Publisher" "������ ��������"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\afce" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\afce" "URLInfoAbout" "http://viktor-zin.blogspot.ru/2011/09/blog-post_5556.html"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\afce" "InstallLocation" "$INSTDIR"

  WriteUninstaller "$INSTDIR\uninst.exe"
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

SectionEnd
