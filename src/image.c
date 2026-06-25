#include "../include/image.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <turbojpeg.h>

Image
load_jpeg(const char* filename, ColorMode colorMode)
{
  Image image = { 0, 0, 0, NULL };

  FILE* file = fopen(filename, "rb");
  if (file == NULL) {
    fprintf(stderr, "Error: can't open %s\n", filename);
    return image;
  }

  fseek(file, 0, SEEK_END);
  int64_t jpeg_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  uint8_t* jpeg_buffer = (uint8_t*)malloc((size_t)jpeg_size);
  if (!jpeg_buffer) {
    fclose(file);
    return image;
  }

  fread(jpeg_buffer, 1, (size_t)jpeg_size, file);
  fclose(file);

  tjhandle decompressor = tjInitDecompress();
  int32_t subsampling, width, height;

  uint32_t res = tjDecompressHeader2(decompressor,
                                     jpeg_buffer,
                                     (unsigned long)jpeg_size,
                                     &width,
                                     &height,
                                     &subsampling);
  if (res != 0) {
    fprintf(stderr, "Decompression failed: %s\n", tjGetErrorStr());
    free(jpeg_buffer);
    tjDestroy(decompressor);
    return image;
  }

  uint32_t tj_pixel_format;

  image.width = width;
  image.height = height;

  switch (colorMode) {
    case COLOR_MODE_BNW:
      image.channels = 1;
      tj_pixel_format = TJPF_GRAY;
      break;
    case COLOR_MODE_RGB:
    default:
      image.channels = 3;
      tj_pixel_format = TJPF_RGB;
      break;
  }

  // This can proly overflow for big images
  size_t buffer_size = (size_t)(image.width * image.height * image.channels);

  image.pixels = (uint8_t*)malloc(buffer_size);

  if (!image.pixels) {
    free(jpeg_buffer);
    tjDestroy(decompressor);
    return image;
  }

  int32_t result = tjDecompress2(decompressor,
                                 jpeg_buffer,
                                 (unsigned long)jpeg_size,
                                 image.pixels,
                                 image.width,
                                 0,
                                 image.height,
                                 tj_pixel_format,
                                 TJFLAG_FASTDCT);

  free(jpeg_buffer);
  tjDestroy(decompressor);

  if (result != 0) {
    free(image.pixels);
    image.pixels = NULL;
  }

  return image;
}

int32_t
write_jpeg(const char* filename, const Image* image, int32_t quality)
{
  if (!image || !image->pixels || image->width == 0 || image->height == 0) {
    return EXIT_FAILURE;
  }

  int32_t tj_pixel_format;
  int32_t tj_subsampling;

  switch (image->channels) {
    case 1:
      tj_pixel_format = TJPF_GRAY;
      tj_subsampling = TJSAMP_GRAY;
      break;
    case 3:
      tj_pixel_format = TJPF_RGB;
      tj_subsampling = TJSAMP_420;
      break;
    default:
      return EXIT_FAILURE;
  }

  tjhandle compressor = tjInitCompress();
  if (!compressor) {
    return EXIT_FAILURE;
  }

  uint8_t* jpeg_buffer = NULL;
  unsigned long jpeg_size = 0;

  int32_t result = tjCompress2(compressor,
                               image->pixels,
                               (int)image->width,
                               0,
                               (int)image->height,
                               tj_pixel_format,
                               &jpeg_buffer,
                               &jpeg_size,
                               tj_subsampling,
                               quality,
                               0);

  if (result != 0) {
    tjDestroy(compressor);
    return EXIT_FAILURE;
  }

  FILE* file = fopen(filename, "wb");
  if (!file) {
    tjFree(jpeg_buffer);
    tjDestroy(compressor);
    return EXIT_FAILURE;
  }

  size_t written = fwrite(jpeg_buffer, 1, jpeg_size, file);
  fclose(file);

  tjFree(jpeg_buffer);
  tjDestroy(compressor);

  if (written != jpeg_size) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

void
free_image(Image* image)
{
  if (image == NULL) {
    return;
  }

  if (image->pixels != NULL) {
    free(image->pixels);
    image->pixels = NULL;
  }

  image->width = 0;
  image->height = 0;
  image->channels = 0;
}
