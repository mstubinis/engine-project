#include "core/engine/utils/PrecompiledHeader.h"
#include <core/engine/fonts/Font.h>
#include <core/engine/system/Engine.h>
#include <core/engine/textures/Texture.h>

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/filesystem.hpp>

#include <ft2build.h>
#include <freetype/freetype.h> //can be replaced with #include FT_FREETYPE_H

#include <SFML/Graphics/Image.hpp>

using namespace Engine;
using namespace std;

Font* first_font = nullptr;

Font::Font(const string& filename, int height, int width, float line_height) : Resource(ResourceType::Font, filename) {
    init(filename, height, width);
    m_LineHeight = line_height;
}
Font::~Font(){ 
}

void Font::init(const std::string& filename, int height, int width) {
    string extension = boost::filesystem::extension(filename);
    /*
    TODO:
        CID-keyed Type 1 fonts
        SFNT-based bitmap fonts, including color Emoji
        Type 42 fonts (limited support)
    */
    if (extension == ".fnt") { //Windows FNT fonts
        init_simple(filename, height, width);
    }else if (extension == ".ttf" || extension == ".ttc") { //TrueType fonts (TTF) and TrueType collections (TTC)
        init_freetype(filename, height, width);
    }else if (extension == ".cff") { //CFF fonts
        init_freetype(filename, height, width);
    }else if (extension == ".pcf") { //X11 PCF fonts
        init_freetype(filename, height, width);
    }else if (extension == ".woff") { //WOFF fonts
        init_freetype(filename, height, width);
    }else if (extension == ".otf" || extension == ".otc") { //OpenType fonts (OTF, both TrueType and CFF variants) and OpenType collections (OTC)
        init_freetype(filename, height, width);
    }else if (extension == ".pfa" || extension == ".pfb") { //Type 1 fonts (PFA and PFB)
        init_freetype(filename, height, width);
    }else if (extension == ".bdf") { //BDF fonts (including anti-aliased ones)
        init_freetype(filename, height, width);
    }else if (extension == ".pfr") { //PFR fonts
        init_freetype(filename, height, width);
    }

    if (!first_font) {
        first_font = this;
    }
}

vector<vector<unsigned char>> Font::generate_bitmap(const FT_GlyphSlotRec_& glyph) {
    vector<vector<unsigned char>> pixels;
    pixels.resize(glyph.bitmap.rows);
    for (unsigned int i = 0; i < pixels.size(); ++i) {
        pixels[i].resize(glyph.bitmap.width, 0_uc);
    }
    auto image_pixel_size = (glyph.bitmap.rows * glyph.bitmap.width) * 1; //last number = color components (so far, just grayscale)
    unsigned int row = 0;
    unsigned int col = 0;
    for (unsigned int buffer_loc = 0; buffer_loc < image_pixel_size; ++buffer_loc) {
        pixels[row][col] = glyph.bitmap.buffer[buffer_loc];
        ++col;
        if (col >= glyph.bitmap.width) {
            ++row;
            col = 0;
        }
    }
    return pixels;
}
void Font::init_simple(const string& filename, int height, int width) {
    string rawname         = filename;
    const size_t lastindex = filename.find_last_of(".");
    if (lastindex != string::npos) {
        rawname = filename.substr(0, lastindex);
        rawname += ".png";
    }
    m_FontTexture = NEW Texture(rawname, false, ImageInternalFormat::SRGB8_ALPHA8);
    Handle handle = priv::Core::m_Engine->m_ResourceManager._addTexture(m_FontTexture);

    float min_y_offset = 9999999999999.0f;
    float max_y_offset = 0.0f;

    const float textureHeight = static_cast<float>(m_FontTexture->height());
    const float textureWidth  = static_cast<float>(m_FontTexture->width());

    boost::iostreams::stream<boost::iostreams::mapped_file_source> str(filename);
    for (string line; getline(str, line, '\n');) {
        if (line[0] == 'c' && line[1] == 'h' && line[2] == 'a' && line[3] == 'r' && line[4] == ' ') {
            auto charGlyph = CharGlyph();
            string token   = "";
            istringstream stream(line);
            while (getline(stream, token, ' ')) {
                const size_t pos   = token.find("=");
                const string key   = token.substr(0, pos);
                const string value = token.substr(pos + 1, string::npos);

                if (key == "id") {
                    charGlyph.char_id = stoi(value);
                }else if (key == "x") {
                    charGlyph.x = stoi(value);
                }else if (key == "y") {
                    charGlyph.y = stoi(value);
                }else if (key == "width") {
                    charGlyph.width = stoi(value);
                }else if (key == "height") {
                    charGlyph.height = stoi(value);
                }else if (key == "xoffset") {
                    charGlyph.xoffset = stoi(value);
                }else if (key == "yoffset") {
                    charGlyph.yoffset = stoi(value);
                }else if (key == "xadvance") {
                    charGlyph.xadvance = stoi(value);
                }
            }
            if (charGlyph.yoffset + charGlyph.height > max_y_offset) {
                max_y_offset = float(charGlyph.yoffset) + float(charGlyph.height);
            }
            if (charGlyph.yoffset < min_y_offset) {
                min_y_offset = float(charGlyph.yoffset);
            }
            charGlyph.pts.emplace_back(0.0f, 0.0f, 0.0f);
            charGlyph.pts.emplace_back(charGlyph.width, charGlyph.height, 0.0f);
            charGlyph.pts.emplace_back(0.0f, charGlyph.height, 0.0f);
            charGlyph.pts.emplace_back(charGlyph.width, 0.0f, 0.0f);

            const float uvW1 = static_cast<float>(charGlyph.x) / textureWidth;
            const float uvW2 = uvW1 + (static_cast<float>(charGlyph.width) / textureWidth);
            const float uvH1 = static_cast<float>(charGlyph.y) / textureHeight;
            const float uvH2 = uvH1 + (static_cast<float>(charGlyph.height) / textureHeight);

            charGlyph.uvs.emplace_back(uvW1, uvH2);
            charGlyph.uvs.emplace_back(uvW2, uvH1);
            charGlyph.uvs.emplace_back(uvW1, uvH1);
            charGlyph.uvs.emplace_back(uvW2, uvH2);

            m_CharGlyphs.emplace(charGlyph.char_id, std::move(charGlyph));
        }
    }
    m_MaxHeight = max_y_offset - min_y_offset;
}

void Font::init_freetype(const string& filename, int height, int width) {
    const unsigned requested_char_count  = 128;

    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << "\n";
        return;
    }
    FT_Face face;
    if (FT_New_Face(ft, filename.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font file: " << filename << "\n";
        return;
    }
    FT_Set_Pixel_Sizes(face, (width < 0) ? 0 : width, height); //Setting the width to 0 lets the face dynamically calculate the width based on the given height.

    unsigned int max_width      = 0;
    unsigned int max_height     = 0;
    float min_y_offset          = 9999999999999.0f;
    float max_y_offset          = 0.0f;
    const auto face_height      = (face->height >> 6);

    for (GLubyte char_id = 0; char_id < requested_char_count; ++char_id) {
        if (FT_Load_Char(face, char_id, FT_LOAD_RENDER)) {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph: " << char_id << "\n";
            continue;
        }
        CharGlyph charGlyph = CharGlyph();
        charGlyph.char_id   = char_id;
        charGlyph.xadvance  = face->glyph->advance.x >> 6;
        charGlyph.width     = face->glyph->bitmap.width;
        charGlyph.height    = face->glyph->bitmap.rows;
        charGlyph.xoffset   = face->glyph->bitmap_left;

        charGlyph.yoffset   = -(face->glyph->bitmap_top) + ((face_height / 2) + 2);

        max_width     = glm::max(max_width, face->glyph->bitmap.width);
        max_height    = glm::max(max_height, face->glyph->bitmap.rows);

        if (charGlyph.yoffset + charGlyph.height > max_y_offset) {
            max_y_offset = float(charGlyph.yoffset) + float(charGlyph.height);
        }
        if (charGlyph.yoffset < min_y_offset) {
            min_y_offset = float(charGlyph.yoffset);
        }
        m_CharGlyphs.emplace(char_id, std::move(charGlyph));
    }
    const unsigned int final_count  = static_cast<unsigned int>(glm::ceil(glm::sqrt(static_cast<float>(requested_char_count))));
    const unsigned int final_width  = final_count * max_width;
    const unsigned int final_height = final_count * max_height;

    sf::Image atlas_image;
    atlas_image.create(final_width, final_height, sf::Color::Transparent);

    int char_id  = 0;
    bool done    = false;
    const float textureHeight = static_cast<float>(final_height);
    const float textureWidth  = static_cast<float>(final_width);
    for (unsigned int i = 0; i < final_count; ++i) {
        for (unsigned int j = 0; j < final_count; ++j) {
            if (FT_Load_Char(face, char_id, FT_LOAD_RENDER)) {
                continue;
            }
            if (!m_CharGlyphs.count(char_id)) {
                done = true;
                break;
            }
            vector<vector<unsigned char>> pixels = generate_bitmap(*face->glyph);
            auto& charGlyph   = m_CharGlyphs.at(char_id);

            const auto startX = i * max_width;
            const auto startY = j * max_height;

            charGlyph.x = startX;
            charGlyph.y = startY;

            unsigned int cx               = 0;
            unsigned int glyph_y_size     = unsigned int(pixels.size());
            for (unsigned int y = startY; y < startY + glyph_y_size; ++y) {
                unsigned int cy           = 0;
                unsigned int glyph_x_size = unsigned int(pixels[cx].size());
                unsigned int glyph_xEnd   = (startX + glyph_x_size);
                unsigned int glyph_yEnd   = (startY + glyph_y_size);
                for (unsigned int x = startX; x < glyph_xEnd; ++x) {
                    const auto gray = pixels[cx][cy];
                    atlas_image.setPixel(x, y, sf::Color(gray, gray, gray, gray));
                    ++cy;
                }
                ++cx;
            }
            charGlyph.pts.emplace_back(0.0f,            0.0f,             0.0f);
            charGlyph.pts.emplace_back(charGlyph.width, charGlyph.height, 0.0f);
            charGlyph.pts.emplace_back(0.0f,            charGlyph.height, 0.0f);
            charGlyph.pts.emplace_back(charGlyph.width, 0.0f,             0.0f);

            const float uvW1 =         static_cast<float>(charGlyph.x)      / textureWidth;
            const float uvW2 = uvW1 + (static_cast<float>(charGlyph.width)  / textureWidth);
            const float uvH1 =         static_cast<float>(charGlyph.y)      / textureHeight;
            const float uvH2 = uvH1 + (static_cast<float>(charGlyph.height) / textureHeight);

            charGlyph.uvs.emplace_back(uvW1, uvH2);
            charGlyph.uvs.emplace_back(uvW2, uvH1);
            charGlyph.uvs.emplace_back(uvW1, uvH1);
            charGlyph.uvs.emplace_back(uvW2, uvH2);

            ++char_id;
        }
        if (done) {
            break;
        }
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    m_FontTexture = NEW Texture(atlas_image, filename + "_Texture", false, ImageInternalFormat::SRGB8_ALPHA8);
    Handle handle = priv::Core::m_Engine->m_ResourceManager._addTexture(m_FontTexture);

    m_MaxHeight = max_y_offset - min_y_offset;
}
float Font::getTextHeight(string_view text) const {
    if (text.empty()) {
        return 0.0f;
    }
    unsigned int line_count = 0;
    for (const char character : text) {
        if (character == '\n') {
            ++line_count;
        }
    }
    return (line_count == 0) ? (m_MaxHeight) : ((line_count + 1) * (m_MaxHeight + m_LineHeight));
}
float Font::getTextHeightDynamic(string_view text) const {
    if (text.empty()) {
        return 0.0f;
    }
    float res = 0.0f;
    int min_y = INT_MAX;
    int max_y = INT_MIN;
    for (const char character : text) {
        if (character == '\n') {
            res += ((max_y - min_y) ) + m_LineHeight;
        }else {
            const auto& glyph = getGlyphData(character);
            min_y = glm::min(min_y, glyph.yoffset);
            max_y = glm::max(max_y, (glyph.yoffset) + int(glyph.height));
        }
    }
    res += ((max_y - min_y) );
    return (min_y == INT_MAX || max_y == INT_MIN) ? m_MaxHeight : res;
}
float Font::getTextWidth(string_view text) const {
    float row_width = 0.0f;
    float maxWidth  = 0.0f;
    for (size_t i = 0; i < text.size(); ++i) {
        const char character = text[i];
        if (character != '\0' && character != '\r') {
            if (character != '\n') {
                const CharGlyph& glyph = getGlyphData(character);
                row_width += float(glyph.xadvance);
            }else{
                //backtrack spaces
                int j = static_cast<int>(i) - 1;
                while (j >= 0) {
                    const char character_backtrack = text[j];
                    if (character_backtrack != ' ') {
                        break;
                    }
                    const CharGlyph& glyph_space = getGlyphData(character_backtrack);
                    row_width -= float(glyph_space.xadvance);
                    --j;
                }
                maxWidth = max(maxWidth, row_width);
                row_width = 0.0f;
            }
        }
    }
    maxWidth = max(maxWidth, row_width);
    return maxWidth;
}
float Font::getMaxHeight() const {
    return m_MaxHeight;
}
Texture* Font::getGlyphTexture() const {
    return m_FontTexture; 
}
float Font::getLineHeight() const {
    return m_LineHeight;
}
const CharGlyph& Font::getGlyphData(const unsigned char character) const {
    return (m_CharGlyphs.count(character)) ? m_CharGlyphs.at(character) : m_CharGlyphs.at('?');
}
void Font::renderText(const string& t, const glm::vec2& p, const glm::vec4& c, const float a, const glm::vec2& s, const float d, const TextAlignment::Type al, const glm::vec4& scissor){
    Renderer::renderText(t, *this, p, c, a, s, d, al, scissor);
}
void Font::renderTextStatic(const string& t, const glm::vec2& p, const glm::vec4& c, const float a, const glm::vec2& s, const float d, const TextAlignment::Type al, const glm::vec4& scissor) {
    if (first_font) {
        Renderer::renderText(t, *first_font, p, c, a, s, d, al, scissor);
    }
}
