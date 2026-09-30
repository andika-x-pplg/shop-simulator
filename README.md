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

## Fitur Tahap 7 (Sistem Ekonomi Toko, Keuntungan, Kerugian, dan Manajemen Harga)
- **Satu Sumber Data Keuangan Toko (`Finance.hpp`, `Finance.cpp`)**:
  - `currentBalance`: Saldo uang toko (dimulai dari Rp100.000).
  - `totalRevenue`: Akumulasi seluruh pendapatan kotor dari transaksi customer.
  - `totalExpenses`: Akumulasi seluruh pengeluaran pengadaan barang dari supplier.
  - `totalProfit`: Keuntungan bersih real-time yang dihitung secara matematis: `Profit = Total Revenue - Total Expenses`.
  - Riwayat 10 transaksi finansial terakhir (*audit trail* pemasukan & pengeluaran).
- **Sistem Pendapatan & Pengeluaran Tepat 1 Kali**:
  - Pembayaran customer di kasir menambahkan `totalRevenue` dan `currentBalance` tepat satu kali per transaksi (`+Rp[Harga] Pendapatan`).
  - Pembelian order supplier menambahkan `totalExpenses` dan mengurangi `currentBalance` tepat satu kali saat order dibuat (`-Rp[Biaya] Pengadaan`).
  - Tiba/selesainya delivery barang di storage tidak memotong saldo lagi.
- **Sistem Manajemen Harga Jual (`PriceManager.hpp`, `PriceManager.cpp`)**:
  - Menu interaktif Manajemen Harga (Tombol `P`) untuk mengatur harga jual masing-masing produk:
    - **Minuman**: Modal Rp3.000 | Jual default Rp5.000 (Margin Rp2.000)
    - **Roti**: Modal Rp5.000 | Jual default Rp8.000 (Margin Rp3.000)
    - **Makanan Kaleng**: Modal Rp8.000 | Jual default Rp12.000 (Margin Rp4.000)
  - Pengubahan harga jual dinamis (`+-Rp500`) menggunakan tombol `A / D / Panah`.
  - Validasi harga: Harga jual diproteksi tidak boleh 0 atau bernilai negatif (minimal Rp500).
  - Indikator peringatan jika harga jual disetel di bawah harga modal beli: *(Peringatan: harga jual di bawah harga beli)*.
  - Perhitungan margin profit per unit secara otomatis: `Margin = Harga Jual - Harga Beli`.
- **Integrasi Transaksi Customer & Kasir Dinamis**:
  - Kasir (`Cashier::ProcessPayment`) selalu membaca harga jual paling mutakhir dari `PriceManager`.
  - Customer membayar produk dengan harga jual terbaru yang telah disesuaikan oleh pemain.
  - Harga grosir supplier tetap stabil dan tidak terpengaruh oleh penyesuaian harga jual toko.
- **Menu Ringkasan Keuangan Toko (Tombol `F`)**:
  - Tampilan modal ringkasan keuangan real-time dengan kartu visual metrik:
    - Saldo Toko (*Current Balance*)
    - Total Pendapatan (*Total Revenue*)
    - Total Pengeluaran (*Total Expenses*)
    - Keuntungan Bersih (*Profit*) / Kerugian (*Loss*)
  - Daftar riwayat transaksi pemasukan dan pengeluaran terbaru.
- **Banner Notifikasi Transaksi & Finansial**:
  - Notifikasi instan saat customer membayar (`+Rp5000 Pendapatan`), saat order supplier (`-Rp30000 Pengadaan`), dan saat mengubah harga jual (`Harga Minuman diubah menjadi Rp6000`).
- **HUD Bersih & Ringkas**:
  - HUD utama tetap fokus menampilkan saldo toko aktual (`Uang Toko: RpXXXXX`) tanpa mengaburkan pandangan permainan.

## Fitur Tahap 8 (Sistem Kepuasan Customer, Rating, dan Reputasi Toko)
- **Skor Kepuasan Customer / Satisfaction (`0 - 100`)**:
  - Setiap customer yang masuk memiliki kepuasan awal bernilai **100%** (Sangat Puas).
  - Skor kepuasan dipengaruhi secara dinamis oleh pengalaman berbelanja:
    - **Berhasil Mendapatkan Produk**: Bonus `+10` kepuasan.
    - **Berhasil Membayar di Kasir**: Bonus `+10` kepuasan.
    - **Rak Pilihan Kosong / Pindah Rak**: Penalti `-10` kepuasan.
    - **Semua Produk Habis (Keluar Tanpa Beli)**: Penalti `-20` kepuasan.
    - **Waktu Antrean Kasir**:
      - `<= 5 detik`: Tidak ada penalti.
      - `5 - 10 detik`: Penalti `-5` kepuasan.
      - `10 - 20 detik`: Penalti `-10` kepuasan kumulatif.
      - `> 20 detik`: Penalti `-20` kepuasan kumulatif.
  - Skor diproteksi fungsi clamp: `0 <= satisfaction <= 100`.
  - Sistem penalti dan bonus menerapkan flag *single-trigger* sehingga tidak dieksekusi berulang per frame.
- **Konversi Rating Bintang Customer (`1 - 5 Bintang`)**:
  - Dihitung saat customer selesai berbelanja dan melangkah keluar toko:
    - `90 - 100`: **5 Bintang** (*"Customer sangat puas!"*)
    - `75 - 89`: **4 Bintang** (*"Customer puas."*)
    - `55 - 74`: **3 Bintang** (*"Customer cukup puas."*)
    - `35 - 54`: **2 Bintang** (*"Customer kurang puas."*)
    - `0 - 34`: **1 Bintang** (*"Customer kecewa."*)
  - Satu customer dijamin hanya memberikan rating tepat **1 kali**.
- **Sistem Rating Rata-Rata & Reputasi Toko (`Reputation.hpp`, `Reputation.cpp`)**:
  - **Reputasi Toko (`0 - 100`)**: Dimulai dari nilai awal **50**.
  - Reputasi bertumbuh secara bertahap dan adil berdasarkan rating customer:
    - `5 Bintang`: `+3` Reputasi
    - `4 Bintang`: `+1` Reputasi
    - `3 Bintang`: `0` Perubahan
    - `2 Bintang`: `-2` Reputasi
    - `1 Bintang`: `-3` Reputasi
  - **Rata-rata Rating Toko (`Average Rating`)**: Dihitung dari `Total Rating Points / Total Ratings` (aman terhadap pembagian nol jika belum ada customer).
- **Menu Reputasi & Rating Toko (Tombol `R`)**:
  - Modal UI interaktif untuk meninjau status reputasi toko:
    - Kartu Reputasi Toko Real-Time (`XX / 100`).
    - Kartu Rata-rata Rating Bintang (`X.X / 5.0`).
    - Kartu Total Customer yang Memberikan Ulasan (`XX Ulasan`).
    - Riwayat ulasan bintang, feedback teks, dan persentase kepuasan customer terbaru.
- **Banner Notifikasi Feedback & Rating**:
  - Menampilkan ulasan instan saat customer selesai: `"[Nama] memberi rating X/5 (Feedback)"`.
- **Integrasi HUD Ringkas**:
  - Menampilkan ringkasan status toko secara elegan pada panel kontrol:
    - `Uang Toko: RpXXXXX`
    - `Rating: X.X/5 | Reputasi: XX/100`

## Fitur Tahap 9 (Sistem Upgrade Toko & Furniture/Peralatan)
- **Sistem Upgrade Toko Berlevel (`ShopUpgrade.hpp`, `ShopUpgrade.cpp`)**:
  - **Shop Size Upgrade (Level 1-3)**:
    - Level 1: Ukuran Toko Standar (20m x 24m)
    - Level 2: Toko Diperluas (25m x 28m) - Biaya Rp 200.000
    - Level 3: Toko Mega Luas (30m x 32m) - Biaya Rp 450.000
    - Visual dinding dan lantai 3D toko meregang/membesar secara dinamis dan aman tanpa merusak posisi rak, kasir, storage, maupun waypoint navigasi customer.
  - **Shelf Capacity Upgrade (Level 1-3)**:
    - Level 1: 15 unit/rak
    - Level 2: 25 unit/rak - Biaya Rp 75.000
    - Level 3: 40 unit/rak - Biaya Rp 150.000
    - Secara dinamis memperbarui kapasitas maksimum seluruh rak produk (`Rack::SetMaxStock`).
  - **Storage Capacity Upgrade (Level 1-3)**:
    - Level 1: 50 unit gudang
    - Level 2: 100 unit gudang - Biaya Rp 80.000
    - Level 3: 180 unit gudang - Biaya Rp 160.000
    - Mengatur batas total daya tampung pallet storage gudang (`Storage::SetMaxCapacity`).
  - **Customer Capacity Upgrade (Level 1-3)**:
    - Level 1: Maksimal 3 customer simultan
    - Level 2: Maksimal 5 customer simultan - Biaya Rp 100.000
    - Level 3: Maksimal 8 customer simultan - Biaya Rp 220.000
- **Sistem Furniture 3D (`Furniture.hpp`, `Furniture.cpp`)**:
  - Objek 3D berestetika tinggi yang dirender di posisi terencana (*predefined layout*) tanpa menghalangi alur customer, kasir, maupun pemain:
    - **Meja Toko (Store Table)** - Rp 50.000: Meja kayu dengan kaki kokoh.
    - **Kursi Pelanggan (Customer Chair)** - Rp 30.000: Kursi kayu dengan sandaran.
    - **Rak Pajangan Ekstra (Display Shelf)** - Rp 100.000: Rak display tambahan dengan bingkai rapi.
    - **Kabinet Penyimpanan (Storage Cabinet)** - Rp 150.000: Lemari kayu dengan gagang perak.
    - **Tanaman Hias (Decoration Plant)** - Rp 40.000: Pot tanaman hijau penyegar suasana toko.
  - Setiap furniture yang telah dibeli otomatis memiliki collider fisik AABB sehingga pemain tidak menembus objek.
- **Sistem Equipment / Peralatan Toko**:
  - **Display Berkualitas (Better Display)** - Rp 120.000: Menambah bonus kapasitas rak (+2 kapasitas tambahan).
  - **Rak Storage Ekstra (Extra Storage Rack)** - Rp 140.000: Menambah kapasitas storage (+15 kapasitas gudang).
  - **Mesin Kasir Canggih (Better Cashier Equipment)** - Rp 180.000: Meningkatkan kepuasan customer saat checkout (+5% kepuasan).
- **Menu Toko & Pengelolaan Finansial Terpadu**:
  - **Menu Upgrade Toko (Tombol `U`)**: Navigasi upgrade 4 kategori dengan status level saat ini, biaya upgrade berikutnya, dan label *MAX LEVEL*.
  - **Menu Furniture & Equipment (Tombol `B`)**: Navigasi katalog furniture dan equipment dengan status kepemilikan (*Owned / Not Owned*).
  - **Integrasi Keuangan 100% Terpadu**: Menggunakan `Finance::currentBalance` dan otomatis mencatat ke `Finance::totalExpenses` tanpa mata uang duplikat.
- **HUD & Notifikasi Tahap 9**:
  - Banner feedback pembelian dan upgrade (*"Upgrade Berhasil!"*, *"Furniture Berhasil Dibeli!"*, *"Saldo tidak cukup!"*, *"Sudah MAX LEVEL"*).
  - HUD real-time menampilkan `Level Toko: Lv.X | Rak: Lv.X | Gudang: Lv.X`.

## Fitur Tahap 10 (Sistem Hari, Waktu, Jam Buka/Tutup Toko & Statistik Harian)
- **Sistem Waktu Game Berbasis DeltaTime (`GameTime.hpp`, `GameTime.cpp`)**:
  - Skala waktu presisi: **1 detik waktu nyata = 1 menit waktu game** (independen dari variasi FPS).
  - Format jam digital: `HH:MM` (misal `08:00`, `12:30`, `20:00`, `21:00`).
  - Proteksi waktu: pergantian menit ke jam (`00 - 59`) tanpa nilai invalid.
- **Sistem Hari Operasional Toko**:
  - Dimulai dari **Day 1 (08:00)**.
  - Memulai hari berikutnya setelah tutup toko: **Day 2 (08:00)** -> **Day 3 (08:00)**, dan seterusnya.
- **Jam Buka & Tutup Toko**:
  - **Jam Buka (OPEN)**: `08:00` (Status HUD: `STATUS TOKO: OPEN (08:00 - 21:00)` dengan warna hijau cerah).
  - **Jam Tutup (CLOSED)**: `21:00` (Status HUD: `STATUS TOKO: CLOSED` dengan warna merah).
  - **Pemberitahuan Waktu (Time Warnings)**:
    - `20:00`: Notifikasi banner oranye *"1 jam lagi toko tutup (21:00)."* (Muncul tepat 1 kali).
    - `20:30`: Notifikasi banner merah *"Toko akan segera tutup!"* (Muncul tepat 1 kali).
    - `21:00`: Notifikasi penutupan *"Toko ditutup."* & aktivasi Daily Summary Modal.
- **Customer Spawner & Behavior Saat Jam Tutup**:
  - Customer baru **hanya boleh spawn saat status toko OPEN**.
  - Saat toko CLOSED (`21:00`), customer baru dilarang masuk toko (`spawnTimer` dihentikan).
  - Customer yang sudah berada di dalam toko **tetap diberi kesempatan menyelesaikan prosesnya dengan aman** (mencari barang, mengambil, mengantre kasir, dan membayar) tanpa terhapus tiba-tiba.
- **Perubahan Visual Waktu / Visual Time Change**:
  - **Morning (08:00 - 11:59)**: Warna langit biru cerah pagi (*Sky Blue*).
  - **Day (12:00 - 17:59)**: Warna langit siang terang (*Bright Day Blue*).
  - **Evening / Night (18:00 - 21:00)**: Warna langit sore/malam (*Dark Twilight*).
- **Statistik Harian & Ringkasan Hari (`DailyStats.hpp`, `DailyStats.cpp`)**:
  - Pemisahan data harian (*Daily*) dengan data akumulasi permanen (*Total*):
    - `dailyRevenue`: Pendapatan kotor penjualan hari ini.
    - `dailyExpenses`: Pengeluaran supplier / upgrade / furniture hari ini.
    - `dailyProfit`: Keuntungan bersih hari ini (`Revenue - Expenses`).
    - `dailyCustomers`: Jumlah customer yang berhasil dilayani hari ini.
    - `dailyRatings`: Riwayat ulasan dan rata-rata rating kepuasan hari ini.
  - **Daily Summary Modal**:
    - Otomatis muncul pada pukul `21:00` menampilkan kartu metrik operasional hari tersebut.
    - Menampilkan ringkasan ulasan customer dan saldo akhir toko.
    - Menekan tombol `ENTER` / `SPASI` akan memulai hari berikutnya (Day bertambah 1, jam kembali ke 08:00, toko kembali OPEN, dan daily stats di-reset).
  - **Integritas Ekonomi & Progres Permanen**:
    - Reset harian **TIDAK PERNAH** menghapus saldo toko (`currentBalance`), total revenue/expenses permanen, upgrade toko, furniture/peralatan, harga jual produk, reputasi toko, maupun stok gudang/rak.
- **Aktivitas Toko Saat CLOSED**:
  - Pemain tetap leluasa bergerak, melakukan restock rak dari storage, memesan barang di supplier (timer delivery tetap berjalan normal), mengatur harga jual, serta membeli furniture/upgrade.

## Struktur Project
```text
shop-simulator/
├── CMakeLists.txt      # Build configuration via CMake & FetchContent raylib
├── README.md           # Dokumentasi project
├── include/            # C++ Header files
│   ├── Cashier.hpp     # Class Cashier (counter 3D, NPC kasir, POS terminal, antrian, & transaksi)
│   ├── Common.hpp      # Struktur matematika & AABB bounding box
│   ├── Customer.hpp    # Class Customer (FSM, shopping, cashier queue, payment, satisfaction & rating)
│   ├── DailyStats.hpp  # Pencatatan statistik harian & rendering modal daily summary
│   ├── Finance.hpp     # Single source of truth keuangan (saldo, revenue, expenses, profit/loss)
│   ├── Furniture.hpp   # Class Furniture & Equipment (render 3D, status kepemilikan, collider & bonus)
│   ├── GameTime.hpp    # Sistem waktu (deltaTime), hari operasional, jam buka/tutup & sky visual
│   ├── Player.hpp      # Controller first person, held item & feedback
│   ├── PriceManager.hpp# Manajemen harga jual produk, validasi harga & margin unit
│   ├── Product.hpp     # Definisi produk, harga jual, harga beli supplier, & visual
│   ├── Rack.hpp        # Class Rack (stok rak, kapasitas maks, visual items di rak & interaksi)
│   ├── Reputation.hpp  # Sistem reputasi toko, rata-rata rating, konversi bintang & ulasan customer
│   ├── Shop.hpp        # Geometri toko, dynamic size level, layout rak, kasir & collision list
│   ├── ShopUpgrade.hpp # Sistem upgrade toko berlevel (size, shelf, storage, customer capacity)
│   ├── Storage.hpp     # Class Storage (pallet kayu 3D, kapasitas dinamis & stok gudang)
│   └── Supplier.hpp    # Class Supplier (katalog produk supplier, order queue & delivery timer)
├── src/                # C++ Source files
│   ├── Cashier.cpp     # Render kasir 3D, NPC kasir berseragam, mesin register & pembayaran
│   ├── Customer.cpp    # Navigasi lorong, antrean kasir, checkout, scoring kepuasan & rating
│   ├── DailyStats.cpp  # Implementasi agregasi statistik harian & render modal ringkasan harian
│   ├── Finance.cpp     # Pencatatan transaksi pendapatan, pengeluaran & perhitungan profit
│   ├── Furniture.cpp   # Render 3D furniture/equipment, collider generator & bonus logic
│   ├── GameTime.cpp    # Perhitungan waktu 1s = 1m game, transisi hari, peringatan jam & sky
│   ├── Player.cpp      # Pergerakan, first-person camera & render held item
│   ├── PriceManager.cpp# Penyesuaian harga jual, validasi batas & kalkulasi margin
│   ├── Rack.cpp        # Implementasi render rak bertingkat, batas kapasitas dinamis & visual produk
│   ├── Reputation.cpp  # Kalkulasi rata-rata rating, penyesuaian reputasi toko & riwayat ulasan
│   ├── Shop.cpp        # Dynamic shop expansion, penempatan rak, kasir, storage, waypoint & collider
│   ├── ShopUpgrade.cpp # Logika upgrade toko, validasi balance, & modal UI upgrade
│   ├── Storage.cpp     # Render pallet gudang 3D, kapasitas maksimum & manajemen stok gudang
│   ├── Supplier.cpp    # Pengadaan barang, countdown delivery & transfer stok otomatis ke gudang
│   └── main.cpp        # Game loop, integrasi waktu, jam buka/tutup, modal UI (U/B/TAB/P/F/R/Summary), & HUD
└── assets/             # Direktori aset (models, textures, sounds, fonts)
```

## Kontrol
- **W / A / S / D**: Bergerak maju, kiri, mundur, kanan
- **Mouse**: Mengarahkan pandangan kamera (Pitch / Yaw)
- **E**: Interaksi Player (Ambil dari storage / Restock ke rak / Taruh kembali ke storage)
- **U**: Buka / Tutup Menu Upgrade Toko (Shop Size, Shelf, Storage, Customer)
- **B**: Buka / Tutup Menu Beli Furniture & Peralatan (Shop Catalog)
- **TAB**: Buka / Tutup Menu Pengadaan Barang Supplier
- **P**: Buka / Tutup Menu Manajemen Harga Jual
- **F**: Buka / Tutup Menu Ringkasan Keuangan Toko
- **R**: Buka / Tutup Menu Reputasi & Rating Toko
- **ENTER / SPASI**: Memulai Hari Berikutnya (di Daily Summary) / Beli Item / Konfirmasi Order
- **ESC**: Tutup Menu Aktif / Keluar dari game

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
