SetCompressor /SOLID lzma
Name "�������� ����-���� ����������"
OutFile "afce-0.9.6-50.i586.exe"
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
  File "release\afce.exe"
  File "release\afce_ru_RU.qm"
  File "release\afce_en_US.qm"
  File "release\README.RU.txt"
  File "release\mingwm10.dll"

  WriteUninstaller "$INSTDIR\uninst.exe"
SectionEnd

Section "Qt-4.4.3"
  SetOutPath "$INSTDIR"
  File "release\Qt*.dll"

SectionEnd

Section "Doc"
  SetOutPath "$INSTDIR\doc"
  File "release\doc\*.*"
  
  WriteUninstaller "$INSTDIR\uninst.exe"
SectionEnd


Section "Shortcuts"
  SetOutPath "$INSTDIR"
  CreateDirectory "$SMPROGRAMS\�������� ����-����"
  CreateShortCut "$SMPROGRAMS\�������� ����-����\�������� ����-����.lnk" "$INSTDIR\afce.exe"
  CreateShortCut "$SMPROGRAMS\�������� ����-����\������� ���������.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section "Uninstall"
  Delete "$INSTDIR\doc\*.*"
  RMDIR "$INSTDIR\doc"

  Delete "$INSTDIR\*.*"
  RMDIR "$INSTDIR"
  
  Delete "$SMPROGRAMS\�������� ����-����\�������� ����-����.lnk"
  Delete "$SMPROGRAMS\�������� ����-����\������� ���������.lnk"
  RMDIR "$SMPROGRAMS\�������� ����-����"

SectionEnd
