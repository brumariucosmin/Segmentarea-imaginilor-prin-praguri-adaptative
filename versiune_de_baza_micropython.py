import time
import math

def downsample_pgm(filename, target_w=32, target_h=32):
    with open(filename, 'r') as f:
        assert f.readline().strip() == 'P2'
        line = f.readline()
        while line.startswith('#'):
            line = f.readline()
        width, height = [int(i) for i in line.strip().split()]
        maxval = int(f.readline().strip())

        row_skip = height // target_h
        col_skip = width // target_w

        image_small = []
        row_idx = 0
        line_buffer = []
        col_idx = 0
        for line in f:
            for val in line.strip().split():
                if col_idx % col_skip == 0:
                    line_buffer.append(int(val))
                col_idx += 1
                if col_idx == width:
                    col_idx = 0
                    if row_idx % row_skip == 0:
                        image_small.append(line_buffer[:target_w])
                    line_buffer = []
                    row_idx += 1
        return image_small

def otsu_threshold(image):
    pixels = [pix for row in image for pix in row]
    histogram = [0] * 256
    for p in pixels:
        histogram[p] += 1

    total = sum(histogram)
    sumB = 0
    wB = 0
    maximum = 0.0
    sum1 = sum(i * histogram[i] for i in range(256))
    for i in range(256):
        wB += histogram[i]
        if wB == 0:
            continue
        wF = total - wB
        if wF == 0:
            break
        sumB += i * histogram[i]
        mB = sumB / wB
        mF = (sum1 - sumB) / wF
        between = wB * wF * (mB - mF) ** 2
        if between >= maximum:
            threshold = i
            maximum = between
    return threshold

def binarize_image(image, threshold):
    return [[1 if pix >= threshold else 0 for pix in row] for row in image]

def preview_image(binary_image):
    for row in binary_image:
        print(''.join(['#' if val else '.' for val in row]))

def compute_psnr(original, binary):
    mse = 0
    max_pixel = 1
    for r in range(len(original)):
        for c in range(len(original[0])):
            orig_bin = 1 if original[r][c] >= 128 else 0
            mse += (orig_bin - binary[r][c]) ** 2
    mse /= (len(original) * len(original[0]))
    if mse == 0:
        return float('inf')
    psnr = 10 * math.log10((max_pixel ** 2) / mse)
    return psnr

# Main
print("Downsampling and reading image...")
start_time = time.ticks_ms()
image_small = downsample_pgm('coins.ascii.pgm', 32, 32)
threshold = otsu_threshold(image_small)
binary_image = binarize_image(image_small, threshold)
end_time = time.ticks_ms()
exec_time = time.ticks_diff(end_time, start_time)

print("Image size: 32 x 32")
print("Computed Otsu threshold:", threshold)
print("Execution time (ms):", exec_time)
print("Binarized 32x32 image preview:")
preview_image(binary_image)

# Optional: Compute PSNR
psnr = compute_psnr(image_small, binary_image)
print("PSNR (compared to threshold 128 binarization):", psnr)
