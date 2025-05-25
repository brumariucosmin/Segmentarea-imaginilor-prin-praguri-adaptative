#include <Arduino.h>           // Include biblioteca de bază pentru plăcile Arduino, oferind funcții esențiale precum setup(), loop() și cele pentru comunicarea serială.
#include <arm_math.h>          // Include biblioteca CMSIS-DSP (Cortex Microcontroller Software Interface Standard - Digital Signal Processing), care conține funcții matematice optimizate pentru procesoarele ARM Cortex-M, utilizate pentru operații DSP rapide (ex. media, varianța, puterea).
#include <math.h>              // Include biblioteca standard C pentru funcții matematice generale, cum ar fi logaritmul în bază 10 (log10f()).

// Dimensiunea imaginii: 64x64 pixeli
#define IMG_SIZE 64            // Definește o constantă pentru dimensiunea laturii imaginii (64 pixeli), rezultând o imagine de 64x64 pixeli.
// Numărul de clase din histogramă (niveluri de intensitate): 256 (0–255)
#define HIST_BINS 256          // Definește numărul de "coșuri" (bins) pentru histogramă, corespunzător celor 256 de niveluri de intensitate posibile (0-255) pentru o imagine pe 8 biți.

// Imaginea originală (tonuri de gri)
uint8_t image_copy[IMG_SIZE * IMG_SIZE]; // Declară un tablou unidimensional de tip uint8_t (octet fără semn) pentru a stoca pixelii imaginii originale în tonuri de gri (valori 0-255).
// Imaginea binarizată (0 sau 1, după prag)
uint8_t binarized[IMG_SIZE * IMG_SIZE];  // Declară un tablou unidimensional de tip uint8_t pentru a stoca pixelii imaginii binarizate (unde valorile sunt fie 0 - negru, fie 1 - alb).
// Histogramă: număr de pixeli pentru fiecare intensitate
uint16_t histogram[HIST_BINS];           // Declară un tablou de tip uint16_t (întreg fără semn pe 16 biți) pentru a stoca histograma imaginii; fiecare element va număra pixelii cu o anumită intensitate.

// Variații în float32 pentru calcule CMSIS-DSP
float32_t float_image[IMG_SIZE * IMG_SIZE];      // Declară un tablou de tip float32_t (float pe 32 de biți) pentru a stoca imaginea originală convertită la float, necesar pentru funcțiile CMSIS-DSP.
float32_t float_binarized[IMG_SIZE * IMG_SIZE];  // Declară un tablou de tip float32_t pentru a stoca imaginea binarizată convertită la float, necesar pentru funcțiile CMSIS-DSP.

// Variabile pentru măsurarea timpului
uint32_t startTime, endTime; // Declară variabile de tip uint32_t pentru a stoca timpii de început și de sfârșit, măsurați în microsecunde, pentru benchmarking.

// -------------------------
// 1. Generarea unei imagini de test
// Creează o imagine 64x64: stânga cu intensitate 50, dreapta cu 200
void generate_image() { // Definește funcția generate_image care creează o imagine sintetică pentru testare.
  for (int y = 0; y < IMG_SIZE; ++y) { // Prima buclă for iterează pe rânduri (coordonata y).
    for (int x = 0; x < IMG_SIZE; ++x) { // A doua buclă for iterează pe coloane (coordonata x) pentru fiecare rând.
      if (x < IMG_SIZE / 2) // Verifică dacă pixelul curent se află în jumătatea stângă a imaginii.
        image_copy[y * IMG_SIZE + x] = 50; // Dacă da, setează intensitatea pixelului la 50 (un gri închis).
      else // Dacă pixelul se află în jumătatea dreaptă.
        image_copy[y * IMG_SIZE + x] = 200; // Setează intensitatea pixelului la 200 (un gri deschis).
    }
  }
}

// -------------------------
// 2. Histogramă optimizată
void compute_histogram_optimized() { // Definește funcția compute_histogram_optimized pentru a calcula histograma imaginii într-un mod eficient.
  memset(histogram, 0, sizeof(histogram));  // Inițializează toate elementele tabloului histogram cu zero, asigurând o stare curată.
  for (int i = 0; i < IMG_SIZE * IMG_SIZE; ++i) { // Buclează prin fiecare pixel al imaginii (stocată liniar).
    histogram[image_copy[i]]++;          // Incrementează contorul corespunzător valorii de intensitate a pixelului curent în histogramă (folosește valoarea pixelului ca index).
  }
}

// -------------------------
// 3. Algoritmul Otsu optimizat (calculează automat pragul de binarizare)
uint8_t otsu_threshold_optimized() { // Definește funcția otsu_threshold_optimized care implementează algoritmul lui Otsu pentru a găsi automat cel mai bun prag de binarizare.
  float32_t total = (float32_t)(IMG_SIZE * IMG_SIZE); // Calculează numărul total de pixeli din imagine și îl convertește la float32_t.
  float32_t sum = 0.0f; // Inițializează o variabilă pentru suma ponderată a tuturor intensităților pixelilor.

  // Suma ponderată a tuturor intensităților
  for (int t = 0; t < HIST_BINS; ++t) // Buclează prin toate intensitățile posibile (0-255).
    sum += t * (float32_t)histogram[t]; // Adună la 'sum' produsul dintre intensitatea curentă 't' și numărul de pixeli cu acea intensitate.

  float32_t sumB = 0.0f, wB = 0.0f, maxVar = 0.0f; // Inițializează sumB (suma ponderată a fundalului), wB (greutatea fundalului) și maxVar (varianta inter-clasă maximă).
  uint8_t threshold = 0; // Inițializează variabila 'threshold' care va stoca pragul optim.

  // Iterează prin toate pragurile posibile
  for (int t = 0; t < HIST_BINS; ++t) { // Buclează prin fiecare valoare posibilă de prag (de la 0 la 255).
    wB += (float32_t)histogram[t];  // Acumulează greutatea (numărul de pixeli) pentru clasa de fundal (pixeli cu intensitatea <= t).
    if (wB == 0.0f) continue; // Dacă greutatea fundalului este zero, trece la următorul prag (nu există pixeli în clasa fundalului).
    float32_t wF = total - wB;      // Calculează greutatea (numărul de pixeli) pentru clasa de obiect (pixeli cu intensitatea > t).
    if (wF == 0.0f) break; // Dacă greutatea obiectului este zero, oprește bucla (toți pixelii sunt în clasa fundalului).

    sumB += t * (float32_t)histogram[t]; // Acumulează suma ponderată a intensităților pentru clasa de fundal.
    float32_t mB = sumB / wB;               // Calculează media intensităților pentru clasa de fundal.
    float32_t mF = (sum - sumB) / wF;       // Calculează media intensităților pentru clasa de obiect.
    float32_t betweenVar = wB * wF * (mB - mF) * (mB - mF); // Calculează varianța inter-clasă, măsura de separare între cele două clase.

    if (betweenVar > maxVar) { // Dacă varianța inter-clasă curentă este mai mare decât maximul înregistrat.
      maxVar = betweenVar; // Actualizează varianța maximă.
      threshold = t; // Actualizează pragul optim.
    }
  }
  return threshold; // Returnează pragul de binarizare optim găsit de algoritmul Otsu.
}

// -------------------------
// 4. Calcul PSNR (calitate) folosind CMSIS-DSP
float calculate_psnr_cmsis() { // Definește funcția calculate_psnr_cmsis pentru a calcula PSNR (Peak Signal-to-Noise Ratio), o metrică de calitate a imaginii.
  float32_t diff[IMG_SIZE * IMG_SIZE]; // Declară un tablou pentru a stoca diferențele dintre pixelii imaginii originale și cei binarizați.
  float32_t mse = 0.0f, log_val; // Inițializează MSE (Mean Squared Error) și o variabilă temporară pentru logaritm.

  // Calculează diferența dintre imaginea originală și cea binarizată
  for (int i = 0; i < IMG_SIZE * IMG_SIZE; ++i) { // Buclează prin fiecare pixel.
    float_image[i] = (float32_t)image_copy[i]; // Convertește pixelul original la float.
    float_binarized[i] = (float32_t)(binarized[i] * 255);  // Convertește pixelul binarizat (0 sau 1) la float (0 sau 255).
    diff[i] = float_image[i] - float_binarized[i]; // Calculează diferența între pixelii corespunzători.
  }

  // MSE = media pătratelor diferențelor
  arm_power_f32(diff, IMG_SIZE * IMG_SIZE, &mse); // Utilizează funcția CMSIS-DSP pentru a calcula suma pătratelor elementelor din tabloul 'diff'.
  mse /= (float32_t)(IMG_SIZE * IMG_SIZE); // Împarte suma pătratelor la numărul total de pixeli pentru a obține MSE.

  // Dacă MSE e 0 (imaginile identice), PSNR maxim
  if (mse == 0.0f) return 100.0f; // Dacă MSE este zero (imaginile sunt identice), returnează o valoare foarte mare pentru PSNR (semnificând o potrivire perfectă).

  float32_t ratio = (255.0f * 255.0f) / mse; // Calculează raportul necesar pentru formula PSNR (MAX_I^2 / MSE).
  log_val = log10f(ratio); // Calculează logaritmul în bază 10 al raportului.
  return 10.0f * log_val; // Returnează valoarea finală a PSNR conform formulei standard.
}

// -------------------------
// 5. Calcul SSIM (similaritate structurală) folosind CMSIS-DSP
float calculate_ssim_cmsis() { // Definește funcția calculate_ssim_cmsis pentru a calcula SSIM (Structural Similarity Index Measure), o metrică de similaritate structurală.
  float32_t mean_orig, mean_bin, var_orig, var_bin, covar = 0.0f; // Declară variabile pentru mediile, varianțele și covarianța celor două imagini.

  // Conversie imagini în float
  for (int i = 0; i < IMG_SIZE * IMG_SIZE; ++i) { // Buclează prin fiecare pixel.
    float_image[i] = (float32_t)image_copy[i]; // Convertește pixelul original la float.
    float_binarized[i] = (float32_t)(binarized[i] * 255); // Convertește pixelul binarizat la float (0 sau 255).
  }

  // Media și varianța folosind CMSIS
  arm_mean_f32(float_image, IMG_SIZE * IMG_SIZE, &mean_orig); // Calculează media imaginii originale folosind funcția CMSIS-DSP.
  arm_mean_f32(float_binarized, IMG_SIZE * IMG_SIZE, &mean_bin); // Calculează media imaginii binarizate folosind funcția CMSIS-DSP.
  arm_var_f32(float_image, IMG_SIZE * IMG_SIZE, &var_orig); // Calculează varianța imaginii originale folosind funcția CMSIS-DSP.
  arm_var_f32(float_binarized, IMG_SIZE * IMG_SIZE, &var_bin); // Calculează varianța imaginii binarizate folosind funcția CMSIS-DSP.

  // Covarianță
  for (int i = 0; i < IMG_SIZE * IMG_SIZE; ++i) { // Buclează prin fiecare pixel pentru a calcula covarianța.
    covar += (float_image[i] - mean_orig) * (float_binarized[i] - mean_bin); // Acumulează produsele diferențelor față de medie.
  }
  covar /= (float32_t)(IMG_SIZE * IMG_SIZE); // Împarte la numărul de pixeli pentru a obține covarianța.

  // Formula SSIM
  const float32_t C1 = 6.5025f, C2 = 58.5225f; // Definește constantele C1 și C2 utilizate în formula SSIM pentru stabilitate numerică.
  float32_t numerator = (2 * mean_orig * mean_bin + C1) * (2 * covar + C2); // Calculează numărătorul formulei SSIM.
  float32_t denominator = (mean_orig * mean_orig + mean_bin * mean_bin + C1) * (var_orig + var_bin + C2); // Calculează numitorul formulei SSIM.
  return numerator / denominator; // Returnează valoarea finală a SSIM.
}

// -------------------------
// 6. Funcția setup() – punctul de pornire al programului
void setup() { // Funcția setup() este apelată o singură dată la pornirea sau resetarea plăcii Arduino.
  Serial.begin(115200);      // Inițializează comunicația serială la o rată de 115200 baud, pentru a trimite date către monitorul serial.
  while (!Serial);           // Așteaptă până când portul serial este deschis (important pentru unele plăci Arduino).
  delay(2000);               // O pauză de 2 secunde pentru a asigura inițializarea completă a portului serial.

  generate_image();          // Apeleză funcția pentru a genera imaginea de test.

  // ----- Execuție optimizată -----
  startTime = micros();                  // Înregistrează timpul de început al execuției optimizate în microsecunde.
  compute_histogram_optimized();         // Calculează histograma folosind metoda optimizată.
  uint8_t threshold = otsu_threshold_optimized(); // Calculează pragul de binarizare folosind algoritmul Otsu optimizat.
  for (int i = 0; i < IMG_SIZE * IMG_SIZE; ++i) // Buclează prin toți pixelii imaginii.
    binarized[i] = image_copy[i] > threshold ? 1 : 0; // Binarizează imaginea: 1 dacă pixelul este mai mare decât pragul, altfel 0.
  endTime = micros();                      // Înregistrează timpul de sfârșit al execuției optimizate.

  Serial.println("[OPTIMIZED]"); // Afișează un antet pentru rezultatele optimizate.
  Serial.print("Threshold: "); Serial.println(threshold); // Afișează pragul de binarizare calculat.
  Serial.print("Execution Time (us): "); Serial.println(endTime - startTime); // Afișează timpul total de execuție pentru secțiunea optimizată.
  Serial.print("PSNR: "); Serial.println(calculate_psnr_cmsis(), 2); // Afișează valoarea PSNR, rotunjită la 2 zecimale.
  Serial.print("SSIM: "); Serial.println(calculate_ssim_cmsis(), 4); // Afișează valoarea SSIM, rotunjită la 4 zecimale.

  // ----- Execuție neoptimizată -----
  memset(histogram, 0, sizeof(histogram));    // Resetează histograma la zero pentru o nouă măsurătoare.
  startTime = micros();                    // Înregistrează timpul de început al execuției neoptimizate.

  // Histogramă ineficientă (căutare brută)
  for (int i = 0; i < IMG_SIZE * IMG_SIZE; ++i) { // Buclează prin fiecare pixel.
    for (int h = 0; h < HIST_BINS; ++h) { // Buclează prin toate coșurile histogramei pentru fiecare pixel (metodă ineficientă).
      if (h == image_copy[i]) { // Verifică dacă valoarea coșului se potrivește cu intensitatea pixelului.
        histogram[h]++; // Dacă se potrivește, incrementează contorul pentru acea intensitate.
        break; // Odată găsită potrivirea, se iese din bucla interioară.
      }
    }
  }

  // Recalculare prag (cod duplicat pentru a simula ciclul complet neoptimizat)
  float32_t total = (float32_t)(IMG_SIZE * IMG_SIZE); // Re-calculează numărul total de pixeli.
  float32_t sum = 0.0f; // Re-inițializează suma ponderată.
  for (int t = 0; t < HIST_BINS; ++t) // Re-calculează suma ponderată a tuturor intensităților.
    sum += t * (float32_t)histogram[t];

  float32_t sumB = 0.0f, wB = 0.0f, maxVar = 0.0f; // Re-inițializează variabilele pentru algoritmul Otsu.
  threshold = 0; // Re-inițializează pragul.
  for (int t = 0; t < HIST_BINS; ++t) { // Re-execută algoritmul Otsu.
    wB += (float32_t)histogram[t];
    if (wB == 0.0f) continue;
    float32_t wF = total - wB;
    if (wF == 0.0f) break;
    sumB += t * (float32_t)histogram[t];
    float32_t mB = sumB / wB;
    float32_t mF = (sum - sumB) / wF;
    float32_t betweenVar = wB * wF * (mB - mF) * (mB - mF);
    if (betweenVar > maxVar) {
      maxVar = betweenVar;
      threshold = t;
    }
  }

  for (int i = 0; i < IMG_SIZE * IMG_SIZE; ++i) // Re-binarizează imaginea cu pragul recalculat.
    binarized[i] = image_copy[i] > threshold ? 1 : 0;

  endTime = micros();                      // Înregistrează timpul de sfârșit al execuției neoptimizate.

  Serial.println("[UNOPTIMIZED]"); // Afișează un antet pentru rezultatele neoptimizate.
  Serial.print("Threshold: "); Serial.println(threshold); // Afișează pragul de binarizare calculat.
  Serial.print("Execution Time (us): "); Serial.println(endTime - startTime); // Afișează timpul total de execuție pentru secțiunea neoptimizată.
  Serial.print("PSNR: "); Serial.println(calculate_psnr_cmsis(), 2); // Afișează valoarea PSNR.
  Serial.print("SSIM: "); Serial.println(calculate_ssim_cmsis(), 4); // Afișează valoarea SSIM.
}

// -------------------------
// 7. Loop – gol, deoarece codul rulează o singură dată
void loop() {} // Funcția loop() este goală deoarece toată logica programului este executată o singură dată în funcția setup().