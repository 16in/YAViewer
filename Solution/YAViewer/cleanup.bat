@rem 作業用データ廃棄

@rem @echo off

rd /S /Q bin
del /A:-S /S /Q *.suo
del /A:-S /S /Q *.user
del /A:-S /S /Q *.ncb
del /A:-S /S /Q *.BuildOptions

for /d %%d in (*) do (
pushd %%d
	call :rdtemp
popd
)

@echo on
@exit /b


:rdtemp
rd /S /Q temp
for /d %%f in (*) do (
pushd %%f
	call :rdtemp
popd
)

exit /b
