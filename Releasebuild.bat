@echo off
rem ���ɍ��킹�ăo�b�`�t�@�C�����w�肵�Ȃ����Ă��������B

SETLOCAL 

VER | find "XP" > nul 
	IF not errorlevel 1 GOTO winXP

goto win7


:winXP
	call "C:\Program Files\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
	goto build

:win7
	call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
	goto build

:build
	VCBUILD YAViewer.sln "FinalRelease|Win32"
	VCBUILD YAViewer.sln "Debug|Win32"
	
	echo.
	xcopy document\* YAViewerRelease /Y /I /D /E
	copy �C������.txt YAViewerRelease
	copy todo.txt YAViewerRelease

	rem Avast�΍��p
	mkdir YAViewerRelease_avast
	copy bin\debug\YAViewer.exe YAViewerRelease_avast
	xcopy document\* YAViewerRelease_avast /Y /I /D /E
	copy �C������.txt YAViewerRelease_avast
	copy todo.txt YAViewerRelease_avast

