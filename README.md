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
  - **Minuman**: Botol kaleng soda biru-silver (Stok awal: 10, Harga: Rp5.000).
  - **Roti**: Balok roti emas keemasan (Stok awal: 8, Harga: Rp8.000).
  - **Makanan Kaleng**: Kaleng makanan merah-silver (Stok awal: 12, Harga: Rp12.000).
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
- Customer memeriksa stok produk di toko dan hanya memilih rak dengan stok > 0.
- Customer mengambil 1 unit produk, mengurangi stok rak secara sinkron, dan membawa produk 3D di tangan kanan.

## Fitur Tahap 5 & Perbaikan (Sistem Kasir, NPC Kasir, Transaksi, dan Pembayaran)
- **Sistem Harga Produk**:
  - Minuman: Rp5.000
  - Roti: Rp8.000
  - Makanan Kaleng: Rp12.000
- **NPC Kasir Khusus (Dedicated Cashier NPC)**:
  - Berdiri tetap di belakang meja kasir menghadap ke area antrean customer.
  - Tampilan visual eksklusif: Seragam biru tua (*Navy Blue Store Uniform*), celemek merah (*Red Apron*), topi visor toko (*Cap*), tangan yang bersiap di meja kasir, animasi pernapasan/idle halus, serta badge marker biru cyan di atas kepala bertuliskan status peran kasir.
  - Posisi solid dan tidak menghalangi jalur keluar/masuk customer maupun pergerakan pemain.
- **Objek Meja Kasir & POS Machine 3D (Class `Cashier`)**:
  - Meja counter pembayaran dengan mesin POS register, layar monitor hijau menyala, papan tanda `KASIR`, serta collider AABB fisik.
- **Sistem Antrean Kasir (`Customer Queue`)**:
  - Slot 0: Posisi bayar di depan monitor kasir & NPC Kasir.
  - Slot 1-2: Posisi mengantre di belakang. Customer otomatis maju saat antrean di depan kosong.
- **Sistem Transaksi & Saldo Toko (`Shop Treasury`)**:
  - Saldo awal toko: **Rp100.000**.
  - Customer membayar di depan NPC Kasir (1.5 detik proses di POS register).
  - Transaksi diproteksi flag `hasPaid` sehingga **hanya terjadi tepat 1 kali per customer**.
  - Uang hasil penjualan langsung ditambahkan ke saldo toko (`shopMoney += price`).
- **Banner Notifikasi Pembayaran**:
  - Banner hijau di bagian atas layar menampilkan informasi sukses: `"Pembayaran Berhasil! [Nama] membeli [Produk] (+Rp[Harga])"`.
- **Informasi Kasir**:
  - HUD menampilkan saldo uang toko secara realtime (`Uang Toko: RpXXXXX`), jumlah antrean kasir, serta info prompt ketika pemain mendekati area kasir.

## Fitur Tahap 6 (Sistem Supplier, Pengadaan Barang, Delivery, dan Restock)
- **Sistem Supplier & Harga Grosir (`Product.hpp`, `Supplier.hpp`, `Supplier.cpp`)**:
  - Minuman: Harga Beli Supplier = Rp3.000 | Harga Jual Customer = Rp5.000 (Margin Rp2.000)
  - Roti: Harga Beli Supplier = Rp5.000 | Harga Jual Customer = Rp8.000 (Margin Rp3.000)
  - Makanan Kaleng: Harga Beli Supplier = Rp8.000 | Harga Jual Customer = Rp12.000 (Margin Rp4.000)
- **Menu Pengadaan Barang Supplier (Tombol `TAB`)**:
  - Modal UI interaktif untuk memilih produk grosir dan mengatur kuantitas order (+1, -1, +5).
  - Penghitungan total harga pesanan secara otomatis dan realtime.
  - Validasi saldo toko: Pembelian diproteksi dan ditolak jika saldo toko tidak mencukupi, dengan notifikasi *"Uang tidak cukup!"*.
  - Saldo toko dipotong otomatis saat pesanan berhasil dibuat.
- **Sistem Pesanan & Timer Delivery (`OrderStatus`)**:
  - Siklus pesanan: `ORDERED` -> `DELIVERING` (Timer 5 detik) -> `ARRIVED`.
  - Animasi progress bar pengiriman delivery pada overlay menu dan banner notifikasi saat pesanan tiba.
- **Area Penyimpanan / Gudang Toko (`Storage.hpp`, `Storage.cpp`)**:
  - Area gudang 3D di bagian belakang toko dengan 3 pallet kayu khusus untuk tiap jenis produk.
  - Visual tumpukan box/kardus 3D bertingkat yang bertambah dan berkurang secara dinamis sesuai jumlah stok di Storage.
  - Collider fisik AABB agar pemain dan NPC tidak menembus pallet penyimpanan.
- **Pemisahan Sistem Dua Jenis Stok**:
  - **Storage Stock**: Stok cadangan hasil pengadaan supplier di gudang.
  - **Shelf Stock**: Stok pajangan di rak toko yang dapat dibeli oleh customer.
- **Interaksi Restock Manual oleh Player (Tombol `E`)**:
  - Mengambil 1 unit barang dari pallet storage (menampilkan prompt *"Tekan E untuk mengambil [Produk]"*).
  - Membawa produk 3D di tangan ke rak toko yang sesuai.
  - Melakukan restock ke rak dengan menekan `E` (menampilkan prompt *"Tekan E untuk restock [Produk]"*).
  - Validasi kecocokan rak (menolak jika produk tidak sesuai rak) dan kapasitas maksimum rak (menolak jika rak penuh).
  - Pemain dapat mengembalikan barang yang sedang dibawa kembali ke pallet storage.
- **HUD & Status Notifikasi**:
  - Informasi perbandingan realtime stok rak (*Shelf Stock*) dan stok gudang (*Storage Stock*).
  - Banner notifikasi delivery pesanan tiba dan feedback popup interaksi restock.

## Struktur Project
```text
shop-simulator/
├── CMakeLists.txt      # Build configuration via CMake & FetchContent raylib
├── README.md           # Dokumentasi project
├── include/            # C++ Header files
│   ├── Cashier.hpp     # Class Cashier (counter 3D, NPC kasir, POS terminal, antrian, & transaksi)
│   ├── Common.hpp      # Struktur matematika & AABB bounding box
│   ├── Customer.hpp    # Class Customer (FSM, shopping, cashier queue & payment)
│   ├── Player.hpp      # Controller first person, held item & feedback
│   ├── Product.hpp     # Definisi produk, harga jual, harga beli supplier, & visual
│   ├── Rack.hpp        # Class Rack (stok rak, kapasitas maks, visual items di rak & interaksi)
│   ├── Shop.hpp        # Geometri toko, layout rak, kasir, storage pallets & collision list
│   ├── Storage.hpp     # Class Storage (pallet kayu 3D, visual tumpukan kardus & stok gudang)
│   └── Supplier.hpp    # Class Supplier (katalog produk supplier, order queue & delivery timer)
├── src/                # C++ Source files
│   ├── Cashier.cpp     # Render kasir 3D, NPC kasir berseragam, mesin register & pembayaran
│   ├── Customer.cpp    # Navigasi lorong, antrean kasir, checkout & status tag
│   ├── Player.cpp      # Pergerakan, first-person camera & render held item
│   ├── Rack.cpp        # Implementasi render rak bertingkat, batas kapasitas & visual produk
│   ├── Shop.cpp        # Layout toko, penempatan rak, kasir, storage gudang, waypoint & collider
│   ├── Storage.cpp     # Render pallet gudang 3D & manajemen stok gudang
│   ├── Supplier.cpp    # Pengadaan barang, countdown delivery & transfer stok otomatis ke gudang
│   └── main.cpp        # Game loop, menu supplier (TAB), restock loop, notifikasi & HUD
└── assets/             # Direktori aset (models, textures, sounds, fonts)
```

## Kontrol
- **W / A / S / D**: Bergerak maju, kiri, mundur, kanan
- **Mouse**: Mengarahkan pandangan kamera (Pitch / Yaw)
- **E**: Interaksi Player (Ambil dari storage / Restock ke rak / Taruh kembali ke storage)
- **TAB**: Buka / Tutup Menu Pengadaan Barang Supplier
- **1, 2, 3**: Pilih jenis produk di Menu Supplier
- **Panah Atas / Bawah (+ / -)**: Menambah atau mengurangi jumlah pesanan supplier
- **ENTER**: Beli & Bayar Pesanan Supplier
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
