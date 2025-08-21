
# ErlikGameEngine — Step 4: Tilemap (CSV + Tileset)

Bu adımda:
- `Tilemap` sınıfı: CSV'den tile index okur ve tileset PNG'iyle çizer.
- Kamera-odaklı culling (sadece ekranda görünen karolar çizilir).
- Örnek `assets/level1.csv` (60x30) ve `assets/tileset.png` (32x32 grid).
- `Renderer2D`'ye `outputSize()` eklendi.

## Kurulum
SDL2_image zaten kurulu olmalı:
```powershell
vcpkg install sdl2-image:x64-windows
```

## Derleme
```powershell
Remove-Item -Recurse -Force .\build -ErrorAction SilentlyContinue
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake `
  -DVCPKG_TARGET_TRIPLET=x64-windows
cmake --build build --config Debug
build/Debug/erlik.exe
```

## Kullanım
- `assets/level1.csv` hücreleri **-1** boş, `0..N-1` tileset index’i.
- Kamera `F` ile takip aç/kapa.
- Karakter WASD/Ok ile hareket eder; atlas animasyonu açık.
