#include <filesystem>
#include <iostream>

#include "bin_packers/lang.h"
#include "bin_packers/font_glyphs.h"
#include "bin_packers/shader.h"
#include "bin_packers/texture.h"
#include "bin_packers/ui_sprites.h"

int main()
{
    const std::filesystem::path inPath = R"(C:\Projects\Build\in)";
    const std::filesystem::path outPath = R"(C:\Projects\Build\out)";

    std::optional<FontGlyphs> font;
    std::optional<LangKeys> keys;
    std::optional<TextureKeys> blockArrayKeys;
    // First save font glyphs
    font = SaveFontGlyphs(inPath / "ui" / "font_glyphs.json", outPath / "ui" / "font_glyphs.bin");
    // Save languages that depend on the font
    if (font) keys = SaveLanguages(inPath / "lang", *font, outPath / "lang");
    else std::cout << "Could not save languages as they require valid font" << std::endl;

    // Save the rest that doesn't depend on language keys
    SaveUiSprites(inPath / "ui" / "ui_sprites.json", outPath / "ui" / "ui_sprites.bin");

    const shaderc::Compiler compiler;
    SaveShader(compiler, inPath / "shaders", "ui", outPath / "shaders");
    SaveShader(compiler, inPath / "shaders", "opaque_chunk", outPath / "shaders");
    SaveShader(compiler, inPath / "shaders", "chunk", outPath / "shaders");

    SaveTexture(inPath / "ui" / "font_atlas.png", outPath / "ui" / "font_atlas.bin", false);
    SaveTexture(inPath / "ui" / "ui_atlas.png", outPath / "ui" / "ui_atlas.bin", true);
    SaveTexture(inPath / "ui" / "missing_block.png", outPath / "ui" / "missing_block.bin", true);
    blockArrayKeys = SaveTextureArray(inPath / "textures" / "blocks", outPath / "textures" / "block_textures.bin");

    // Save the rest that depends on language keys

   return 0;
}
