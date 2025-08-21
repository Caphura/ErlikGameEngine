# ErlikGameEngine — Minimal 2D Engine Skeleton (C++20 + SDL2 + CMake)

Bu repo, PC için kendi 2D engine’ini adım adım geliştirmek üzere **temiz ve küçük bir iskelet** sağlar.
SDL2; pencere, input ve donanım hızlandırmalı 2D renderer için kullanılır. Üzerine sırasıyla kendi modüllerimizi ekleyeceğiz.

Windows + Visual Studio 2022 ile test edildi.

---

## 0) Önkoşullar (Windows)
- Visual Studio 2022 (Desktop development with C++)
- Git + CMake (3.21+)
- vcpkg paket yöneticisi

### vcpkg kurulumu
```powershell
git clone https://github.com/microsoft/vcpkg
cd vcpkg
bootstrap-vcpkg.bat
.\vcpkg integrate install
```
SDL2 kur:
```powershell
vcpkg install sdl2:x64-windows
```

---

## 1) Derleme
Repo kökünde:
```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
build/Debug/erlik.exe
```
1280x720 bir pencere, hareket eden kare ve başlıkta FPS görmelisin.

---

## 2) GitHub (ilk push)
```bash
git init
git add .
git commit -m "chore: initial ErlikGameEngine skeleton (CMake + SDL2)"
git branch -M main
git remote add origin https://github.com/<kullanici-adin>/ErlikGameEngine.git
git push -u origin main
```

---

## 3) Yol Haritası
- [ ] Input katmanı (klavye/fare state)
- [ ] Renderer2D sarmalayıcısı (sprite API, kamera)
- [ ] Asset sistemi (texture yükleme, atlas)
- [ ] Tilemap (CSV/JSON)
- [ ] Çarpışma (AABB, tile collision, raycast)
- [ ] Audio (SDL_mixer/miniaudio)
- [ ] Scene/State sistemi (stack)
- [ ] (Opsiyonel) ECS (entt/flecs)

Küçük ve odaklı commit’ler önerilir.