# 3D Shop Simulator (C++ & raylib)

Game simulasi toko 3D modern berbasis C++17 dan raylib 5.0 tanpa game engine berat (Unity/Unreal/Godot).

## Fitur Tahap 1 (Fondasi & Lingkungan 3D)
- First-person 3D camera controller (WASD + Mouse look dengan pitch clamping).
- Collision detection berbasis AABB (Axis-Aligned Bounding Box) untuk dinding dan objek.
- Desain toko 3D (lantai grid ubin, dinding pembatas, pintu masuk, counter kasir).
- On-screen HUD, Crosshair, dan FPS counter.

## Fitur Tahap 2 (Sistem Rak, Produk, Stok & Interaksi)
- **Sistem Rak (Class `Rack`)**: Rak 3D bertingkat dengan papan penanda kategori produk dan collider AABB.
- **Sistem Produk (Header `Product.hpp`)**:
  - **Minuman**: Botol kaleng soda biru-silver (Stok awal: 10).
  - **Roti**: Balok roti emas keemasan (Stok awal: 8).
  - **Makanan Kaleng**: Kaleng makanan merah-silver (Stok awal: 12).
- **Tampilan Visual Produk di Rak**: Menampilkan barisan item 3D di atas rak yang menyesuaikan dengan jumlah stok yang tersedia.
- **Interaksi Tombol `E`**:
  - Deteksi arah pandang dan jarak interaksi (<= 3.5m) ke rak produk.
  - Prompt interaksi kontekstual di layar ("Tekan E untuk mengambil [Produk]", "Tekan E untuk menaruh [Produk]").
  - Crosshair berubah warna (hijau) ketika mengarahkan pandangan ke rak yang bisa diinteraksikan.
- **Sistem Membawa 1 Produk**:
  - Pemain memegang 1 produk di tangan yang dirender secara 3D di sudut pandang first-person kamera.
  - Slot membawa tunggal (Tangan penuh tidak bisa mengambil produk lain).
  - Mengembalikan produk ke rak yang sesuai menambah stok rak dan mengosongkan tangan.
  - Pesan peringatan jika rak habis, penuh, atau jenis produk tidak cocok.

## Fitur Tahap 3 (Sistem Customer / NPC Dasar)
- **Class `Customer`**: Model humanoid 3D (kepala sphere, badan cube berkaos warna-warni, kaki beranimasi langkah kaki, dan marker status di atas kepala).
- **Customer Lifecycle & Finite State Machine (FSM)**:
  - `ENTERING`: Muncul di luar toko, berjalan menuju pintu masuk toko.
  - `WALKING_TO_SHELF`: Masuk melewati pintu, berjalan di lorong toko menuju rak target.
  - `AT_SHELF`: Berhenti di depan rak selama 3.5 - 5.5 detik untuk melihat produk.
  - `LEAVING`: Berjalan dari rak kembali ke lorong utama menuju pintu keluar.
  - `EXITING`: Keluar melewati pintu toko ke area luar.
  - `DESPAWNED`: Hilang setelah mencapai titik despawn luar.
- **Spawn Loop & Batasan**:
  - Maksimal 3 customer aktif sekaligus di dalam/luar toko.
  - Customer baru di-spawn secara berkala dengan variasi nama, warna pakaian, dan pemilihan rak target yang acak/berotasi.
- **HUD Customer Tracker**: Menampilkan jumlah customer aktif (`Customer Aktif: X/3`) dan status/nama customer terkini.

## Struktur Project
```text
shop-simulator/
├── CMakeLists.txt      # Build configuration via CMake & FetchContent raylib
├── README.md           # Dokumentasi project
├── include/            # C++ Header files
│   ├── Common.hpp      # Struktur matematika & AABB bounding box
│   ├── Customer.hpp    # Class Customer (FSM, waypoints, & visual NPC)
│   ├── Player.hpp      # Controller first person, held item & feedback
│   ├── Product.hpp     # Definisi tipe produk, dimensi & properti visual
│   ├── Rack.hpp        # Class Rack (stok, visual items di rak & interaksi)
│   └── Shop.hpp        # Geometri toko, layout rak, waypoint navigasi & collision
├── src/                # C++ Source files
│   ├── main.cpp        # Game loop, customer spawner, input handling & HUD
│   ├── Customer.cpp    # Navigasi waypoint NPC, update state & render 3D
│   ├── Player.cpp      # Pergerakan, first-person camera & render held item
│   ├── Rack.cpp        # Implementasi render rak bertingkat & visual produk
│   └── Shop.cpp        # Layout penempatan rak toko, waypoint & collider
└── assets/             # Direktori aset (models, textures, sounds, fonts)
```

## Kontrol
- **W / A / S / D**: Bergerak maju, kiri, mundur, kanan
- **Mouse**: Mengarahkan pandangan kamera (Pitch / Yaw)
- **E**: Interaksi (Mengambil produk dari rak / Menaruh produk ke rak)
- **ESC**: Keluar dari game

## Cara Build & Menjalankan (Windows)

### Command Build:
```powershell
# 1. Konfigurasi CMake
$env:Path = "C:\Program Files\CMake\bin;C:\mingw64\bin;" + $env:Path
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"

# 2. Build executable
cmake --build build --config Release

# 3. Jalankan game
.\build\ShopSimulator.exe
```
