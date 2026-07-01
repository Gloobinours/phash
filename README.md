# phash

## Image
### Image Specs
Images consist of a heigh, width, color channels and a matrix of pixels.

Image struct
```
struct {
  uint32_t heigh,
  uint32_t width,
  uint32_t channels,
  uint8_t* pixels
}
```
All images can be constructed with a `RGB` or `BNW` (black and white) mode but the phash function will only accept a `BNW` value. 

Color Mode enum
```
enum {
  COLOR_MODE_RGB,
  COLOR_MODE_BNW
}
```
The values in this enum map to the `TJPF_RGB` and `TJPF_GRAY` from the [TurboJPEG api](https://jpeg-turbo.dpldocs.info/libjpeg.turbojpeg.TJPF.html)

### Image Library

| function | description |
| --- | --- |
| `Image load_jpeg(char* filename, ColorMode colorMode)` | Loads a JPEG image from the filename (which is truly the path to the image file) and a ColorMode. The color mode must be BNW to perform a phash. |
| `write_jpeg(char* filename, Image* image, uint32_t quality)` | Writes a JPEG image to the filename (which is also the path). |

## Perceptuyal Hashsing Computation

The p-hash of a picture is a 64 bits integer. A [hamming distance](https://en.wikipedia.org/wiki/Hamming_distance) can be computed with a reference and a source image to get their dissimilarity. A value equal to 0 means 100% similariy.

| function | description |
| --- | --- |
| `uint64_t phash(Image* image)` | Creates a perceptual hash for the given image. This function only allow images in `BNW` color mode. |
| `uint32_t hamming_distance(uint64_t hash1, uint64_t hash2)` | Computes the hamming distance from 2 hashes. |
| `Image phash_to_image(uint64_t hash)` | Creates an image representation of the hash. |
