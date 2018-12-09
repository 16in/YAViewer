@echo off
rem 環境に合わせてバッチファイルを指定しなおしてください。

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
	copy 修正履歴.txt YAViewerRelease
	copy todo.txt YAViewerRelease

	rem Avast対策用
	mkdir YAViewerRelease_avast
	copy bin\debug\YAViewer.exe YAViewerRelease_avast
	xcopy document\* YAViewerRelease_avast /Y /I /D /E
	copy 修正履歴.txt YAViewerRelease_avast
	copy todo.txt YAViewerRelease_avast

