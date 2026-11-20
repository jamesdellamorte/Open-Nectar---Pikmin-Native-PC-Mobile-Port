/* Single translation unit carrying the stb_image implementation, formerly
 * inside pc_texpack.cpp (removed with the OpenGL texture pack). ZipSource and
 * the HD model converter still decode PNG/TGA through stbi_load_from_memory
 * and stbi_zlib_decode_noheader_*. */
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
