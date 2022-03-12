RMDIR /S /Q bin
dotnet build worker/port-forward --output "bin" --self-contained true --runtime win-x64
dotnet build signalling --output "bin" --self-contained false 
dotnet build client/webapp --output "bin" --self-contained true --runtime win-x64

call "D:\TraTanTinhThan\VS\VC\Auxiliary\Build\vcvarsall.bat" amd64_x86


RMDIR /S /Q build 
mkdir build

cd build && cmake .. && msbuild ALL_BUILD.vcxproj && cd ..

robocopy script/runtime bin/script /E
robocopy deployment/lib bin /E
robocopy deployment/virtualAudio bin/virtualAudio /E
robocopy deployment/virtualDisplay bin/virtualDisplay /E
robocopy client/webapp/wwwroot bin/wwwroot /E

cd client/electron/window && npm install && npm run make && robocopy out/ThinkMay-win32-x64 ../../../bin /E && cd ../../../