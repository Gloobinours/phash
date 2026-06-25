#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

typedef enum
{
  COLOR_MODE_RGB,
  COLOR_MODE_BNW,
} ColorMode;

typedef struct
{
  uint32_t width;
  uint32_t height;
  uint32_t channels;
  uint8_t* pixels;
} Image;

Image
load_jpeg(const char* filename, ColorMode colorMode);

int32_t
write_jpeg(const char* filename, const Image* image, int32_t quality);

void
free_image(Image* image);

#endif // !IMAGE_H
