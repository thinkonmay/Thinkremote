RMDIR /S /Q bin
dotnet build worker/port-forward --output "bin" --self-contained true --runtime win-x64

call "C:\Program Files\Microsoft Visual Studio\2022\Preview\VC\Auxiliary\Build\vcvarsall.bat" amd64_x86


RMDIR /S /Q build && mkdir build

cd build && cmake .. && msbuild ALL_BUILD.vcxproj && cd ..

robocopy deployment/lib bin /E
robocopy deployment/virtualAudio bin/virtualAudio /E
robocopy deployment/virtualDisplay bin/virtualDisplay /E

cd client/window/electron && npm run make && robocopy  out/ThinkMay-win32-x64 ../../../bin /E && cd ../../../