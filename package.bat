RMDIR /S /Q bin
dotnet build worker/port-forward --output "bin" --self-contained false --runtime win-x64

call "C:\Program Files\Microsoft Visual Studio\2022\Preview\VC\Auxiliary\Build\vcvarsall.bat" amd64_x86
cd build && cmake .. && msbuild ALL_BUILD.vcxproj && cd ../client/window/electron

npm run make && robocopy  out/ThinkMay-win32-x64 ../../../bin /E && cd ../../../