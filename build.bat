rmdir bin
dotnet build worker/port-forward --output "bin"
call "C:\Program Files\Microsoft Visual Studio\2022\Preview\VC\Auxiliary\Build\vcvarsall.bat" amd64_x86
cd build
cmake ..
msbuild ALL_BUILD.vcxproj