@call ./setupvs.bat
@msbuild YAViewer.sln -property:Configuration=Release;Platform=x86 -t:rebuild
@msbuild YAViewer.sln -property:Configuration=Release;Platform=x64 -t:rebuild
@exit /b
