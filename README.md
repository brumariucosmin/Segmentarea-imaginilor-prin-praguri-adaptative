
# Segmentarea-imaginilor-prin-praguri-adaptative

**Procesor folosit:** Raspberry Pi Pico 2W (RP2350A) *(dacă se dovedește insuficient, proiectul va fi portat pe Raspberry Pi 5)*  
**Algoritm studiat:** Implementarea și optimizarea metodei Otsu pentru binarizarea imaginilor. Algoritmul determină automat pragul optim pentru separarea obiectelor dintr-o imagine, folosind histograma acesteia și minimizarea varianței intra-clasă.

---

## 🎯 Scopul proiectului

- ✅ Implementarea metodei de binarizare Otsu pe **Raspberry Pi Pico 2W**
- ✅ Optimizarea execuției folosind **instrucțiuni DSP și FPU**
- ✅ Compararea performanței metodei pe imagini generate sau reale
- ✅ Evaluarea timpului de execuție și a calității segmentării prin metrici precum **PSNR** și **SSIM**
- ✅ Testarea și diferențierea mai multor versiuni: neoptimizată, semi-optimizată și complet optimizată

---

## 📖 Descrierea metodei Otsu

Algoritmul Otsu caută automat un prag `T` care împarte histograma unei imagini în două clase (`C0` și `C1`), astfel încât varianța între clase (`inter-class variance`) să fie maximizată, iar varianța din interiorul fiecărei clase (`intra-class variance`) să fie minimă.  
Aceasta este o metodă neparametrică și non-iterativă, ideală pentru imagini cu două moduri (foreground și background bine separate).

---

## 🔄 Update 1 – MicroPython (versiunea de bază)
- **Platformă:** Raspberry Pi Pico 2 W (RP2350)
- **Limbaj:** MicroPython
- **Funcționalități:**
  - Downsampling pe fișiere `.pgm`
  - Calcul histograme și prag Otsu
  - Binarizare simplă 1/0
  - Previzualizare ASCII în consolă
  - Măsurare timp execuție (ms)
  - Estimare PSNR vs prag static

### 🧪 Exemplu test:
- Imagine: `coins.ascii.pgm` (300×246)
- Rezultat: binarizare afișată ca matrice 32×32  
  ![image](https://github.com/user-attachments/assets/810e69bf-d375-4f6a-ab9d-ce9417a2026e)

---

## 🔄 Update 2 – Versiune Arduino C/C++ (optimizare completă)

### ✅ Versiunea Optimizată (`versiunea_optimizata.ino`)
- **Limbaj:** C++ cu Arduino IDE
- **Procesor:** ARM Cortex-M33 cu FPU și instrucțiuni DSP (RP2350A)
- **Imagine:** generată local (gradient artificial)
- **Histogramă:** optimizată cu loop unrolling și procesare pe 32 biți
- **Algoritm Otsu:** float și rapid
- **Metode de evaluare:** PSNR + SSIM

#### 🔎 Rezultate:
```
[OPTIMIZED]
Threshold: 124
Execution Time (us): 586
PSNR: 10.81
SSIM: 0.7521

[UNOPTIMIZED]
Threshold: 124
Execution Time (us): 690
PSNR: 10.81
SSIM: 0.7521
```

> ❗ Deși rezultatele binare sunt identice, versiunea optimizată rulează cu ~15% mai rapid.

---

### ⚠️ Versiunea Neoptimizată Intenționat (`versiunea_neoptimizata.ino`)
- **Histogramă calculată** prin căutare lineară cu `for` + `if`, extrem de ineficientă
- **Imagine:** gradient similar
- **Aceleași calcule PSNR/SSIM** și prag Otsu

#### 🔎 Rezultate:
```
[OPTIMIZED]
Execution Time (us): 582

[UNOPTIMIZED - slow]
Execution Time (us): 25012
```

> 🔻 Versiunea neoptimizată a fost încetinită artificial pentru evidențierea diferenței. Rezultatul este același, dar timpul este de ~43× mai mare.

---

## 🔬 Detalii despre Optimizările DSP/FPU

Pe microcontrolerul **RP2350A**, compilatorul din platforma Arduino (Mbed GCC) activează automat extensii ARMv8-M:

| Optimizare / Flag          | Folosit | Detalii tehnice                                                                 |
|---------------------------|---------|----------------------------------------------------------------------------------|
| `-mcpu=cortex-m33`        | ✅      | Activează instrucțiunile specifice Cortex-M33                                   |
| `-mfloat-abi=hard`        | ✅      | Permite utilizarea hardware FPU (fără emulare software)                         |
| `-mfpu=fpv5-sp-d16`       | ✅      | Activează FPU-ul pentru calcule pe `float32`                                    |
| `-march=armv8-m.main+dsp` | ✅      | Permite folosirea instrucțiunilor DSP SIMD în cod                               |
| `-funroll-loops`          | ✅      | Implementat **manual** în cod la histogramă                                     |
| `-ftree-vectorize`        | ⚠️ parțial | Compilatorul poate vectoriza unele bucle, dar limitat pe ARMv8-M               |

### ✅ Optimizări reale în cod:
- `loop unrolling` pe 4 pixeli / iterație pentru histogramă
- `memcpy` și `bitmasking` pentru acces paralel
- `float math` efectuat pe **FPU hardware**
- Evitarea conversiilor inutile sau operațiilor lente

---

## 📚 Concluzii comparative

| Versiune                | Histogramă            | PSNR / SSIM | Timp execuție (us) | Observații                  |
|-------------------------|------------------------|-------------|--------------------|-----------------------------|
| MicroPython             | Simplă                 | Parțial     | ms                 | Implementare de bază        |
| Versiune optimizată     | Loop unrolling (DSP)   | ✔           | ~586               | Rapid, eficient             |
| Versiune semi-optimizată| directă                | ✔           | ~690               | Referință de comparație     |
| Versiune neoptimizată   | lineară + if           | ✔           | ~25012             | Încetinită intenționat      |
