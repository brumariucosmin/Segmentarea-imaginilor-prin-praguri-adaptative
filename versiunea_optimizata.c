#include <Arduino.h>
#include <arm_math.h>
#include <math.h>

#define IMG_SIZE 64
#define HIST_BINS 256

uint8_t image_copy[IMG_SIZE * IMG_SIZE];
uint8_t binarized[IMG_SIZE * IMG_SIZE];
uint16_t histogram[HIST_BINS];

float32_t float_image[IMG_SIZE * IMG_SIZE];
float32_t float_binarized[IMG_SIZE * IMG_SIZE];

uint32_t startTime, endTime;

void generate_image() {
  for (int y = 0; y < IMG_SIZE; ++y) {
    for (int x = 0; x < IMG_SIZE; ++x) {
      image_copy[y * IMG_SIZE + x] = (x < IMG_SIZE / 2) ? 50 : 200;
    }
  }
}

void compute_histogram_optimized() {
  memset(histogram, 0, sizeof(histogram));
  for (int i = 0; i < IMG_SIZE * IMG_SIZE; ++i) {
    histogram[image_copy[i]]++;
  }
}

uint8_t otsu_threshold_optimized() {
  float32_t total = (float32_t)(IMG_SIZE * IMG_SIZE);
  float32_t sum = 0.0f;
  for (int t = 0; t < HIST_BINS; ++t)
    sum += t * (float32_t)histogram[t];

  float32_t sumB = 0.0f, wB = 0.0f, maxVar = 0.0f;
  uint8_t threshold = 0;

  for (int t = 0; t < HIST_BINS; ++t) {
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
  return threshold;
}

float calculate_psnr_cmsis() {
  float32_t diff[IMG_SIZE * IMG_SIZE];
  float32_t mse = 0.0f, log_val;

  for (int i = 0; i < IMG_SIZE * IMG_SIZE; ++i) {
    float_image[i] = (float32_t)image_copy[i];
    float_binarized[i] = (float32_t)(binarized[i] * 255);
    diff[i] = float_image[i] - float_binarized[i];
  }

  arm_power_f32(diff, IMG_SIZE * IMG_SIZE, &mse);
  mse /= (float32_t)(IMG_SIZE * IMG_SIZE);

  if (mse == 0.0f) return 100.0f;

  float32_t ratio = (255.0f * 255.0f) / mse;
  log_val = log10f(ratio);
  return 10.0f * log_val;
}

float calculate_ssim_cmsis() {
  float32_t mean_orig, mean_bin, var_orig, var_bin, covar = 0.0f;

  for (int i = 0; i < IMG_SIZE * IMG_SIZE; ++i) {
    float_image[i] = (float32_t)image_copy[i];
    float_binarized[i] = (float32_t)(binarized[i] * 255);
  }

  arm_mean_f32(float_image, IMG_SIZE * IMG_SIZE, &mean_orig);
  arm_mean_f32(float_binarized, IMG_SIZE * IMG_SIZE, &mean_bin);
  arm_var_f32(float_image, IMG_SIZE * IMG_SIZE, &var_orig);
  arm_var_f32(float_binarized, IMG_SIZE * IMG_SIZE, &var_bin);

  for (int i = 0; i < IMG_SIZE * IMG_SIZE; ++i) {
    covar += (float_image[i] - mean_orig) * (float_binarized[i] - mean_bin);
  }
  covar /= (float32_t)(IMG_SIZE * IMG_SIZE);

  const float32_t C1 = 6.5025f, C2 = 58.5225f;
  float32_t numerator = (2 * mean_orig * mean_bin + C1) * (2 * covar + C2);
  float32_t denominator = (mean_orig * mean_orig + mean_bin * mean_bin + C1) * (var_orig + var_bin + C2);
  return numerator / denominator;
}

float calculate_psnr_basic() {
  float mse = 0.0f;
  for (int i = 0; i < IMG_SIZE * IMG_SIZE; ++i) {
    float a = (float)image_copy[i];
    float b = (float)(binarized[i] * 255);
    float diff = a - b;
    mse += diff * diff;
  }
  mse /= (float)(IMG_SIZE * IMG_SIZE);
  if (mse == 0.0f) return 100.0f;
  float ratio = (255.0f * 255.0f) / mse;
  return 10.0f * log10f(ratio);
}

float calculate_ssim_basic() {
  float mean_orig = 0.0f, mean_bin = 0.0f;
  for (int i = 0; i < IMG_SIZE * IMG_SIZE; ++i) {
    mean_orig += image_copy[i];
    mean_bin += binarized[i] * 255;
  }
  mean_orig /= (float)(IMG_SIZE * IMG_SIZE);
  mean_bin /= (float)(IMG_SIZE * IMG_SIZE);

  float var_orig = 0.0f, var_bin = 0.0f, covar = 0.0f;
  for (int i = 0; i < IMG_SIZE * IMG_SIZE; ++i) {
    float a = (float)image_copy[i];
    float b = (float)(binarized[i] * 255);
    var_orig += (a - mean_orig) * (a - mean_orig);
    var_bin += (b - mean_bin) * (b - mean_bin);
    covar += (a - mean_orig) * (b - mean_bin);
  }
  var_orig /= (float)(IMG_SIZE * IMG_SIZE);
  var_bin /= (float)(IMG_SIZE * IMG_SIZE);
  covar /= (float)(IMG_SIZE * IMG_SIZE);

  const float C1 = 6.5025f, C2 = 58.5225f;
  float numerator = (2 * mean_orig * mean_bin + C1) * (2 * covar + C2);
  float denominator = (mean_orig * mean_orig + mean_bin * mean_bin + C1) * (var_orig + var_bin + C2);
  return numerator / denominator;
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  delay(2000);

  generate_image();

  // Optimizat
  startTime = micros();
  compute_histogram_optimized();
  uint8_t threshold = otsu_threshold_optimized();
  for (int i = 0; i < IMG_SIZE * IMG_SIZE; ++i)
    binarized[i] = image_copy[i] > threshold ? 1 : 0;
  float psnr_opt = calculate_psnr_cmsis();
  float ssim_opt = calculate_ssim_cmsis();
  endTime = micros();

  Serial.println("[OPTIMIZED]");
  Serial.print("Threshold: "); Serial.println(threshold);
  Serial.print("Execution Time (us): "); Serial.println(endTime - startTime);
  Serial.print("PSNR: "); Serial.println(psnr_opt, 2);
  Serial.print("SSIM: "); Serial.println(ssim_opt, 4);

  // Neoptimizat
  memset(histogram, 0, sizeof(histogram));
  startTime = micros();
  for (int i = 0; i < IMG_SIZE * IMG_SIZE; ++i) {
    for (int h = 0; h < HIST_BINS; ++h) {
      if (h == image_copy[i]) {
        histogram[h]++;
        break;
      }
    }
  }

  float32_t total = (float32_t)(IMG_SIZE * IMG_SIZE);
  float32_t sum = 0.0f;
  for (int t = 0; t < HIST_BINS; ++t)
    sum += t * (float32_t)histogram[t];

  float32_t sumB = 0.0f, wB = 0.0f, maxVar = 0.0f;
  threshold = 0;
  for (int t = 0; t < HIST_BINS; ++t) {
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

  for (int i = 0; i < IMG_SIZE * IMG_SIZE; ++i)
    binarized[i] = image_copy[i] > threshold ? 1 : 0;

  float psnr_unopt = calculate_psnr_basic();
  float ssim_unopt = calculate_ssim_basic();
  endTime = micros();

  Serial.println("[UNOPTIMIZED]");
  Serial.print("Threshold: "); Serial.println(threshold);
  Serial.print("Execution Time (us): "); Serial.println(endTime - startTime);
  Serial.print("PSNR: "); Serial.println(psnr_unopt, 2);
  Serial.print("SSIM: "); Serial.println(ssim_unopt, 4);
}

void loop() {}