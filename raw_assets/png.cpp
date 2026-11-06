#include "raw_assets/png.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Png ReadRgbaPng(const std::filesystem::path& path)
{
    int w, h, c;
    stbi_uc* data = stbi_load(path.string().c_str(), &w, &h, &c, STBI_rgb_alpha);
    if (!data) return {};

    Png png;
    png.width = w;
    png.height = h;
    png.channels = 4;

    const size_t size = static_cast<size_t>(w) * h * 4;
    png.data.assign(data, data + size);

    stbi_image_free(data);

    return png;
}

Png ReadAlphaPng(const std::filesystem::path& path)
{
    int w, h, c;
    stbi_uc* data = stbi_load(path.string().c_str(), &w, &h, &c, STBI_rgb_alpha);
    if (!data) return {};
    const stbi_uc* src = data;

    Png png;
    png.width = w;
    png.height = h;
    png.channels = 1;

    png.data.resize(w * h);
    for (size_t i = 0; i < w * h; i++, src += 4)
    {
        png.data[i] = src[3]; // Extract only alpha channel
    }
    stbi_image_free(data);
    return png;
}