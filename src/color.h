#ifndef COLOR_H
#define COLOR_H

#include <cstdint>

namespace fallout {

typedef unsigned char Color;
typedef const char*(ColorFileNameManger)(const char*);
typedef void(ColorTransitionCallback)();

extern unsigned char _cmap[768];

extern unsigned char _systemCmap[256 * 3];
extern unsigned char _currentGammaTable[64];
extern unsigned char* _blendTable[256];
extern unsigned char _mappedColor[256];
extern Color colorMixAddTable[256][256];
extern Color intensityColorTable[256][256];
extern Color colorMixMulTable[256][256];
extern unsigned char _colorTable[32768];

int _calculateColor(int intensity, Color color);
int Color2RGB(Color c);
void colorPaletteFadeBetween(unsigned char* oldPalette, unsigned char* newPalette, int steps);
void colorPaletteSetTransitionCallback(ColorTransitionCallback* callback);
void _setSystemPalette(unsigned char* palette);
unsigned char* _getSystemPalette();
void _setSystemPaletteEntries(unsigned char* palette, int start, int end);
bool colorPaletteLoad(const char* path);
char* _colorError();
unsigned char* _getColorBlendTable(int ch);
void _freeColorBlendTable(int color);
void colorSetBrightness(double value);
bool _initColors();
void _colorsClose();

// Colour helpers - convert 24-bit hex (#RRGGBB) to/from 15-bit keys.

// Convert 24-bit RGB (e.g., 0xFCFCFC) to the 15-bit key used by _colorTable[].
inline constexpr uint16_t ColorRGB15(uint32_t rgb24)
{
    return (uint16_t)((((rgb24 >> 16) & 0xFF) >> 3) << 10 | (((rgb24 >> 8) & 0xFF) >> 3) << 5 | ((rgb24 & 0xFF) >> 3));
}

// ============================================================================
// Full fallout.act palette colour constants - one per palette index.
// Replaces all magic numbers from original code. Some original magic numbers
// Did not match colors in palette - likely a palette change at some point
// ============================================================================

inline constexpr uint16_t COL_BRIGHT_BLUE = ColorRGB15(0x0000FF); // idx   0
inline constexpr uint16_t COL_VERY_LIGHT_GRAY = ColorRGB15(0xECECEC); // idx   1
inline constexpr uint16_t COL_LIGHT_GRAY = ColorRGB15(0xDCDCDC); // idx   2
inline constexpr uint16_t COL_SILVER = ColorRGB15(0xCCCCCC); // idx   3
inline constexpr uint16_t COL_MEDIUM_LIGHT_GRAY = ColorRGB15(0xBCBCBC); // idx   4
inline constexpr uint16_t COL_MEDIUM_GRAY = ColorRGB15(0xB0B0B0); // idx   5
inline constexpr uint16_t COL_GUNMETAL = ColorRGB15(0xA0A0A0); // idx   6
inline constexpr uint16_t COL_DARK_MEDIUM_GRAY = ColorRGB15(0x909090); // idx   7
inline constexpr uint16_t COL_DARK_GRAY = ColorRGB15(0x808080); // idx   8
inline constexpr uint16_t COL_CHARCOAL = ColorRGB15(0x747474); // idx   9
inline constexpr uint16_t COL_DARK_CHARCOAL = ColorRGB15(0x646464); // idx  10
inline constexpr uint16_t COL_VERY_DARK_GRAY = ColorRGB15(0x545454); // idx  11
inline constexpr uint16_t COL_DARK_SLATE = ColorRGB15(0x444444); // idx  12
inline constexpr uint16_t COL_ANTHRACITE = ColorRGB15(0x383838); // idx  13
inline constexpr uint16_t COL_NEAR_BLACK = ColorRGB15(0x282828); // idx  14
inline constexpr uint16_t COL_ALMOST_BLACK = ColorRGB15(0x202020); // idx  15

inline constexpr uint16_t COL_VERY_PALE_PINK = ColorRGB15(0xFCECEC); // idx  16
inline constexpr uint16_t COL_PALE_PINK = ColorRGB15(0xECD8D8); // idx  17
inline constexpr uint16_t COL_LIGHT_DUSTY_ROSE = ColorRGB15(0xDCC4C4); // idx  18
inline constexpr uint16_t COL_DUSTY_ROSE = ColorRGB15(0xD0B0B0); // idx  19
inline constexpr uint16_t COL_MUTED_ROSE = ColorRGB15(0xC0A0A0); // idx  20
inline constexpr uint16_t COL_OLD_ROSE = ColorRGB15(0xB09090); // idx  21
inline constexpr uint16_t COL_ROSEWOOD = ColorRGB15(0xA48080); // idx  22
inline constexpr uint16_t COL_DUSTY_MAUVE = ColorRGB15(0x947070); // idx  23
inline constexpr uint16_t COL_DESERT_ROSE = ColorRGB15(0x846060); // idx  24
inline constexpr uint16_t COL_WARM_BRICK = ColorRGB15(0x785454); // idx  25
inline constexpr uint16_t COL_BRICK_RED = ColorRGB15(0x684444); // idx  26
inline constexpr uint16_t COL_DARK_BRICK = ColorRGB15(0x583838); // idx  27
inline constexpr uint16_t COL_DEEP_MAROON = ColorRGB15(0x4C2C2C); // idx  28
inline constexpr uint16_t COL_VERY_DARK_MAROON = ColorRGB15(0x3C2424); // idx  29
inline constexpr uint16_t COL_NEARLY_BLACK_BROWN = ColorRGB15(0x2C1818); // idx  30
inline constexpr uint16_t COL_BLACKISH_RED = ColorRGB15(0x201010); // idx  31

inline constexpr uint16_t COL_VERY_PALE_LAVENDER = ColorRGB15(0xECECFC); // idx  32
inline constexpr uint16_t COL_PALE_LAVENDER = ColorRGB15(0xD8D8EC); // idx  33
inline constexpr uint16_t COL_LIGHT_PERIWINKLE = ColorRGB15(0xC4C4DC); // idx  34
inline constexpr uint16_t COL_PERIWINKLE = ColorRGB15(0xB0B0D0); // idx  35
inline constexpr uint16_t COL_MUTED_PERIWINKLE = ColorRGB15(0xA0A0C0); // idx  36
inline constexpr uint16_t COL_CADET_GRAY = ColorRGB15(0x9090B0); // idx  37
inline constexpr uint16_t COL_SLATE_BLUE = ColorRGB15(0x8080A4); // idx  38
inline constexpr uint16_t COL_DARK_SLATE_BLUE = ColorRGB15(0x707094); // idx  39
inline constexpr uint16_t COL_DEEP_SLATE = ColorRGB15(0x606084); // idx  40
inline constexpr uint16_t COL_DARK_CADET = ColorRGB15(0x545478); // idx  41
inline constexpr uint16_t COL_BLUE_GRAY = ColorRGB15(0x444468); // idx  42
inline constexpr uint16_t COL_DARK_BLUE_GRAY = ColorRGB15(0x383858); // idx  43
inline constexpr uint16_t COL_VERY_DARK_BLUE = ColorRGB15(0x2C2C4C); // idx  44
inline constexpr uint16_t COL_MIDNIGHT_BLUE = ColorRGB15(0x24243C); // idx  45
inline constexpr uint16_t COL_NEAR_BLACK_BLUE = ColorRGB15(0x18182C); // idx  46
inline constexpr uint16_t COL_BLACKISH_BLUE = ColorRGB15(0x101020); // idx  47

inline constexpr uint16_t COL_ORCHID_PINK = ColorRGB15(0xFCB0F0); // idx  48
inline constexpr uint16_t COL_MAUVE = ColorRGB15(0xC460A8); // idx  49
inline constexpr uint16_t COL_DEEP_MAUVE = ColorRGB15(0x682460); // idx  50
inline constexpr uint16_t COL_DARK_PLUM = ColorRGB15(0x4C1448); // idx  51
inline constexpr uint16_t COL_AUBERGINE = ColorRGB15(0x380C34); // idx  52
inline constexpr uint16_t COL_VERY_DARK_PLUM = ColorRGB15(0x281024); // idx  53
inline constexpr uint16_t COL_BLACKISH_PLUM = ColorRGB15(0x240424); // idx  54
inline constexpr uint16_t COL_NEAR_BLACK_PURPLE = ColorRGB15(0x1C0C18); // idx  55
inline constexpr uint16_t COL_VERY_PALE_YELLOW = ColorRGB15(0xFCFCC8); // idx  56
inline constexpr uint16_t COL_LIGHT_LEMON = ColorRGB15(0xFCFC7C); // idx  57
inline constexpr uint16_t COL_GOLDEN_YELLOW = ColorRGB15(0xE4D80C); // idx  58
inline constexpr uint16_t COL_MUSTARD = ColorRGB15(0xCCB81C); // idx  59
inline constexpr uint16_t COL_OLIVE_YELLOW = ColorRGB15(0xB89C28); // idx  60
inline constexpr uint16_t COL_DARK_MUSTARD = ColorRGB15(0xA48830); // idx  61
inline constexpr uint16_t COL_GREENISH_BROWN = ColorRGB15(0x907824); // idx  62
inline constexpr uint16_t COL_DARK_OLIVE = ColorRGB15(0x7C6818); // idx  63

inline constexpr uint16_t COL_OLIVE_BROWN = ColorRGB15(0x6C5810); // idx  64
inline constexpr uint16_t COL_DARK_OLIVE_BROWN = ColorRGB15(0x584808); // idx  65
inline constexpr uint16_t COL_VERY_DARK_OLIVE = ColorRGB15(0x483804); // idx  66
inline constexpr uint16_t COL_BLACKISH_OLIVE = ColorRGB15(0x342800); // idx  67
inline constexpr uint16_t COL_NEAR_BLACK_BROWN_2 = ColorRGB15(0x201800); // idx  68
inline constexpr uint16_t COL_LIGHT_SPRING_GREEN = ColorRGB15(0xD8FC9C); // idx  69
inline constexpr uint16_t COL_PALE_GREEN = ColorRGB15(0xB4D884); // idx  70
inline constexpr uint16_t COL_SAGE_GREEN = ColorRGB15(0x98B870); // idx  71
inline constexpr uint16_t COL_MUTED_GREEN = ColorRGB15(0x78985C); // idx  72
inline constexpr uint16_t COL_FERN_GREEN = ColorRGB15(0x5C7848); // idx  73
inline constexpr uint16_t COL_DARK_FERN = ColorRGB15(0x405834); // idx  74
inline constexpr uint16_t COL_VERY_DARK_GREEN_2 = ColorRGB15(0x283820); // idx  75
inline constexpr uint16_t COL_WARM_GRAY_BROWN = ColorRGB15(0x706050); // idx  76
inline constexpr uint16_t COL_DARK_BEIGE = ColorRGB15(0x544834); // idx  77
inline constexpr uint16_t COL_COCOA_BROWN = ColorRGB15(0x383020); // idx  78
inline constexpr uint16_t COL_OLIVE_GREEN = ColorRGB15(0x687850); // idx  79

inline constexpr uint16_t COL_MOSS = ColorRGB15(0x707820); // idx  80
inline constexpr uint16_t COL_DRAB_GREEN = ColorRGB15(0x706828); // idx  81
inline constexpr uint16_t COL_DARK_MOSS = ColorRGB15(0x606024); // idx  82
inline constexpr uint16_t COL_OLIVE_DRAB = ColorRGB15(0x4C4424); // idx  83
inline constexpr uint16_t COL_COCOA_BROWN_2 = ColorRGB15(0x383020); // idx  84  // duplicate of idx 78
inline constexpr uint16_t COL_PALE_SAGE = ColorRGB15(0x9CAC9C); // idx  85
inline constexpr uint16_t COL_GRAYISH_GREEN = ColorRGB15(0x789478); // idx  86
inline constexpr uint16_t COL_FOREST_GREEN = ColorRGB15(0x587C58); // idx  87
inline constexpr uint16_t COL_DARK_FOREST_GREEN = ColorRGB15(0x406840); // idx  88
inline constexpr uint16_t COL_TEAL_GRAY = ColorRGB15(0x385858); // idx  89
inline constexpr uint16_t COL_DARK_TEAL = ColorRGB15(0x304C48); // idx  90
inline constexpr uint16_t COL_DEEP_TEAL = ColorRGB15(0x28443C); // idx  91
inline constexpr uint16_t COL_DARK_GREEN_TEAL = ColorRGB15(0x203C2C); // idx  92
inline constexpr uint16_t COL_VERY_DARK_TEAL = ColorRGB15(0x1C3024); // idx  93
inline constexpr uint16_t COL_BLACKISH_TEAL = ColorRGB15(0x142818); // idx  94
inline constexpr uint16_t COL_NEAR_BLACK_GREEN = ColorRGB15(0x102010); // idx  95

inline constexpr uint16_t COL_VERY_DARK_FOREST = ColorRGB15(0x183018); // idx  96
inline constexpr uint16_t COL_BLACKISH_GREEN = ColorRGB15(0x10240C); // idx  97
inline constexpr uint16_t COL_ALMOST_BLACK_GREEN = ColorRGB15(0x081C04); // idx  98
inline constexpr uint16_t COL_NEAR_BLACK_2 = ColorRGB15(0x041400); // idx  99
inline constexpr uint16_t COL_BLACKISH_2 = ColorRGB15(0x040C00); // idx 100
inline constexpr uint16_t COL_COOL_GRAY = ColorRGB15(0x8C9C9C); // idx 101
inline constexpr uint16_t COL_GRAYISH_CYAN = ColorRGB15(0x789498); // idx 102
inline constexpr uint16_t COL_DUSTY_BLUE = ColorRGB15(0x648894); // idx 103
inline constexpr uint16_t COL_STEEL_BLUE = ColorRGB15(0x507C90); // idx 104
inline constexpr uint16_t COL_DARK_STEEL_BLUE = ColorRGB15(0x406C8C); // idx 105
inline constexpr uint16_t COL_BLUE_NAVY = ColorRGB15(0x30588C); // idx 106
inline constexpr uint16_t COL_DARK_NAVY = ColorRGB15(0x2C4C7C); // idx 107
inline constexpr uint16_t COL_NAVY = ColorRGB15(0x28446C); // idx 108
inline constexpr uint16_t COL_DEEP_NAVY = ColorRGB15(0x20385C); // idx 109
inline constexpr uint16_t COL_VERY_DARK_NAVY = ColorRGB15(0x1C304C); // idx 110
inline constexpr uint16_t COL_BLACKISH_NAVY = ColorRGB15(0x182840); // idx 111

inline constexpr uint16_t COL_WARM_GRAY = ColorRGB15(0x9CA4A4); // idx 112
inline constexpr uint16_t COL_DARK_BLUE_SLATE = ColorRGB15(0x384868); // idx 113
inline constexpr uint16_t COL_COOL_DARK_GRAY = ColorRGB15(0x505858); // idx 114
inline constexpr uint16_t COL_SLATE_GRAY = ColorRGB15(0x586884); // idx 115
inline constexpr uint16_t COL_DARK_SLATE_2 = ColorRGB15(0x384050); // idx 116
inline constexpr uint16_t COL_LIGHT_SILVER = ColorRGB15(0xBCBCBC); // idx 117 // duplicate of idx 4
inline constexpr uint16_t COL_STONE = ColorRGB15(0xACA498); // idx 118
inline constexpr uint16_t COL_WARM_STONE = ColorRGB15(0xA0907C); // idx 119
inline constexpr uint16_t COL_SANDSTONE = ColorRGB15(0x947C60); // idx 120
inline constexpr uint16_t COL_WARM_TAUPE = ColorRGB15(0x88684C); // idx 121
inline constexpr uint16_t COL_TAUPE = ColorRGB15(0x7C5834); // idx 122
inline constexpr uint16_t COL_BROWN_TAUPE = ColorRGB15(0x704824); // idx 123
inline constexpr uint16_t COL_DARK_TAUPE = ColorRGB15(0x643C14); // idx 124
inline constexpr uint16_t COL_VERY_DARK_TAUPE = ColorRGB15(0x583008); // idx 125
inline constexpr uint16_t COL_PALE_CORAL = ColorRGB15(0xFCCCCC); // idx 126
inline constexpr uint16_t COL_CORAL = ColorRGB15(0xFCB0B0); // idx 127

inline constexpr uint16_t COL_LIGHT_CORAL = ColorRGB15(0xFC9898); // idx 128
inline constexpr uint16_t COL_SALMON = ColorRGB15(0xFC7C7C); // idx 129
inline constexpr uint16_t COL_WARM_SALMON = ColorRGB15(0xFC6464); // idx 130
inline constexpr uint16_t COL_BRIGHT_RED = ColorRGB15(0xFC4848); // idx 131
inline constexpr uint16_t COL_VIVID_RED = ColorRGB15(0xFC3030); // idx 132
inline constexpr uint16_t COL_PURE_RED = ColorRGB15(0xFC0000); // idx 133
inline constexpr uint16_t COL_RED = ColorRGB15(0xE00000); // idx 134
inline constexpr uint16_t COL_DARK_RED = ColorRGB15(0xC40000); // idx 135
inline constexpr uint16_t COL_DEEP_RED = ColorRGB15(0xA80000); // idx 136
inline constexpr uint16_t COL_RUBY_RED = ColorRGB15(0x900000); // idx 137
inline constexpr uint16_t COL_DARK_RUBY = ColorRGB15(0x740000); // idx 138
inline constexpr uint16_t COL_VERY_DARK_RED = ColorRGB15(0x580000); // idx 139
inline constexpr uint16_t COL_BLACKISH_RED_2 = ColorRGB15(0x400000); // idx 140
inline constexpr uint16_t COL_PALE_PEACH = ColorRGB15(0xFCE0C8); // idx 141
inline constexpr uint16_t COL_PEACH = ColorRGB15(0xFCC494); // idx 142
inline constexpr uint16_t COL_LIGHT_APRICOT = ColorRGB15(0xFCB878); // idx 143

inline constexpr uint16_t COL_APRICOT = ColorRGB15(0xFCAC60); // idx 144
inline constexpr uint16_t COL_ORANGE = ColorRGB15(0xFC9C48); // idx 145
inline constexpr uint16_t COL_VIBRANT_ORANGE = ColorRGB15(0xFC942C); // idx 146
inline constexpr uint16_t COL_BRIGHT_ORANGE = ColorRGB15(0xFC8814); // idx 147
inline constexpr uint16_t COL_PURE_ORANGE = ColorRGB15(0xFC7C00); // idx 148
inline constexpr uint16_t COL_DARK_ORANGE = ColorRGB15(0xDC6C00); // idx 149
inline constexpr uint16_t COL_BURNT_ORANGE = ColorRGB15(0xC06000); // idx 150
inline constexpr uint16_t COL_RUST_ORANGE = ColorRGB15(0xA45000); // idx 151
inline constexpr uint16_t COL_RUST = ColorRGB15(0x844400); // idx 152
inline constexpr uint16_t COL_DARK_RUST = ColorRGB15(0x683400); // idx 153
inline constexpr uint16_t COL_VERY_DARK_RUST = ColorRGB15(0x4C2400); // idx 154
inline constexpr uint16_t COL_BLACKISH_RUST = ColorRGB15(0x301800); // idx 155
inline constexpr uint16_t COL_PALE_SAND = ColorRGB15(0xF8D4A4); // idx 156
inline constexpr uint16_t COL_SAND = ColorRGB15(0xD8B078); // idx 157
inline constexpr uint16_t COL_WARM_SAND = ColorRGB15(0xC8A064); // idx 158
inline constexpr uint16_t COL_DESERT_SAND = ColorRGB15(0xBC9054); // idx 159

inline constexpr uint16_t COL_WARM_SAND_2 = ColorRGB15(0xAC8044); // idx 160
inline constexpr uint16_t COL_BUFF = ColorRGB15(0x9C7434); // idx 161
inline constexpr uint16_t COL_DARK_BUFF = ColorRGB15(0x8C6428); // idx 162
inline constexpr uint16_t COL_BROWN_BUFF = ColorRGB15(0x7C581C); // idx 163
inline constexpr uint16_t COL_DARK_BROWN = ColorRGB15(0x704C14); // idx 164
inline constexpr uint16_t COL_VERY_DARK_BROWN_2 = ColorRGB15(0x604008); // idx 165
inline constexpr uint16_t COL_NEAR_BLACK_BROWN_3 = ColorRGB15(0x503404); // idx 166
inline constexpr uint16_t COL_BLACKISH_BROWN = ColorRGB15(0x402800); // idx 167
inline constexpr uint16_t COL_VERY_DARK_2 = ColorRGB15(0x342000); // idx 168
inline constexpr uint16_t COL_CREAM = ColorRGB15(0xFCE4B8); // idx 169
inline constexpr uint16_t COL_WARM_CREAM = ColorRGB15(0xE8C898); // idx 170
inline constexpr uint16_t COL_BEIGE = ColorRGB15(0xD4AC7C); // idx 171
inline constexpr uint16_t COL_DARK_BEIGE_2 = ColorRGB15(0xC49064); // idx 172
inline constexpr uint16_t COL_CAMEL = ColorRGB15(0xB0744C); // idx 173
inline constexpr uint16_t COL_DARK_CAMEL = ColorRGB15(0xA05C38); // idx 174
inline constexpr uint16_t COL_BROWN = ColorRGB15(0x904C2C); // idx 175

inline constexpr uint16_t COL_MAHOGANY = ColorRGB15(0x843C20); // idx 176
inline constexpr uint16_t COL_DARK_MAHOGANY = ColorRGB15(0x782C18); // idx 177
inline constexpr uint16_t COL_DEEP_MAHOGANY = ColorRGB15(0x6C2010); // idx 178
inline constexpr uint16_t COL_VERY_DARK_MAHOGANY = ColorRGB15(0x5C1408); // idx 179
inline constexpr uint16_t COL_NEAR_BLACK_MAHOGANY = ColorRGB15(0x480C04); // idx 180
inline constexpr uint16_t COL_BLACKISH_MAHOGANY = ColorRGB15(0x3C0400); // idx 181
inline constexpr uint16_t COL_PALE_PINKISH_PEACH = ColorRGB15(0xFCE8DC); // idx 182
inline constexpr uint16_t COL_PINKISH_PEACH = ColorRGB15(0xF8D4BC); // idx 183
inline constexpr uint16_t COL_PEACH_2 = ColorRGB15(0xF4C0A0); // idx 184
inline constexpr uint16_t COL_LIGHT_PEACH = ColorRGB15(0xF0B084); // idx 185
inline constexpr uint16_t COL_PEACH_3 = ColorRGB15(0xF0A06C); // idx 186
inline constexpr uint16_t COL_ORANGE_PEACH = ColorRGB15(0xF0945C); // idx 187
inline constexpr uint16_t COL_TERRACOTTA = ColorRGB15(0xD88054); // idx 188
inline constexpr uint16_t COL_DARK_TERRACOTTA = ColorRGB15(0xC07048); // idx 189
inline constexpr uint16_t COL_BRICK_2 = ColorRGB15(0xA86040); // idx 190
inline constexpr uint16_t COL_DARK_BRICK_2 = ColorRGB15(0x905038); // idx 191

inline constexpr uint16_t COL_DEEP_BRICK = ColorRGB15(0x784030); // idx 192
inline constexpr uint16_t COL_VERY_DARK_BRICK = ColorRGB15(0x603024); // idx 193
inline constexpr uint16_t COL_NEAR_BLACK_BRICK = ColorRGB15(0x48241C); // idx 194
inline constexpr uint16_t COL_BLACKISH_BRICK = ColorRGB15(0x381814); // idx 195
inline constexpr uint16_t COL_BRIGHT_GREEN_2 = ColorRGB15(0x64E464); // idx 196
inline constexpr uint16_t COL_MEDIUM_GREEN = ColorRGB15(0x149814); // idx 197
inline constexpr uint16_t COL_PURE_GREEN = ColorRGB15(0x00A400); // idx 198
inline constexpr uint16_t COL_DARK_WARM_GRAY = ColorRGB15(0x505048); // idx 199
inline constexpr uint16_t COL_DARK_GREEN_2 = ColorRGB15(0x006C00); // idx 200
inline constexpr uint16_t COL_WARM_GRAY_2 = ColorRGB15(0x8C8C84); // idx 201
inline constexpr uint16_t COL_NEAR_BLACK_3 = ColorRGB15(0x1C1C1C); // idx 202
inline constexpr uint16_t COL_DARK_WARM_BROWN = ColorRGB15(0x685038); // idx 203
inline constexpr uint16_t COL_VERY_DARK_BROWN_3 = ColorRGB15(0x302820); // idx 204
inline constexpr uint16_t COL_WARM_BROWN = ColorRGB15(0x8C7060); // idx 205
inline constexpr uint16_t COL_DARK_BROWN_2 = ColorRGB15(0x483828); // idx 206
inline constexpr uint16_t COL_BLACK_2 = ColorRGB15(0x0C0C0C); // idx 207

inline constexpr uint16_t COL_DARK_GRAY_2 = ColorRGB15(0x3C3C3C); // idx 208
inline constexpr uint16_t COL_GRAYISH_GREEN_2 = ColorRGB15(0x6C746C); // idx 209
inline constexpr uint16_t COL_GRAY_OLIVE = ColorRGB15(0x788478); // idx 210
inline constexpr uint16_t COL_OLIVE_GRAY = ColorRGB15(0x889488); // idx 211
inline constexpr uint16_t COL_PALE_OLIVE_GRAY = ColorRGB15(0x94A494); // idx 212
inline constexpr uint16_t COL_DARK_OLIVE_GRAY = ColorRGB15(0x586860); // idx 213
inline constexpr uint16_t COL_GRAY_GREEN = ColorRGB15(0x607068); // idx 214
inline constexpr uint16_t COL_LIME_GREEN = ColorRGB15(0x3CF800); // idx 215
inline constexpr uint16_t COL_BRIGHT_LIME = ColorRGB15(0x38D408); // idx 216
inline constexpr uint16_t COL_GREEN_LIME = ColorRGB15(0x34B410); // idx 217
inline constexpr uint16_t COL_FOREST_GREEN_2 = ColorRGB15(0x309414); // idx 218
inline constexpr uint16_t COL_DARK_FOREST = ColorRGB15(0x287418); // idx 219
inline constexpr uint16_t COL_WHITE = ColorRGB15(0xFCFCFC); // idx 220
inline constexpr uint16_t COL_PALE_IVORY = ColorRGB15(0xF0ECD0); // idx 221
inline constexpr uint16_t COL_IVORY = ColorRGB15(0xD0B888); // idx 222
inline constexpr uint16_t COL_TAN = ColorRGB15(0x987C50); // idx 223

inline constexpr uint16_t COL_DARK_TAN = ColorRGB15(0x68583C); // idx 224
inline constexpr uint16_t COL_VERY_DARK_TAN = ColorRGB15(0x504024); // idx 225
inline constexpr uint16_t COL_NEAR_BLACK_TAN = ColorRGB15(0x34281C); // idx 226
inline constexpr uint16_t COL_BLACKISH_TAN = ColorRGB15(0x18100C); // idx 227
inline constexpr uint16_t COL_BLACK = ColorRGB15(0x000000); // idx 228
// Indices 229-255 are all #ffffff but are actually used for color cycling

} // namespace fallout

#endif /* COLOR_H */
