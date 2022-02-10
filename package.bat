RMDIR /S /Q bin
dotnet build worker/port-forward --output "bin" --self-contained true --runtime win-x64

call "C:\Program Files\Microsoft Visual Studio\2022\Preview\VC\Auxiliary\Build\vcvarsall.bat" amd64_x86
cd build && cmake .. && msbuild ALL_BUILD.vcxproj && cd ..

robocopy deployment/lib bin /E

powershell Invoke-WebRequest -Uri https://gstreamer.freedesktop.org/data/pkg/windows/1.20.0/msvc/gstreamer-1.0-msvc-x86_64-1.20.0-merge-modules.zip -OutFile deployment/gstreamer.zip && cd deployment && powershell Expand-Archive gstreamer.zip -DestinationPath modules && cd ..

cd client/window/electron && npm run make && robocopy  out/ThinkMay-win32-x64 ../../../bin /E && cd ../../../
