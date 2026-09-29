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
- Model humanoid 3D (kepala sphere, badan cube berkaos warna-warni, kaki beranimasi langkah kaki).
- Sistem navigasi lorong toko tanpa menembus dinding maupun rak.
- Batasan maksimal 3 customer aktif sekaligus dan loop spawner teratur.

## Fitur Tahap 4 (Customer Memilih dan Mengambil Produk)
- **Pemilihan Produk Cerdas (`FindAvailableRackIndex`)**:
  - Customer otomatis memeriksa ketersediaan produk di toko.
  - Customer **hanya memilih rak yang memiliki stok > 0**.
  - Jika rak target habis sebelum customer sampai, customer otomatis mencari rak alternatif yang masih memiliki stok.
  - Jika seluruh rak di toko kosong (`stok = 0`), customer tidak akan macet/stuck dan langsung berjalan keluar toko dengan tertib.
- **Sistem Mengambil Produk & Pengurangan Stok**:
  - Ketika customer berada di depan rak target, customer menunggu sejenak (browsing) lalu mengambil 1 unit produk.
  - Stok rak berkurang 1 secara sinkron dengan sistem produk Tahap 2 (`stock -= 1`), dan visual barang di rak langsung berkurang.
- **Visualisasi Customer Membawa Produk (3D NPC Held Item)**:
  - Produk yang diambil dirender secara 3D di tangan kanan customer sesuai jenisnya (Minuman / Roti / Makanan Kaleng).
  - Indikator status kepala berubah menjadi hijau saat customer berhasil membawa barang belanjaan.
  - Customer membawa barang belanjaan tersebut menyusuri lorong menuju pintu keluar hingga despawn di area luar.
- **HUD Monitoring Belanja**:
  - Menampilkan nama customer, state aktivitas saat ini, dan nama produk yang sedang dibawa.

## Struktur Project
```text
shop-simulator/
├── CMakeLists.txt      # Build configuration via CMake & FetchContent raylib
├── README.md           # Dokumentasi project
├── include/            # C++ Header files
│   ├── Common.hpp      # Struktur matematika & AABB bounding box
│   ├── Customer.hpp    # Class Customer (FSM, shopping inventory & 3D NPC)
│   ├── Player.hpp      # Controller first person, held item & feedback
│   ├── Product.hpp     # Definisi tipe produk, dimensi & properti visual
│   ├── Rack.hpp        # Class Rack (stok, visual items di rak & interaksi)
│   └── Shop.hpp        # Geometri toko, layout rak, waypoint navigasi & collision
├── src/                # C++ Source files
│   ├── main.cpp        # Game loop, customer spawner, shopping loop & HUD
│   ├── Customer.cpp    # Pemilihan produk, navigasi, pengambilan stok & render barang bawaan
│   ├── Player.cpp      # Pergerakan, first-person camera & render held item
│   ├── Rack.cpp        # Implementasi render rak bertingkat & visual produk
│   └── Shop.cpp        # Layout penempatan rak toko, waypoint & collider
└── assets/             # Direktori aset (models, textures, sounds, fonts)
```

## Kontrol
- **W / A / S / D**: Bergerak maju, kiri, mundur, kanan
- **Mouse**: Mengarahkan pandangan kamera (Pitch / Yaw)
- **E**: Interaksi Player (Mengambil produk dari rak / Menaruh produk ke rak)
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
