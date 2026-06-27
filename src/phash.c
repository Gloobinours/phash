#include "../include/phash.h"
#include "../include/utils.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static float DCT_LUT[32][32];
static int lut_initialized = 0;

static int
load_phash_tables(const char* filepath)
{
  if (lut_initialized)
    return 1;

  FILE* file = fopen(filepath, "rb");
  if (!file) {
    fprintf(stderr, "Critical Error: Could not open %s\n", filepath);
    return 0;
  }

  size_t elements_read = fread(DCT_LUT, sizeof(float), 32 * 32, file);
  fclose(file);

  if (elements_read != 32 * 32) {
    fprintf(stderr,
            "Critical Error: Corrupt or incomplete bin file structure.\n");
    return 0;
  }

  lut_initialized = 1;
  return 1;
}

__attribute__((hot)) void
dct32_optimized(float in[32][32], float out[32][32])
{
  float intermediate[32][32];

  for (int y = 0; y < 32; y++) {
    for (int u = 0; u < 32; u++) {
      float sum = 0.0f;
      for (int x = 0; x < 32; x += 4) {
        sum += in[y][x] * DCT_LUT[u][x];
        sum += in[y][x + 1] * DCT_LUT[u][x + 1];
        sum += in[y][x + 2] * DCT_LUT[u][x + 2];
        sum += in[y][x + 3] * DCT_LUT[u][x + 3];
      }
      intermediate[y][u] = sum;
    }
  }

  for (int v = 0; v < 8; v++) {
    for (int u = 0; u < 8; u++) {
      float sum = 0.0f;
      for (int y = 0; y < 32; y += 4) {
        sum += intermediate[y][u] * DCT_LUT[v][y];
        sum += intermediate[y + 1][u] * DCT_LUT[v][y + 1];
        sum += intermediate[y + 2][u] * DCT_LUT[v][y + 2];
        sum += intermediate[y + 3][u] * DCT_LUT[v][y + 3];
      }
      out[v][u] = sum;
    }
  }
}

void
resize_32x32(const Image* image, float out[32][32])
{
  if (image->channels != 1) {
    fprintf(stderr, "Error: image provided is not BNW\n");
    return;
  }

  const int img_width = image->width;
  const int img_height = image->height;
  const uint8_t* const pixels = image->pixels;

  for (int y = 0; y < 32; y++) {
    int sy = y * img_height / 32;
    const uint8_t* row_pixels = &pixels[sy * img_width];

    for (int x = 0; x < 32; x += 4) {
      out[y][x] = (float)row_pixels[(x * img_width) / 32];
      out[y][x + 1] = (float)row_pixels[((x + 1) * img_width) / 32];
      out[y][x + 2] = (float)row_pixels[((x + 2) * img_width) / 32];
      out[y][x + 3] = (float)row_pixels[((x + 3) * img_width) / 32];
    }
  }
}

uint64_t
phash_from_dct(const float dct[32][32])
{
  float coeffs[63];
  int k = 0;

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (x == 0 && y == 0)
        continue;

      coeffs[k++] = dct[y][x];
    }
  }

  float median = quickselect(coeffs, 63, 31);

  uint64_t hash = 0;
  int bit = 0;

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (x == 0 && y == 0)
        continue;

      if (dct[y][x] > median)
        hash |= (1ULL << bit);

      bit++;
    }
  }

  return hash;
}

uint64_t
phash(const Image* image)
{
  if (!lut_initialized) {
    if (!load_phash_tables("dct_lut.bin")) {
      fprintf(stderr, "Application halting due to missing LUT assets.\n");
      exit(EXIT_FAILURE);
    }
  }

  float resized[32][32];
  float dct[32][32];

  resize_32x32(image, resized);
  dct32_optimized(resized, dct);

  return phash_from_dct(dct);
}

Image
phash_to_image(uint64_t hash)
{
  Image img;

  img.width = 8;
  img.height = 8;
  img.channels = 1;
  img.pixels = malloc(64);

  if (!img.pixels) {
    img.width = img.height = img.channels = 0;
    return img;
  }

  for (int i = 0; i < 64; i++) {
    img.pixels[i] = ((hash >> i) & 1ULL) ? 255 : 0;
  }

  return img;
}

uint32_t
hamming_distance(uint64_t hash1, uint64_t hash2)
{
  return __builtin_popcountll(hash1 ^ hash2);
}
