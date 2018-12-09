@echo off
call build.bat

echo.
rem 32bit”Å
mkdir ..\YAViewer.x86
copy bin\Win32\Release\YAViewer.exe ..\YAViewer.x86
xcopy ..\..\document\* ..\YAViewer.x86 /Y /I /D /E

rem 64bit”Å
mkdir ..\YAViewer.x64
copy bin\x64\Release\YAViewer.exe ..\YAViewer.x64
xcopy ..\..\document\* ..\YAViewer.x64 /Y /I /D /E

pause