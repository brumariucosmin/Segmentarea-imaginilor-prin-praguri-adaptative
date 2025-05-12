#include <Arduino.h>
#include <arm_math.h>

#define IMG_SIZE 64
#define HIST_BINS 256

uint8_t image_copy[IMG_SIZE * IMG_SIZE];  // imagine generată în loc de fingerprint_image
uint8_t binarized[IMG_SIZE * IMG_SIZE];
uint16_t histogram[HIST_BINS];

uint32_t startTime, endTime;

void generate_image() {
  for (int i = 0; i < IMG_SIZE * IMG_SIZE; ++i) {
    image_copy[i] = (i % 64) * 4;  // gradient de la 0 la 252
  }
}

void compute_histogram_optimized() {
  memset(histogram, 0, sizeof(histogram));
  int i = 0;
  int total = IMG_SIZE * IMG_SIZE;
  int limit = total - (total % 4);
  for (; i < limit; i += 4) {
    uint32_t p;
    memcpy(&p, &image_copy[i], sizeof(uint32_t));
    uint8_t p0 = p & 0xFF;
    uint8_t p1 = (p >> 8) & 0xFF;
    uint8_t p2 = (p >> 16) & 0xFF;
    uint8_t p3 = (p >> 24) & 0xFF;
    histogram[p0]++;
    histogram[p1]++;
    histogram[p2]++;
    histogram[p3]++;
  }
  for (; i < total; ++i) {
    histogram[image_copy[i]]++;
  }
}

uint8_t otsu_threshold_optimized() {
  float total = (float)(IMG_SIZE * IMG_SIZE);
  float sum = 0.0f;
  for (int t = 0; t < HIST_BINS; ++t)
    sum += t * (float)histogram[t];

  float sumB = 0.0f, wB = 0.0f, maxVar = 0.0f;
  uint8_t threshold = 0;

  for (int t = 0; t < HIST_BINS; ++t) {
    wB += (float)histogram[t];
    if (wB == 0.0f) continue;
    float wF = total - wB;
    if (wF == 0.0f) break;

    sumB += t * (float)histogram[t];
    float mB = sumB / wB;
    float mF = (sum - sumB) / wF;
    float betweenVar = wB * wF * (mB - mF) * (mB - mF);

    if (betweenVar > maxVar) {
      maxVar = betweenVar;
      threshold = t;
    }
  }
  return threshold;
}

float calculate_psnr() {
  float mse = 0.0f;
  for (int i = 0; i < IMG_SIZE * IMG_SIZE; ++i) {
    float diff = (float)image_copy[i] - (float)(binarized[i] * 255);
    mse += diff * diff;
  }
  mse /= (float)(IMG_SIZE * IMG_SIZE);
  if (mse == 0.0f) return 100.0f;
  return 10.0f * log10f(255.0f * 255.0f / mse);
}

float calculate_ssim() {
  float mean_orig = 0, mean_bin = 0, var_orig = 0, var_bin = 0, covar = 0;
  for (int i = 0; i < IMG_SIZE * IMG_SIZE; ++i) {
    mean_orig += image_copy[i];
    mean_bin += binarized[i] * 255;
  }
  mean_orig /= (IMG_SIZE * IMG_SIZE);
  mean_bin /= (IMG_SIZE * IMG_SIZE);

  for (int i = 0; i < IMG_SIZE * IMG_SIZE; ++i) {
    float o = image_copy[i] - mean_orig;
    float b = binarized[i] * 255 - mean_bin;
    var_orig += o * o;
    var_bin += b * b;
    covar += o * b;
  }

  var_orig /= (IMG_SIZE * IMG_SIZE);
  var_bin /= (IMG_SIZE * IMG_SIZE);
  covar /= (IMG_SIZE * IMG_SIZE);

  const float C1 = 6.5025f, C2 = 58.5225f;
  float ssim = (2 * mean_orig * mean_bin + C1) * (2 * covar + C2);
  ssim /= (mean_orig * mean_orig + mean_bin * mean_bin + C1) * (var_orig + var_bin + C2);
  return ssim;
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  delay(2000);

  generate_image();

  // OPTIMIZED
  startTime = micros();
  compute_histogram_optimized();
  uint8_t threshold = otsu_threshold_optimized();
  for (int i = 0; i < IMG_SIZE * IMG_SIZE; ++i)
    binarized[i] = image_copy[i] > threshold ? 1 : 0;
  endTime = micros();

  Serial.println("[OPTIMIZED]");
  Serial.print("Threshold: "); Serial.println(threshold);
  Serial.print("Execution Time (us): "); Serial.println(endTime - startTime);
  Serial.print("PSNR: "); Serial.println(calculate_psnr(), 2);
  Serial.print("SSIM: "); Serial.println(calculate_ssim(), 4);

  // UNOPTIMIZED
  memset(histogram, 0, sizeof(histogram));
  startTime = micros();
  for (int i = 0; i < IMG_SIZE * IMG_SIZE; ++i)
    histogram[image_copy[i]]++;

  float sum = 0.0f;
  for (int t = 0; t < HIST_BINS; ++t)
    sum += t * (float)histogram[t];

  float sumB = 0.0f, wB = 0.0f, maxVar = 0.0f;
  threshold = 0;
  float total = (float)(IMG_SIZE * IMG_SIZE);
  for (int t = 0; t < HIST_BINS; ++t) {
    wB += (float)histogram[t];
    if (wB == 0.0f) continue;
    float wF = total - wB;
    if (wF == 0.0f) break;
    sumB += t * (float)histogram[t];
    float mB = sumB / wB;
    float mF = (sum - sumB) / wF;
    float betweenVar = wB * wF * (mB - mF) * (mB - mF);
    if (betweenVar > maxVar) {
      maxVar = betweenVar;
      threshold = t;
    }
  }
  for (int i = 0; i < IMG_SIZE * IMG_SIZE; ++i)
    binarized[i] = image_copy[i] > threshold ? 1 : 0;
  endTime = micros();

  Serial.println("[UNOPTIMIZED]");
  Serial.print("Threshold: "); Serial.println(threshold);
  Serial.print("Execution Time (us): "); Serial.println(endTime - startTime);
  Serial.print("PSNR: "); Serial.println(calculate_psnr(), 2);
  Serial.print("SSIM: "); Serial.println(calculate_ssim(), 4);
}

void loop() {}
