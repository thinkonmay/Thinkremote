rmdir bin
dotnet build worker/port-forward --output "bin" 
dotnet build signalling --output "bin"
dotnet build client/webapp --output "bin"
robocopy client/webapp/wwwroot bin/wwwroot /E

call "C:\Program Files\Microsoft Visual Studio\2022\Preview\VC\Auxiliary\Build\vcvarsall.bat" amd64_x86
cd build
cmake ..
msbuild ALL_BUILD.vcxproj