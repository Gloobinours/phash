#ifndef PHASH_H
#define PHASH_H

#include "image.h"

int
load_phash_tables(const char* filepath);

uint64_t
phash(const Image* image);

void
resize_32x32(const Image* image, float out[32][32]);

void
dct32(float in[32][32], float out[32][32]);

uint64_t
phash_from_dct(const float dct[32][32]);

Image
phash_to_image(uint64_t hash);

uint32_t
hamming_distance(uint64_t hash1, uint64_t hash2);

#endif // !PHASH_H
