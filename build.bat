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
	for %%s in (*.sln) do (
		VCBUILD %%s
	)
