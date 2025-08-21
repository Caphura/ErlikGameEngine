# Windows setup — vcpkg + SDL2

1. Visual Studio 2022 (Desktop C++) kurulu olmalı.
2. Git ile vcpkg’i kur:
```powershell
git clone https://github.com/microsoft/vcpkg
cd vcpkg
bootstrap-vcpkg.bat
.\vcpkg integrate install
```
3. SDL2 kur:
```powershell
vcpkg install sdl2:x64-windows
```
4. Projeyi yapılandır ve derle:
```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
```