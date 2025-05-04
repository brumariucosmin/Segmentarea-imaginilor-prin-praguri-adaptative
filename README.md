# Segmentarea-imaginilor-prin-praguri-adaptative

Procesor folosit: Raspberry Pi Pico 2 (RP2040) (daca nu este suficient de bun, o sa folosesc un raspberry 5) <br/>
Algoritm studiat: Implementarea și optimizarea metodei Otsu pentru binarizarea imaginilor. Algoritmul determină automat pragul optim pentru separarea obiectelor dintr-o imagine, bazându-se pe histograma acesteia și minimizarea varianței intra-clasă.<br/>

Scopul proiectului:<br/>

Implementarea metodei de binarizare Otsu pe Raspberry Pi Pico 2.<br/>
Optimizarea execuției folosind instrucțiuni DSP.<br/>
Compararea performanței metodei pe imagini cu și fără zgomot.<br/>
Evaluarea timpului de execuție și calității segmentării prin metrici precum PSNR și SSIM.<br/>
In functie de timpul ramas, este posibil sa se mai adauge si alte functionalitati, ele vor fi notate pe github.<br/>


Update 1: <br/>
- Platformă: Raspberry Pi Pico 2 W (RP2350) <br/>
- Limbaj: MicroPython <br/>

- Downsampling pe fișiere `.pgm` (de exemplu de la ~300×246 la 32×32) <br/>
- Calcularea histogramei <br/>
- Determinarea pragului optim folosind algoritmul Otsu <br/>
- Binarizarea imaginii (1 / 0) <br/>
- Previzualizare în consolă a imaginii binarizate (`#` = 1, `.` = 0) <br/>
- Măsurarea timpului de execuție (în milisecunde) <br/>
- Estimarea PSNR (Peak Signal-to-Noise Ratio) față de o referință la pragul 128 <br/>

 Exemplu testat: <br/> 
- Imagine de intrare: `coins.ascii.pgm` (300×246, 6 nicheluri și 4 dime-uri) <br/>
- Rezultat: imagine binarizată, afișată în consolă ca matrice 32×32 <br/>
![image](https://github.com/user-attachments/assets/810e69bf-d375-4f6a-ab9d-ce9417a2026e)
