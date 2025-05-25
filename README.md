# 🧠 Segmentarea imaginilor prin metoda Otsu optimizată pe Raspberry Pi Pico 2W

**Proiect realizat pentru disciplina: Prelucrarea Semnalelor Digitale pe Sisteme Embedded**  
Autor: BRUMARIU Cosmin-Nicusor  <br>
Platformă: Raspberry Pi Pico 2W (RP2350A, ARM Cortex-M33)

---

## 📌 Descriere generală

Acest proiect demonstrează implementarea și optimizarea metodei de binarizare **Otsu** pe o platformă embedded low-cost: **Raspberry Pi Pico 2 W**. Algoritmul a fost testat pe imagini generate local, evaluate prin metrici precum **PSNR** și **SSIM**, și implementat în două variante: **optimizată** (cu CMSIS-DSP) și **neoptimizată**.

---

## 🎯 Scopul proiectului

- ✅ Implementarea metodei Otsu în C++ cu Arduino IDE
- ✅ Utilizarea funcțiilor CMSIS-DSP pentru accelerare pe ARM Cortex-M33
- ✅ Analiza performanței (timp execuție, PSNR, SSIM)
- ✅ Compararea între versiune optimizată și una ineficientă (de referință)

---

## 🧠 Algoritmul Otsu

Otsu este o metodă automată de binarizare care determină un **prag optim** de separare a pixelilor în fundal și obiecte, pe baza **varianței inter-clasă maxime**. Acesta se aplică pe histograma imaginii.

---

## ⚙️ Detalii tehnice

- **Procesor:** RP2350A (dual-core ARM Cortex-M33, 150 MHz, FPU, DSP)
- **Biblioteci:** `Arduino.h`, `arm_math.h` (CMSIS-DSP)
- **Dimensiune imagine:** 64×64 pixeli
- **Metrici folosiți:** PSNR (calitate), SSIM (similaritate structurală)
- **Funcții CMSIS utilizate:** `arm_mean_f32`, `arm_var_f32`, `arm_power_f32`

---

## 📈 Rezultate obținute (medii pe 10 de imagini simulate cu zgomot variabil)

| Versiune       | Timp mediu execuție (µs) | PSNR mediu (dB) | SSIM mediu | Prag mediu Otsu | Raport viteză (x) |
|----------------|---------------------------|------------------|-------------|------------------|--------------------|
| Optimizată     | 694.16                    | 12.0             | 0.72        | 62.65            | 34.88              |
| Neoptimizată   | 24209.74                  | 12.0             | 0.72        | 62.65            | 1.00               |

> ✅ Rezultatele binare sunt identice. Diferențele apar în viteză de execuție și eficiența calculului.


---

## 📚 Bibliografie

- [1] N. Otsu, “A Threshold Selection Method from Gray-Level Histograms,” IEEE Trans. Systems, Man, Cybernetics, 1979.
- [2] Arm Ltd., “CMSIS-DSP Library,” https://developer.arm.com/Tools%20and%20Software/CMSIS/DSP
- [3] Raspberry Pi Ltd., “RP2040 and Pico Datasheets,” https://www.raspberrypi.com/documentation/
- [4] S. W. Smith, *The Scientist and Engineer’s Guide to DSP*, https://www.dspguide.com

---

## ✅ Concluzie

Proiectul demonstrează eficiența utilizării extensiilor DSP/FPU în prelucrarea imaginilor pe microcontrolere ARM. Prin integrarea CMSIS-DSP, se obține o accelerare semnificativă fără a sacrifica acuratețea. Otsu este implementat clar, modular și evaluat riguros.
