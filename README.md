# ErlikGameEngine — Step 3: Renderer2D + Texture (SDL2_image)

Bağımlılık: `vcpkg install sdl2-image:x64-windows`

Derleme:
```
Remove-Item -Recurse -Force .\build -ErrorAction SilentlyContinue
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake `
  -DVCPKG_TARGET_TRIPLET=x64-windows
cmake --build build --config Debug
build/Debug/erlik.exe
```
