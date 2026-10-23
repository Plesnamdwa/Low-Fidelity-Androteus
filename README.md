# 🎮 Prototype Game UI & Animasi - Pengalaman Pengguna

[![C++17](https://img.shields.io/badge/Language-C%2B%2B17-blue.svg)](https://isocpp.org/)
[![OpenGL](https://img.shields.io/badge/Graphics-OpenGL-orange.svg)](https://www.opengl.org/)
[![GLFW3](https://img.shields.io/badge/Windowing-GLFW3-green.svg)](https://www.glfw.org/)
[![Platform](https://img.shields.io/badge/Platform-Windows%20x64-lightgrey.svg)](https://www.microsoft.com/windows)
[![Release](https://img.shields.io/badge/Release-v1.0.0-brightgreen.svg)](https://github.com/)

Prototype Game ini dibangun dari dasar menggunakan **C++17** dan **OpenGL** dengan fokus utama pada **tampilan UI, animasi mikro, interaktivitas, konsistensi navigasi, dan error handling** berkonsep **Low Fidelity** sesuai rancangan mockup pengguna.

Aplikasi berjalan pada resolusi paten **1280 x 720 (Fixed Aspect Ratio)** dengan *VSync* aktif untuk memastikan animasi UI bergerak mulus (*smooth 60 FPS*).

---

## 📦 Download Versi Rilis Cepat (Siap Main)

Bagi yang ingin langsung mencoba game tanpa perlu mengompilasi dari kode sumber:
1. Unduh file rilis: **`Pengalaman_Pengguna_Game_Prototype_v1.0.0.zip`** (tersedia di tab [Releases](../../releases) atau di root direktori).
2. Ekstrak file zip ke folder mana saja di komputer Windows Anda.
3. Jalankan **`game_prototype.exe`** (aplikasi *standalone portable*, tidak membutuhkan instalasi compiler atau dependensi DLL tambahan).

---

## ✨ Fitur & Rincian Setiap Fase Permainan

```
┌────────────────────────────────────────────────────────────────────────┐
│                        ALUR PERMAINAN (GAME FLOW)                      │
├─────────────────┬──────────────────┬──────────────────┬────────────────┤
│ 1. MAIN MENU    │ 2. FASE 1: BAKAR │ 3. FASE 2: INTIP │ 4. FASE 3: BOS │
│ Logo Tekstur    │ Obor & Minyak    │ Menyelinap / Eye │ Busur Panah    │
│ Tombol Rounded  │ Pohon Menghitam  │ Kamera Zoom-In   │ Crosshair Paten│
│ Wireframe Pilihan│ Api Dahan Presisi│ Anti-Loop Alert │ HP Drain 5 Dtk │
└─────────────────┴──────────────────┴──────────────────┴────────────────┘
         ▲                                                       │
         └───────────── [ESC] Return Navigation Modal ───────────┘
```

### 1. Menu Utama (*Main Menu*) & Pengaturan (*Settings*)
* **Logo Emblem Bertekstur**: Memuat gambar tekstur asli kepala rusa (`assets/logo.png`) menggunakan modul OpenGL `stb_image` dengan transparansi alpha dan *linear texture filtering* (dilengkapi sistem *fallback* otomatis).
* **Animasi Tombol Dinamis**: Tombol **PLAY**, **SETTING**, dan **EXIT** memiliki efek hover *scaling* ($1.05\times$), transisi warna hangat, dan animasi tekan (*press state*).
* **Latar Belakang Bergerak**: Awan putih yang meluncur halus (*smooth drifting*) di atas bukit hijau dan deretan pohon pinus.
* **Layar Pengaturan (*Low-Fidelity Wireframe*)**: Panel pengaturan konseptual untuk volume BGM, SFX, dan rasio layar.

### 2. Fase 1: Membakar Hutan (*The Burning Phase*)
* **Misi**: Membakar hutan lebat yang menutupi akses gerbang depan kerajaan.
* **Sistem Bahan Bakar Proporsional**: Mengayunkan obor mengurangi bar minyak secara bertahap dan menyisakan tepat **25% (1/4 kapasitas)** ketika pohon habis terbakar (100%).
* **Visualisasi Dahan Hangus**:
  * 16 titik api animasi menempel presisi di dahan dan ranting pohon pinus (tidak ada api yang melayang di udara).
  * Pepohonan pinus secara bertahap berubah warna dari hijau segar menjadi hitam legam hangus (*charcoal black*) seiring bertambahnya persentase terbakar.
* **Absorpsi Klik UI**: Mengklik slot inventori di bawah tidak akan memicu aksi pembakaran pohon di dunia.

### 3. Fase 2: Mengendap-endap (*The Stealth Phase*)
* **Misi**: Menyelinap masuk ke area gerbang kerajaan tanpa memicu kecurigaan penjaga.
* **Kamera Dinamis**: Dunia permainan melakukan *zoom-in* halus (hingga $1.85\times$) terpusat ke arah gerbang emas diiringi efek *head-bobbing* langkah kaki yang natural saat pemain berjalan maju.
* **Mekanik Indikator Mata (Alert Eye Meter)**:
  * Kewaspadaan akan meningkat cepat jika pemain berlari tanpa mengendap atau bergerak membawa senjata terhunus.
  * **Penurunan Alami (*Natural Alert Decay*)**: Mengendap atau berhenti diam (*idle*) akan menurunkan bar kewaspadaan kembali ke 0%.
  * **Pencegahan Infinite Loop Deteksi (*Anti-Forever Loop*)**: Jika kewaspadaan mencapai 100%, senjata pemain otomatis diturunkan ke tangan kosong (`clearSelection`), kewaspadaan di-reset ke 0%, dan diberikan jeda imunitas (*grace cooldown*) selama 2.5 detik.

### 4. Fase 3: Penyerangan Dewa Hutan (*Boss Fight*)
* **Misi**: Mengalahkan Dewa Hutan (Rusa Putih bertanduk megah) menggunakan busur panah.
* **Konstruksi Busur & Anak Panah Realistis**:
  * Anak panah bertengger presisi tepat di titik tengah busur simetris.
  * Tali busur saat istirahat tegak lurus sempurna tanpa tarikan abnormal ke depan, dan melengkung elastis ke belakang saat ditarik.
  * Orientasi sayap bulu panah (*fletching*) condong ke belakang menuju ujung ekor (*nock*).
* **Bidikan Terkunci (*Centered Crosshair*)**: Retikel bidikan merah terkunci permanen di pusat layar `(640, 360)` dan menyempit dinamis saat busur ditarik penuh.
* **Demonstrasi Bar Darah Pemain**: Darah pemain berkurang secara berkala setiap 5 detik (-15 HP) untuk memperlihatkan animasi *depletion* bar HP.
* **Layar Akhir**: Wireframe **VICTORY** saat bos kalah, atau **DEFEAT** saat HP habis, lengkap dengan tombol coba lagi.

### 5. Konsistensi Navigasi Kembali (*Return Navigation System*)
* **Di Dalam Permainan**: Menekan tombol **`[ESC]`** di fase mana pun akan mempause permainan dan menampilkan modal dialog konfirmasi:
  * *"Yakin ingin kembali ke menu utama?"* dengan tombol **`[YES]`** dan **`[CANCEL]`**.
  * Menekan **`[ESC]`** sekali lagi otomatis membatalkan dialog dan melanjutkan permainan.
* **Di Menu Utama**: Menekan tombol **`[ESC]`** atau tombol **`[EXIT]`** menampilkan konfirmasi *"Keluar dari permainan?"*.

---

## ⌨️ Panduan Kontrol Lengkap

| Tombol / Input | Fungsi & Aksi |
| :--- | :--- |
| **Kursor Mouse** | Mengarahkan pandangan & membidik tombol UI |
| **Klik Kiri Mouse** | Memilih tombol UI / Mengayunkan obor / Menahan untuk menarik busur panah |
| **Angka `[1]`** | Memegang Obor (*Torch*) pada slot inventori 1 |
| **Angka `[2]`** | Memegang Busur Panah (*Bow*) pada slot inventori 2 |
| **Tombol `[X]`** | Mengosongkan tangan (*Bare Hands*) |
| **Tombol `[W]` / `[Panah Atas]`** | Berjalan maju menuju gerbang |
| **Tombol `[Left Ctrl]`** | Masuk ke mode mengendap (*Sneak / Crouch*) |
| **Tombol `[Space]`** | Berdiri / bangkit dari mode mengendap |
| **Tombol `[ESC]`** | Navigasi kembali / Menampilkan dialog konfirmasi / Membatalkan dialog |

---

## 🏗️ Arsitektur Kode & Struktur Proyek

```
Pengalaman Pengguna/
├── assets/
│   └── logo.png              # Aset tekstur logo kepala rusa (PNG transparan)
├── release/
│   ├── game_prototype.exe    # Binary portable rilis
│   ├── assets/logo.png       # Aset pendukung binary
│   └── PANDUAN_MAIN.txt      # Panduan singkat pengguna
├── src/
│   ├── main.cpp              # Inisialisasi window GLFW 1280x720, VSync & Game Loop
│   ├── Game.h / Game.cpp     # State machine permainan, logika fase 1-3, Return Navigation
│   ├── Renderer2D.h / .cpp   # Engine grafis 2D, primitif, font bitmap 8x8, pemuat tekstur
│   ├── UI.h / UI.cpp         # Komponen interaktif: UIButton, UIHealthBar, UIFuelBar, UIEyeMeter, dll.
│   └── stb_image.h           # Header library decoding gambar PNG/JPG
├── build.bat                 # Script otomatis kompilasi rilis menggunakan MinGW g++
├── .gitignore                # Filter file binary & artefak lokal untuk Git
├── Pengalaman_Pengguna_Game_Prototype_v1.0.0.zip # Paket rilis mandiri siap pakai
└── README.md                 # Dokumentasi utama repositori
```

---

## 🛠️ Cara Kompilasi dari Source Code

Jika Anda ingin memodifikasi atau mengompilasi ulang kode sumber:

### Prasyarat
* Sistem Operasi: **Windows 10 / 11 (64-bit)**
* Compiler: **MinGW-w64** (mendukung C++17) dengan library GLFW3 terpasang.

### Langkah Kompilasi (1-Klik)
Buka terminal (PowerShell atau Command Prompt) pada direktori proyek, lalu jalankan:
```cmd
build.bat
```
Script akan mengompilasi seluruh file sumber dengan optimasi tinggi (`-O3 -s`), mode native Windows (`-mwindows`), serta *static linking* penuh (`-static -static-libgcc -static-libstdc++`) menghasilkan binary mandiri:
```
game_prototype.exe
```

---

## 📝 Catatan Desain UI/UX
Proyek ini mengedepankan prinsip:
1. **Low-Fidelity Prototyping**: Menekankan kejelasan tata letak elemen visual dan struktur antarmuka sebelum memasuki tahap grafis detail.
2. **Interactive Micro-Animations**: Memberikan umpan balik langsung pada setiap input pemain (hover button scaling, fuel draining, eye alert pulsing, error shake animation).
3. **Forgiving Error Handling**: Memberikan notifikasi toast informatif di area yang tidak menghalangi gameplay, serta mencegah kondisi *stuck* atau *forever loop*.
4. **Consistent Return Pattern**: Navigasi pembatalan dan kembali yang terstandarisasi di seluruh alur permainan melalui tombol `[ESC]`.

---
*Dibuat untuk tugas/evaluasi mata kuliah Pengalaman Pengguna (UI/UX Prototype Game).*
