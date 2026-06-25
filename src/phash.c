#include "../include/phash.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

uint64_t
phash(const Image* image)
{
  float resized[32][32];
  float dct[32][32];

  resize_32x32(image, resized);
  dct32(resized, dct);

  return phash_from_dct(dct);
}

void
resize_32x32(const Image* image, float out[32][32])
{
  if (image->channels != 1) {
    fprintf(stderr, "Error: image provided is not BNW\n");
    return;
  }

  for (int y = 0; y < 32; y++) {
    int sy = y * image->height / 32;

    for (int x = 0; x < 32; x++) {
      int sx = x * image->width / 32;

      out[y][x] = image->pixels[sy * image->width + sx];
    }
  }
}

void
dct32(float in[32][32], float out[32][32])
{
  for (int u = 0; u < 32; u++) {
    for (int v = 0; v < 32; v++) {

      float sum = 0.0f;

      for (int x = 0; x < 32; x++) {
        for (int y = 0; y < 32; y++) {
          sum += in[y][x] * cos((2 * x + 1) * u * M_PI / 64.0) *
                 cos((2 * y + 1) * v * M_PI / 64.0);
        }
      }

      float cu = (u == 0) ? 0.70710678f : 1.0f;
      float cv = (v == 0) ? 0.70710678f : 1.0f;

      out[v][u] = 0.25f * cu * cv * sum;
    }
  }
}

int
compare_float(const void* a, const void* b)
{
  float fa = *(const float*)a;
  float fb = *(const float*)b;
  return (fa > fb) - (fa < fb);
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

  qsort(coeffs, 63, sizeof(float), compare_float);
  float median = coeffs[31];

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
    uint8_t bit = (hash >> i) & 1ULL;

    img.pixels[i] = bit ? 255 : 0;
  }

  return img;
}

uint32_t
hamming_distance(uint64_t hash1, uint64_t hash2)
{
  return __builtin_popcountll(hash1 ^ hash2);
}
