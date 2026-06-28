#include "../include/core/errors.h"
#include "../include/core/image.h"
#include "../include/core/phash.h"
#include <bits/time.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

void
setLogLevel(int argc, char* argv[])
{
  log_set_level(LOG_LEVEL_INFO);

  char* logLevel = "INFO";

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "-d") == 0) {
      log_set_level(LOG_LEVEL_DEBUG);
      logLevel = "DEBUG";
    } else if (strcmp(argv[i], "--quiet") == 0 || strcmp(argv[i], "-q") == 0) {
      log_set_level(LOG_LEVEL_ERROR);
      logLevel = "ERROR";
    }
  }

  LOG_INFO("Log Level: %s", logLevel);
}

int
main(int argc, char* argv[])
{
  setLogLevel(argc, argv);

  struct timespec a, b;
  clock_gettime(CLOCK_MONOTONIC, &a);

  const char* filepath = NULL;

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] != '-') {
      filepath = argv[i];
      break;
    }
  }

  if (filepath == NULL) {
    LOG_ERROR("Usage: %s <input_image.jpg> [--debug | --quiet]", argv[0]);
    return 1;
  }

  Image image = load_jpeg(filepath, COLOR_MODE_BNW);

  uint64_t hash = phash(&image);

  clock_gettime(CLOCK_MONOTONIC, &b);

  Image hash_image = phash_to_image(hash);

  int32_t status = write_jpeg("out_test.jpeg", &hash_image, 95);

  free_image(&image);

  LOG_INFO("ms: %.3f\n",
           (b.tv_sec - a.tv_sec) * 1000.0 + (b.tv_nsec - a.tv_nsec) / 1e6);

  printf("Hash: %lu\n", hash);

  return status;
}
