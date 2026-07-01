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

`Image load_jpeg(char* filename, ColorMode colorMode)` : Loads a JPEG image from the filename (which is truly the path to the image file) and a ColorMode. The color mode must be BNW to perform a phash.
`write_jpeg(char* filename, Image* image, uint32_t quality)` : Writes a JPEG image to the filename (which is also the path). 

