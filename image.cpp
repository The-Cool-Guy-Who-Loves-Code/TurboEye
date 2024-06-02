#include "image.h"


void write_png(const char* filename, const unsigned int* image, int width, int height) {
    std::vector<unsigned char> data(width * height * 4);

    // Convert RGBA values from unsigned int to unsigned char
    for (int i = 0; i < width * height; ++i) {
        unsigned int pixel = image[i];
        data[i * 4 + 0] = (pixel >> 16) & 0xFF; // Red
        data[i * 4 + 1] = (pixel >> 8) & 0xFF;  // Green
        data[i * 4 + 2] = pixel & 0xFF;         // Blue
        data[i * 4 + 3] = (pixel >> 24) & 0xFF; // Alpha
    }

    // Write the image as a PNG file
    unsigned error = lodepng::encode(filename, data, width, height);
    if (error) {
        std::cerr << "Error " << error << ": " << lodepng_error_text(error) << std::endl;
    }
}

void read_png(const char* filename, int width, int height, unsigned int* image) {
    int w, h, channels;
    unsigned char* data = stbi_load(filename, &w, &h, &channels, 4); // Force RGBA

    if (!data) {
        std::cerr << "Error: could not load image " << filename << std::endl;
        return;
    }

    // Check if the provided dimensions match the image dimensions
    if (w != width || h != height) {
        std::cerr << "Error: dimensions do not match for image " << filename << std::endl;
        stbi_image_free(data);
        return;
    }

    // Pack the RGBA values into integers
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int index = (y * w + x) * 4;
            unsigned char r = data[index];
            unsigned char g = data[index + 1];
            unsigned char b = data[index + 2];
            unsigned char a = data[index + 3];
            image[y * w + x] = (a << 24) | (r << 16) | (g << 8) | b;
        }
    }

    stbi_image_free(data);
}
