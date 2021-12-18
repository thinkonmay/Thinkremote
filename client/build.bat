call "C:\Program Files\Microsoft Visual Studio\2022\Preview\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64
@REM call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat" x86_amd64
cd build
cmake .. 
msbuild ALL_BUILD.vcxproj



@REM call "D:\Qt\5.12.2\mingw73_64\bin\qtenv2.bat"
@REM cd D:/Thinkmay-client/remote-ui/build
@REM qmake ..
@REM D:\Qt\Tools\mingw730_64\bin\mingw32-make.exe
 