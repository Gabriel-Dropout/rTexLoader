# rTexLoader-cpp
`rTexPacker`에서 생성된 아틀라스 데이터를 동적으로 불러오는 C++ 단일 헤더 라이브러리입니다. 사용하기 위해 `raylib`이 필요합니다.

---

## 기능

- 지정된 경로에서 아틀라스 이미지를 로딩하고 텍스쳐를 `atlas` 전역변수에 저장
- 아틀라스 png파일에 포함된 `rTPb` 청크를 읽고 `spriteMap` 생성
- 스프라이트 드로우

---

## 전역 변수 및 기본 함수

```C++
// Global variables
extern Texture atlas;
extern std::map<std::string, AtlasSprite> spriteMap;

// Load/Unload atlas and sprite map from file
void initAtlas(const char* filename);
void initAtlas(const std::string& filename);
void unloadAtlas();

// Get a sprite from the map by nameId
AtlasSprite& getAtlasSprite(const std::string& nameId);
AtlasSprite& getAtlasSprite(const std::string& nameId, int suffix);
AtlasSprite& getAtlasSprite(const std::string_view& nameId);
AtlasSprite& getAtlasSprite(const std::string_view& nameId, int suffix);
AtlasSprite& getAtlasSprite(const char* nameId);
AtlasSprite& getAtlasSprite(const char* nameId, int suffix);

// Aliases for getAtlasSprite
AtlasSprite& getSprite(const std::string& nameId);
AtlasSprite& getSprite(const std::string& nameId, int suffix);
AtlasSprite& getSprite(const std::string_view& nameId);
AtlasSprite& getSprite(const std::string_view& nameId, int suffix);
AtlasSprite& getSprite(const char* nameId);
AtlasSprite& getSprite(const char* nameId, int suffix);

// Draw a sprite from atlas
void DrawSpriteAtlas(const Texture2D &atlas, const AtlasSprite &sprite, int x, int y, Color color);
void DrawSpriteAtlas(const Texture2D &atlas, const AtlasSprite &sprite, int x, int y, float angle, Color color);
void DrawSpriteAtlas(const Texture2D &atlas, const AtlasSprite &sprite, int x, int y, float xscale, float yscale, float angle, Color color);
// warning: it ignores origin of sprite
void DrawSpriteAtlas(const Texture2D &atlas, const AtlasSprite &sprite, Rectangle slice, int x, int y, float xscale, float yscale, float angle, Color color);
```

---

## 사용 예시

```C++
#include "raylib.h"
#define RTEXLOADER_IMPLEMENTATION
#include "rtexloader.hpp"

const int screenWidth = 640;
const int screenHeight = 480;

int main() {
    /// Initialization ///
    InitWindow(screenWidth, screenHeight, "atlas loading test");
    SetTargetFPS(60);
    initAtlas("atlas.png");
    
    /// Main game loop ///
    while (!WindowShouldClose()) {
        /// Update ///

        /// Draw ///
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawSpriteAtlas(atlas, getSprite("image_name_without_extension"), 320, 240, WHITE);
        EndDrawing();
    }
    unloadAtlas();
}
```

---

## 예제 빌드

`CMake`를 이용해 원하는 빌드 도구를 활용할 수 있습니다. 예를 들어 `MinGW Makefiles`의 경우 다음과 같습니다.

```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
make
cd example && ./main.exe
```

---

## Why RTEXLOADER_IMPLEMENTATION?

이는 라이브러리 디자인의 일부입니다. `rTexLoader`는 단일 헤더 라이브러리이므로 전체 구현이 헤더파일 내에 있어야 합니다. 모든 함수를 인라이닝하는 대신, 일부 함수를 `extern`으로 선언하고 구현을 `#ifdef RTEXLOADER_IMPLEMENTATION` 아래에 위치했습니다. 

`rTexLoader`를 사용하는 변환 단위 중 **오직 한 개**의 파일에서 이 매크로를 정의하면, 해당 파일에서 이 함수들의 정의가 삽입됩니다.

---

## 그 외

현재 이 라이브러리는 단일 아틀라스만을 지원합니다. 다중 아틀라스 구현을 원하신다면 자유롭게 PR을 보내주세요 :)

`example` 코드에서 사용된 리소스 출처는 `https://0x72.itch.io/dungeontileset-ii`입니다.
