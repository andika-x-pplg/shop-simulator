# 3D Shop Simulator (C++ & raylib)

Game simulasi toko 3D berbasis C++ modern dan raylib tanpa engine berat (Unity/Unreal/Godot).

## Fitur Tahap 1 (Fondasi & Prototype 3D)
- First-person 3D camera controller (WASD + Mouse look).
- Collision detection berbasis AABB (Axis-Aligned Bounding Box) agar pemain tidak menembus dinding dan rak.
- Desain denah toko 3D sederhana: lantai ubin, dinding luar, pintu masuk, lorong rak belanja, meja kasir/counter, dan display tengah.
- Crosshair dan On-screen controls HUD.
- Window management & VSync 60 FPS.

## Struktur Project
```text
shop-simulator/
├── CMakeLists.txt      # Build configuration via CMake & FetchContent raylib
├── README.md           # Dokumentasi project
├── include/            # C++ Header files
│   ├── Common.hpp      # Struktur matematika & AABB bounding box
│   ├── Player.hpp      # Controller first person & collision handler
│   └── Shop.hpp        # Geometri toko, layout rak, dan collider
├── src/                # C++ Source files
│   ├── main.cpp        # Game loop & window initialization
│   ├── Player.cpp      # Implementasi pergerakan & kamera player
│   └── Shop.cpp        # Implementasi render dan collider toko
└── assets/             # Direktori asset masa depan (models, textures, audio)
    ├── fonts/
    ├── models/
    ├── sounds/
    └── textures/
```

## Kontrol
- **W / A / S / D**: Bergerak maju, kiri, mundur, kanan
- **Mouse**: Mengarahkan pandangan kamera (Pitch / Yaw)
- **ESC**: Keluar dari game

## Cara Build & Menjalankan (Windows)

### Prasyarat
- CMake (>= 3.20)
- C++ Compiler (MSVC / Clang / GCC MinGW)
- Ninja / NMake / Visual Studio

### Command Build:
```powershell
# 1. Buat folder build dan konfigurasi CMake
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release

# 2. Build executable
cmake --build build --config Release

# 3. Jalankan game
./build/ShopSimulator.exe
```
