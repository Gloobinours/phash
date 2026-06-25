#include "../include/image.h"
#include "../include/phash.h"
#include <bits/time.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

int
main(int argc, char* argv[])
{
  struct timespec a, b;
  clock_gettime(CLOCK_MONOTONIC, &a);

  char* filepath = argv[1];

  Image image = load_jpeg(filepath, COLOR_MODE_BNW);

  uint64_t hash = phash(&image);

  clock_gettime(CLOCK_MONOTONIC, &b);

  Image hash_image = phash_to_image(hash);

  int32_t status = write_jpeg("out_test.jpeg", &hash_image, 95);

  free_image(&image);

  printf("ms: %.3f\n",
         (b.tv_sec - a.tv_sec) * 1000.0 + (b.tv_nsec - a.tv_nsec) / 1e6);

  printf("Hash: %lu\n", hash);

  return status;
}
