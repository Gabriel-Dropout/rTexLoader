#ifndef RTEXLOADER_H
#define RTEXLOADER_H

#include <map>
#include <string>

#include "raylib.h"

typedef struct AtlasSprite {
    char nameId[128];           // Sprite original filename (without extension)
    int originX, originY;       // Sprite origin (pivot point), useful in some cases
    int positionX, positionY;   // Sprite position in the atlas
    int sourceWidth;            // Sprite source width (before trim)
    int sourceHeight;           // Sprite source height (before trim)
    int padding;                // Sprite padding, must be added to source size
    int trimmed;                // Sprite is trimmed (removed blank space for better packing)
    int trimX, trimY, trimWidth, trimHeight; // Sprite trim rectangle 
    
    // Glyph info, in case sprite is a font character
    // NOTE: This data could probably be ommited 
    int value;                  // Character value (Unicode)
    int offsetX, offsetY;       // Character offset when drawing
    int advanceX;               // Character advance position X
} AtlasSprite;

// Global variables
extern Texture atlas;
extern std::map<std::string, AtlasSprite> spriteMap;

// Load/Unload atlas and sprite map from file
void initAtlas(const char* filename);
void initAtlas(const std::string& filename);
void unloadAtlas();

// Get a sprite from the map by nameId
inline AtlasSprite& getAtlasSprite(const std::string& nameId)                   {return spriteMap[nameId];}
inline AtlasSprite& getAtlasSprite(const std::string& nameId, int suffix)       {return spriteMap[nameId + std::to_string(suffix)];}
inline AtlasSprite& getAtlasSprite(const std::string_view& nameId)              {return spriteMap[std::string(nameId)];}
inline AtlasSprite& getAtlasSprite(const std::string_view& nameId, int suffix)  {return spriteMap[std::string(nameId) + std::to_string(suffix)];}
inline AtlasSprite& getAtlasSprite(const char* nameId)                          {return spriteMap[std::string(nameId)];}
inline AtlasSprite& getAtlasSprite(const char* nameId, int suffix)              {return spriteMap[std::string(nameId) + std::to_string(suffix)];}

// Aliases for getAtlasSprite
inline AtlasSprite& getSprite(const std::string& nameId)                   {return getAtlasSprite(nameId);}
inline AtlasSprite& getSprite(const std::string& nameId, int suffix)       {return getAtlasSprite(nameId, suffix);}
inline AtlasSprite& getSprite(const std::string_view& nameId)              {return getAtlasSprite(nameId);}
inline AtlasSprite& getSprite(const std::string_view& nameId, int suffix)  {return getAtlasSprite(nameId, suffix);}
inline AtlasSprite& getSprite(const char* nameId)                          {return getAtlasSprite(nameId);}
inline AtlasSprite& getSprite(const char* nameId, int suffix)              {return getAtlasSprite(nameId, suffix);}

// Draw a sprite from atlas
inline void DrawSpriteAtlas(const Texture2D &atlas, const AtlasSprite &sprite, int x, int y, Color color) {
    DrawTexturePro(atlas,
        Rectangle{(float)sprite.positionX, (float)sprite.positionY, (float)sprite.sourceWidth, (float)sprite.sourceHeight},
        Rectangle{(float)x, (float)y, (float)sprite.sourceWidth, (float)sprite.sourceHeight},
        Vector2{(float)sprite.originX, (float)sprite.originY},
        0.0f,
        color);
}
inline void DrawSpriteAtlas(const Texture2D &atlas, const AtlasSprite &sprite, int x, int y, float angle, Color color) {
    DrawTexturePro(atlas,
        Rectangle{(float)sprite.positionX, (float)sprite.positionY, (float)sprite.sourceWidth, (float)sprite.sourceHeight},
        Rectangle{(float)x, (float)y, (float)sprite.sourceWidth, (float)sprite.sourceHeight},
        Vector2{(float)sprite.originX, (float)sprite.originY},
        angle,
        color);
}
inline void DrawSpriteAtlas(const Texture2D &atlas, const AtlasSprite &sprite, int x, int y, float xscale, float yscale, float angle, Color color) {
    if(xscale >= 0)
        DrawTexturePro(atlas,
            Rectangle{(float)sprite.positionX, (float)sprite.positionY, (float)sprite.sourceWidth, (float)sprite.sourceHeight},
            Rectangle{(float)x, (float)y, (float)sprite.sourceWidth*xscale, (float)sprite.sourceHeight*yscale},
            Vector2{(float)sprite.originX*xscale, (float)sprite.originY*yscale},
            angle,
            color);
    else
        DrawTexturePro(atlas,
            Rectangle{(float)sprite.positionX, (float)sprite.positionY, -(float)sprite.sourceWidth, (float)sprite.sourceHeight},
            Rectangle{(float)x, (float)y, -(float)sprite.sourceWidth*xscale, (float)sprite.sourceHeight*yscale},
            Vector2{(float)sprite.originX*(-xscale), (float)sprite.originY*yscale},
            angle,
            color);
}
// warning: it ignores origin of sprite
inline void DrawSpriteAtlas(const Texture2D &atlas, const AtlasSprite &sprite, Rectangle slice, int x, int y, float xscale, float yscale, float angle, Color color) {
    DrawTexturePro(atlas,
        Rectangle{(float)sprite.positionX + slice.x, (float)sprite.positionY + slice.y, slice.width, slice.height},
        Rectangle{(float)x, (float)y, slice.width*xscale, slice.height*yscale},
        Vector2{0, 0},
        angle,
        color);
}

#endif  // RTEXLOADER_H

#ifdef RTEXLOADER_IMPLEMENTATION

#include <iostream>
#include <vector>
#include <map>
#include <array>
#include <stdint.h>
#include <string_view>
#include <algorithm>

#include "raylib.h"

#include <fstream>

constexpr std::array<unsigned char, 8> pngSignature = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
constexpr std::array<unsigned char, 4> rTPbChunkType = {'r', 'T', 'P', 'b'};

static std::vector<unsigned char> readChunkFromPNG(const char *filename, const std::array<unsigned char, 4> chunk_type);
static std::map<std::string, AtlasSprite> loadSpriteMapFromRTPB(const std::vector<unsigned char> rtpbData);

static unsigned int readIntFromBytes(std::vector<unsigned char>::const_iterator it, bool big_endian = false);
static unsigned int readShortFromBytes(std::vector<unsigned char>::const_iterator it, bool big_endian = false);

Texture atlas;
std::map<std::string, AtlasSprite> spriteMap;

void initAtlas(const char* filename) {
    atlas = LoadTexture(filename);
    
    // Read custom PNG chunk: rTPb
    std::vector<unsigned char> chunk = readChunkFromPNG(filename, rTPbChunkType);
    if(chunk.empty()) {
        std::cout << "Error loading chunk data" << std::endl;
        exit(1);
    }

    // Load sprite data from .rtpb file data
    spriteMap = loadSpriteMapFromRTPB(chunk);  // global variable
    if(spriteMap.empty()) {
        std::cout << "Error loading sprite informaion" << std::endl;
        exit(1);
    }
    
    std::cout << "Sprite Count: " << spriteMap.size() << std::endl;
}
void initAtlas(const std::string& filename) {
    initAtlas(filename.c_str());
}
void unloadAtlas() {
    UnloadTexture(atlas);
}


// Load sprite data from .rtpb file data
static std::map<std::string, AtlasSprite> loadSpriteMapFromRTPB(const std::vector<unsigned char> rtpbData) {
    std::map<std::string, AtlasSprite> spriteMap;
    std::vector<unsigned char>::const_iterator it = rtpbData.begin();
    
    // Check signature
    if(std::equal(it, it + 4, rTPbChunkType.begin())) {  // Valid rTPb file
        it += 4;  // Move to next value
        
        int version = readShortFromBytes(it);  // rTPb version
        
        if (version == 200) {  // This is the rTPb version we will read
            it += 2*sizeof(short);  // Skip the version and reserved values
    
            int count = readIntFromBytes(it);  // Number of sprites packed in the atlas
            it += sizeof(int);      // Skip sprites packed
            it += sizeof(int);      // Skip flags (0 by default, no image included)
            
            // Read font info, it could be useful
            short fontType = readShortFromBytes(it);            // Font type: 0-No font, 1-Normal, 2-SDF
            short fontSize = readShortFromBytes(it + 2);        // Font size
            short fontSdfPadding = readShortFromBytes(it + 4);  // Font SDF padding
            
            it += 4*sizeof(short);     // Skip to sprites data

            AtlasSprite tmpSprite = {0};
            
            // Read and copy sprites data from rTPb data
            for (int i = 0; i < count; i++) {
                memcpy(tmpSprite.nameId, &*it, 128);            // Sprite NameId (128 bytes by default)
                it += 128;

                memcpy(&tmpSprite.originX, &*it, 4);            // Sprite Origin X
                memcpy(&tmpSprite.originY, &*it + 4, 4);        // Sprite Origin Y
                memcpy(&tmpSprite.positionX, &*it + 8, 4);      // Sprite Position X
                memcpy(&tmpSprite.positionY, &*it + 12, 4);     // Sprite Position Y
                memcpy(&tmpSprite.sourceWidth, &*it + 16, 4);   // Sprite Source Width
                memcpy(&tmpSprite.sourceHeight, &*it + 20, 4);  // Sprite Source Height
                memcpy(&tmpSprite.padding, &*it + 24, 4);       // Sprite Padding
                memcpy(&tmpSprite.trimmed, &*it + 28, 4);       // Sprite is trimmed?
                memcpy(&tmpSprite.trimX, &*it + 32, 4);         // Sprite Trimmed Rectangle X
                memcpy(&tmpSprite.trimY, &*it + 36, 4);         // Sprite Trimmed Rectangle Y
                memcpy(&tmpSprite.trimWidth, &*it + 40, 4);     // Sprite Trimmed Rectangle Width
                memcpy(&tmpSprite.trimHeight, &*it + 44, 4);    // Sprite Trimmed Rectangle Height
                it += 48;
                
                if (fontType > 0) {
                    memcpy(&tmpSprite.value, &*it, 4);          // Character value (Unicode)
                    memcpy(&tmpSprite.offsetX, &*it + 4, 4);    // Character offset X when drawing
                    memcpy(&tmpSprite.offsetY, &*it + 8, 4);    // Character offset Y when drawing
                    memcpy(&tmpSprite.advanceX, &*it + 12, 4);  // Character advance position X
                    it += 16;
                }

                spriteMap.insert(std::make_pair(std::string(tmpSprite.nameId), tmpSprite));
            }
        }
    }
    
    return spriteMap;
}

// Read one chunk from a PNG file
// NOTE: There could be multiple chunks of same type, only first found is returned
static std::vector<unsigned char> readChunkFromPNG(const char *filename, const std::array<unsigned char, 4> chunk_type) {

    std::ifstream input(filename, std::ios::binary);
    if(!input) {
        std::cout << "Error loading file: " << filename << std::endl;
        exit(1);
    }
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});
    input.close();
    if(buffer.empty()) {
        std::cout << "The buffer is empty." << std::endl;
        exit(1);
    }

    std::vector<unsigned char>::const_iterator it = buffer.begin();

    if(std::equal(it, it + 8, pngSignature.begin())) {  // Check valid PNG file
        it += 8;  // Move pointer after signature

        while (it != buffer.end()) {  // While IEND chunk not reached
            unsigned int chunk_size = readIntFromBytes(it, true);
            if (std::equal(it+4, it + 8, chunk_type.begin()))
                return std::vector<unsigned char>(it + 8, it + 8 + chunk_size);

            it += (4 + 4 + chunk_size + 4);  // Move pointer to next chunk of input data
        }
    }

    return std::vector<unsigned char>();
}

static unsigned int readIntFromBytes(std::vector<unsigned char>::const_iterator it, bool big_endian) {
    unsigned int result = 0;
    if(big_endian) {
        result = (result<<8) + it[0];
        result = (result<<8) + it[1];
        result = (result<<8) + it[2];
        result = (result<<8) + it[3];
    } else {
        result = (result<<8) + it[3];
        result = (result<<8) + it[2];
        result = (result<<8) + it[1];
        result = (result<<8) + it[0];
    }
    return result;
}
static unsigned int readShortFromBytes(std::vector<unsigned char>::const_iterator it, bool big_endian) {
    unsigned int result = 0;
    if(big_endian) {
        result = (result<<8) + it[0];
        result = (result<<8) + it[1];
    } else {
        result = (result<<8) + it[1];
        result = (result<<8) + it[0];
    }
    return result;
}

#endif  // RTEXLOADER_IMPLEMENTATION