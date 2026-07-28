#include "color.h"

#include <math.h>
#include <string.h>

#include <algorithm>

#include "db.h"
#include "memory.h"
#include "svga.h"

namespace fallout {

/*
 * Original fallout.act Palette -> 15-bit key, with unique human-readable colour names
/*
    Block 0 (indices 0-15)
       0,  // idx   0: #0000FF  (bright blue)
   30653,  // idx   1: #ECECEC  (very light gray)
   28539,  // idx   2: #DCDCDC  (light gray)
   26425,  // idx   3: #CCCCCC  (silver)
   24311,  // idx   4: #BCBCBC  (medium light gray)
   23254,  // idx   5: #B0B0B0  (medium gray)
   21140,  // idx   6: #A0A0A0  (gunmetal)
   19026,  // idx   7: #909090  (dark medium gray)
   16912,  // idx   8: #808080  (dark gray)
   14798,  // idx   9: #747474  (charcoal)
   12684,  // idx  10: #646464  (dark charcoal)
   10570,  // idx  11: #545454  (very dark gray)
    9513,  // idx  12: #444444  (dark slate)
    7399,  // idx  13: #383838  (anthracite)
    5285,  // idx  14: #282828  (near black)
    4228,  // idx  15: #202020  (almost black)

    Block 1 (indices 16-31)
   32701,  // idx  16: #FCECEC  (very pale pink)
   30587,  // idx  17: #ECD8D8  (pale pink)
   28440,  // idx  18: #DCC4C4  (light dusty rose)
   27350,  // idx  19: #D0B0B0  (dusty rose)
   25236,  // idx  20: #C0A0A0  (muted rose)
   23122,  // idx  21: #B09090  (old rose)
   21008,  // idx  22: #A48080  (rosewood)
   18894,  // idx  23: #947070  (dusty mauve)
   16780,  // idx  24: #846060  (desert rose)
   15690,  // idx  25: #785454  (warm brick)
   13576,  // idx  26: #684444  (brick red)
   11495,  // idx  27: #583838  (dark brick)
    9381,  // idx  28: #4C2C2C  (deep maroon)
    7300,  // idx  29: #3C2424  (very dark maroon)
    5219,  // idx  30: #2C1818  (nearly black brown)
    4162,  // idx  31: #201010  (blackish red)

    Block 2 (indices 32-47)
   30655,  // idx  32: #ECECFC  (very pale lavender)
   28541,  // idx  33: #D8D8EC  (pale lavender)
   25371,  // idx  34: #C4C4DC  (light periwinkle)
   23258,  // idx  35: #B0B0D0  (periwinkle)
   21144,  // idx  36: #A0A0C0  (muted periwinkle)
   19030,  // idx  37: #9090B0  (cadet gray)
   16916,  // idx  38: #8080A4  (slate blue)
   14802,  // idx  39: #707094  (dark slate blue)
   12688,  // idx  40: #606084  (deep slate)
   10575,  // idx  41: #545478  (dark cadet)
    8461,  // idx  42: #444468  (blue gray)
    7403,  // idx  43: #383858  (dark blue gray)
    5289,  // idx  44: #2C2C4C  (very dark blue)
    4231,  // idx  45: #24243C  (midnight blue)
    3173,  // idx  46: #18182C  (near black blue)
    2116,  // idx  47: #101020  (blackish blue)

    Block 3 (indices 48-63)
   32478,  // idx  48: #FCB0F0  (orchid pink)
   24981,  // idx  49: #C460A8  (mauve)
   13452,  // idx  50: #682460  (deep mauve)
    9289,  // idx  51: #4C1448  (dark plum)
    7206,  // idx  52: #380C34  (aubergine)
    5188,  // idx  53: #281024  (very dark plum)
    4100,  // idx  54: #240424  (blackish plum)
    3107,  // idx  55: #1C0C18  (near black purple)
   32761,  // idx  56: #FCFCC8  (very pale yellow)
   32751,  // idx  57: #FCFC7C  (light lemon)
   29537,  // idx  58: #E4D80C  (golden yellow)
   26339,  // idx  59: #CCB81C  (mustard)
   24165,  // idx  60: #B89C28  (olive yellow)
   21030,  // idx  61: #A48830  (dark mustard)
   18916,  // idx  62: #907824  (greenish brown)
   15779,  // idx  63: #7C6818  (dark olive)

    Block 4 (indices 64-79)
   13666,  // idx  64: #6C5810  (olive brown)
   11553,  // idx  65: #584808  (dark olive brown)
    9440,  // idx  66: #483804  (very dark olive)
    6304,  // idx  67: #342800  (blackish olive)
    4192,  // idx  68: #201800  (near black brown)
   28659,  // idx  69: #D8FC9C  (light spring green)
   23408,  // idx  70: #B4D884  (pale green)
   20206,  // idx  71: #98B870  (sage green)
   15979,  // idx  72: #78985C  (muted green)
   11753,  // idx  73: #5C7848  (fern green)
    8550,  // idx  74: #405834  (dark fern)
    5348,  // idx  75: #283820  (very dark green)
   14730,  // idx  76: #706050  (warm gray brown)
   10534,  // idx  77: #544834  (dark beige)
    7364,  // idx  78: #383020  (cocoa brown)
   13802,  // idx  79: #687850  (olive green)

    Block 5 (indices 80-95)
   14820,  // idx  80: #707820  (moss)
   14757,  // idx  81: #706828  (drab green)
   12676,  // idx  82: #606024  (dark moss)
    9476,  // idx  83: #4C4424  (olive drab)
    7364,  // idx  84: #383020  (cocoa brown)
   20147,  // idx  85: #9CAC9C  (pale sage)
   15951,  // idx  86: #789478  (grayish green)
   11755,  // idx  87: #587C58  (forest green)
    8616,  // idx  88: #406840  (dark forest green)
    7531,  // idx  89: #385858  (teal gray)
    6441,  // idx  90: #304C48  (dark teal)
    5383,  // idx  91: #28443C  (deep teal)
    4325,  // idx  92: #203C2C  (dark green teal)
    3268,  // idx  93: #1C3024  (very dark teal)
    2211,  // idx  94: #142818  (blackish teal)
    2178,  // idx  95: #102010  (near black green)

    Block 6 (indices 96-111)
    3267,  // idx  96: #183018  (very dark forest)
    2177,  // idx  97: #10240C  (blackish green)
    1120,  // idx  98: #081C04  (almost black green)
      64,  // idx  99: #041400  (near black)
      32,  // idx 100: #040C00  (blackish)
   18035,  // idx 101: #8C9C9C  (cool gray)
   15955,  // idx 102: #789498  (grayish cyan)
   12850,  // idx 103: #648894  (dusty blue)
   10738,  // idx 104: #507C90  (steel blue)
    8625,  // idx 105: #406C8C  (dark steel blue)
    6513,  // idx 106: #30588C  (blue navy)
    5423,  // idx 107: #2C4C7C  (dark navy)
    5389,  // idx 108: #28446C  (navy)
    4331,  // idx 109: #20385C  (deep navy)
    3273,  // idx 110: #1C304C  (very dark navy)
    3240,  // idx 111: #182840  (blackish navy)

    Block 7 (indices 112-127)
   20116,  // idx 112: #9CA4A4  (warm gray)
    7469,  // idx 113: #384868  (dark blue slate)
   10603,  // idx 114: #505858  (cool dark gray)
   11696,  // idx 115: #586884  (slate gray)
    7434,  // idx 116: #384050  (dark slate)
   24311,  // idx 117: #BCBCBC  (light silver)
   22163,  // idx 118: #ACA498  (stone)
   21071,  // idx 119: #A0907C  (warm stone)
   18924,  // idx 120: #947C60  (sandstone)
   17833,  // idx 121: #88684C  (warm taupe)
   15718,  // idx 122: #7C5834  (taupe)
   14628,  // idx 123: #704824  (brown taupe)
   12514,  // idx 124: #643C14  (dark taupe)
   11457,  // idx 125: #583008  (very dark taupe)
   32569,  // idx 126: #FCCCCC  (pale coral)
   32470,  // idx 127: #FCB0B0  (coral)

    Block 8 (indices 128-143)
   32371,  // idx 128: #FC9898  (light coral)
   32239,  // idx 129: #FC7C7C  (salmon)
   32140,  // idx 130: #FC6464  (warm salmon)
   32041,  // idx 131: #FC4848  (bright red)
   31942,  // idx 132: #FC3030  (vivid red)
   31744,  // idx 133: #FC0000  (pure red)
   28672,  // idx 134: #E00000  (red)
   24576,  // idx 135: #C40000  (dark red)
   21504,  // idx 136: #A80000  (deep red)
   18432,  // idx 137: #900000  (ruby red)
   14336,  // idx 138: #740000  (dark ruby)
   11264,  // idx 139: #580000  (very dark red)
    8192,  // idx 140: #400000  (dark red)
   32665,  // idx 141: #FCE0C8  (pale peach)
   32530,  // idx 142: #FCC494  (peach)
   32495,  // idx 143: #FCB878  (light apricot)

    Block 9 (indices 144-159)
   32428,  // idx 144: #FCAC60  (apricot)
   32361,  // idx 145: #FC9C48  (orange)
   32325,  // idx 146: #FC942C  (vibrant orange)
   32290,  // idx 147: #FC8814  (bright orange)
   32224,  // idx 148: #FC7C00  (pure orange)
   28064,  // idx 149: #DC6C00  (dark orange)
   24960,  // idx 150: #C06000  (burnt orange)
   20800,  // idx 151: #A45000  (rust orange)
   16640,  // idx 152: #844400  (rust)
   13504,  // idx 153: #683400  (dark rust)
    9344,  // idx 154: #4C2400  (very dark rust)
    6240,  // idx 155: #301800  (blackish rust)
   32596,  // idx 156: #F8D4A4  (pale sand)
   28367,  // idx 157: #D8B078  (sand)
   26252,  // idx 158: #C8A064  (warm sand)
   24138,  // idx 159: #BC9054  (desert sand)

    Block 10 (indices 160-175)
   22024,  // idx 160: #AC8044  (warm sand)
   19910,  // idx 161: #9C7434  (buff)
   17797,  // idx 162: #8C6428  (dark buff)
   15715,  // idx 163: #7C581C  (brown buff)
   14626,  // idx 164: #704C14  (dark brown)
   12545,  // idx 165: #604008  (very dark brown)
   10432,  // idx 166: #503404  (near black brown)
    8352,  // idx 167: #402800  (blackish brown)
    6272,  // idx 168: #342000  (very dark)
   32663,  // idx 169: #FCE4B8  (cream)
   30515,  // idx 170: #E8C898  (warm cream)
   27311,  // idx 171: #D4AC7C  (beige)
   25164,  // idx 172: #C49064  (dark beige)
   22985,  // idx 173: #B0744C  (camel)
   20839,  // idx 174: #A05C38  (dark camel)
   18725,  // idx 175: #904C2C  (brown)

    Block 11 (indices 176-191)
   16612,  // idx 176: #843C20  (mahogany)
   15523,  // idx 177: #782C18  (dark mahogany)
   13442,  // idx 178: #6C2010  (deep mahogany)
   11329,  // idx 179: #5C1408  (very dark mahogany)
    9248,  // idx 180: #480C04  (near black mahogany)
    7168,  // idx 181: #3C0400  (blackish mahogany)
   32699,  // idx 182: #FCE8DC  (pale pinkish peach)
   32599,  // idx 183: #F8D4BC  (pinkish peach)
   31508,  // idx 184: #F4C0A0  (peach)
   31440,  // idx 185: #F0B084  (light peach)
   31373,  // idx 186: #F0A06C  (peach)
   31307,  // idx 187: #F0945C  (orange peach)
   28170,  // idx 188: #D88054  (terracotta)
   25033,  // idx 189: #C07048  (dark terracotta)
   21896,  // idx 190: #A86040  (brick)
   18759,  // idx 191: #905038  (dark brick)

    Block 12 (indices 192-207)
   15622,  // idx 192: #784030  (deep brick)
   12484,  // idx 193: #603024  (very dark brick)
    9347,  // idx 194: #48241C  (near black brick)
    7266,  // idx 195: #381814  (blackish brick)
   13196,  // idx 196: #64E464  (bright green)
    2658,  // idx 197: #149814  (medium green)
     640,  // idx 198: #00A400  (pure green)
   10569,  // idx 199: #505048  (dark warm gray)
     416,  // idx 200: #006C00  (dark green)
   17968,  // idx 201: #8C8C84  (warm gray)
    3171,  // idx 202: #1C1C1C  (near black)
   13639,  // idx 203: #685038  (dark warm brown)
    6308,  // idx 204: #302820  (very dark brown)
   17868,  // idx 205: #8C7060  (warm brown)
    9445,  // idx 206: #483828  (dark brown)
    1057,  // idx 207: #0C0C0C  (black)

    Block 13 (indices 208-223)
    7399,  // idx 208: #3C3C3C  (dark gray)
   13773,  // idx 209: #6C746C  (grayish green)
   15887,  // idx 210: #788478  (gray olive)
   18001,  // idx 211: #889488  (olive gray)
   19090,  // idx 212: #94A494  (pale olive gray)
   11692,  // idx 213: #586860  (dark olive gray)
   12749,  // idx 214: #607068  (gray green)
    8160,  // idx 215: #3CF800  (lime green)
    8001,  // idx 216: #38D408  (bright lime)
    6850,  // idx 217: #34B410  (green lime)
    6722,  // idx 218: #309414  (forest green)
    5571,  // idx 219: #287418  (dark forest)
   32767,  // idx 220: #FCFCFC  (white)
   31674,  // idx 221: #F0ECD0  (pale ivory)
   27377,  // idx 222: #D0B888  (ivory)
   19946,  // idx 223: #987C50  (tan)

    Block 14 (indices 224-239)
   13671,  // idx 224: #68583C  (dark tan)
   10500,  // idx 225: #504024  (very dark tan)
    6307,  // idx 226: #34281C  (near black tan)
    3137,  // idx 227: #18100C  (blackish tan)
       0,  // idx 228: #000000  (color cycle)
       0,  // idx 229: #000000  (color cycle)
       0,  // idx 230: #000000  (color cycle)
       0,  // idx 231: #000000  (color cycle)
       0,  // idx 232: #000000  (color cycle)
       0,  // idx 233: #000000  (color cycle)
       0,  // idx 234: #000000  (color cycle)
       0,  // idx 235: #000000  (color cycle)
       0,  // idx 236: #000000  (color cycle)
       0,  // idx 237: #000000  (color cycle)
       0,  // idx 238: #000000  (color cycle)
       0,  // idx 239: #000000  (color cycle)

    Block 15 (indices 240-255)
       0,  // idx 240: #000000  (color cycle)
       0,  // idx 241: #000000  (color cycle)
       0,  // idx 242: #000000  (color cycle)
       0,  // idx 243: #000000  (color cycle)
       0,  // idx 244: #000000  (color cycle)
       0,  // idx 245: #000000  (color cycle)
       0,  // idx 246: #000000  (color cycle)
       0,  // idx 247: #000000  (color cycle)
       0,  // idx 248: #000000  (color cycle)
       0,  // idx 249: #000000  (color cycle)
       0,  // idx 250: #000000  (color cycle)
       0,  // idx 251: #000000  (color cycle)
       0,  // idx 252: #000000  (color cycle)
       0,  // idx 253: #000000  (color cycle)
       0,  // idx 254: #000000  (color cycle)
       0   // idx 255: #000000  (color cycle)
*/

static void _setIntensityTableColor(int color);
static void _setIntensityTables();
static void _setMixTableColor(int color);
static void _buildBlendTable(unsigned char* ptr, unsigned char ch);
static void _rebuildColorBlendTables();

// 0x50F930
static char _aColor_cNoError[] = "color.c: No errors\n";

// 0x50F95C
static char _aColor_cColorTa[] = "color.c: color table not found\n";

// 0x50F984
static char _aColor_cColorpa[] = "color.c: colorpalettestack overflow";

// 0x50F9AC
static char aColor_cColor_0[] = "color.c: colorpalettestack underflow";

// 0x51DF10
static char* _errorStr = _aColor_cNoError;

// 0x51DF14
static bool _colorsInited = false;

// 0x51DF18
static double gBrightness = 1.0;

// 0x51DF20
static ColorTransitionCallback* gColorPaletteTransitionCallback = nullptr;

// 0x51DF30
static ColorFileNameManger* gColorFileNameMangler = nullptr;

// 0x51DF34
unsigned char _cmap[768] = {
    0x3F, 0x3F, 0x3F
};

// 0x673090
unsigned char _systemCmap[256 * 3];

// 0x673390
unsigned char _currentGammaTable[64];

// 0x6733D0
unsigned char* _blendTable[256];

// 0x6737D0
unsigned char _mappedColor[256];

// 0x6738D0
Color colorMixAddTable[256][256];

// 0x6838D0
Color intensityColorTable[256][256];

// 0x6938D0
Color colorMixMulTable[256][256];

// 0x6A38D0
unsigned char _colorTable[32768];

// 0x4C72B4
int _calculateColor(int intensity, Color color)
{
    return intensityColorTable[color][intensity / 512];
}

// 0x4C72E0
int Color2RGB(Color c)
{
    int r = _cmap[3 * c] >> 1;
    int g = _cmap[3 * c + 1] >> 1;
    int b = _cmap[3 * c + 2] >> 1;

    return (r << 10) | (g << 5) | b;
}

// Performs animated palette transition.
//
// 0x4C7320
void colorPaletteFadeBetween(unsigned char* oldPalette, unsigned char* newPalette, int steps)
{
    for (int step = 0; step < steps; step++) {
        sharedFpsLimiter.mark();

        unsigned char palette[768];

        for (int index = 0; index < 768; index++) {
            palette[index] = oldPalette[index] - (oldPalette[index] - newPalette[index]) * step / steps;
        }

        if (gColorPaletteTransitionCallback != nullptr) {
            if (step % 128 == 0) {
                gColorPaletteTransitionCallback();
            }
        }

        _setSystemPalette(palette);
        renderPresent();
        sharedFpsLimiter.throttle();
    }

    sharedFpsLimiter.mark();
    _setSystemPalette(newPalette);
    renderPresent();
    sharedFpsLimiter.throttle();
}

// 0x4C73D4
void colorPaletteSetTransitionCallback(ColorTransitionCallback* callback)
{
    gColorPaletteTransitionCallback = callback;
}

// 0x4C73E4
void _setSystemPalette(unsigned char* palette)
{
    unsigned char newPalette[768];

    for (int index = 0; index < 768; index++) {
        newPalette[index] = _currentGammaTable[palette[index]];
        _systemCmap[index] = palette[index];
    }

    directDrawSetPalette(newPalette);
}

// 0x4C7420
unsigned char* _getSystemPalette()
{
    return _systemCmap;
}

// 0x4C7428
void _setSystemPaletteEntries(unsigned char* palette, int start, int end)
{
    unsigned char newPalette[768];

    int length = end - start + 1;
    for (int index = 0; index < length; index++) {
        newPalette[index * 3] = _currentGammaTable[palette[index * 3]];
        newPalette[index * 3 + 1] = _currentGammaTable[palette[index * 3 + 1]];
        newPalette[index * 3 + 2] = _currentGammaTable[palette[index * 3 + 2]];

        _systemCmap[start * 3 + index * 3] = palette[index * 3];
        _systemCmap[start * 3 + index * 3 + 1] = palette[index * 3 + 1];
        _systemCmap[start * 3 + index * 3 + 2] = palette[index * 3 + 2];
    }

    directDrawSetPaletteInRange(newPalette, start, end - start + 1);
}

// 0x4C7550
static void _setIntensityTableColor(int cc)
{
    int shift = 0;

    for (int index = 0; index < 128; index++) {
        int r = (Color2RGB(cc) & 0x7C00) >> 10;
        int g = (Color2RGB(cc) & 0x3E0) >> 5;
        int b = (Color2RGB(cc) & 0x1F);

        int darkerR = ((r * shift) >> 16);
        int darkerG = ((g * shift) >> 16);
        int darkerB = ((b * shift) >> 16);
        int darkerColor = (darkerR << 10) | (darkerG << 5) | darkerB;
        intensityColorTable[cc][index] = _colorTable[darkerColor];

        int lighterR = r + (((0x1F - r) * shift) >> 16);
        int lighterG = g + (((0x1F - g) * shift) >> 16);
        int lighterB = b + (((0x1F - b) * shift) >> 16);
        int lighterColor = (lighterR << 10) | (lighterG << 5) | lighterB;
        intensityColorTable[cc][128 + index] = _colorTable[lighterColor];

        shift += 512;
    }
}

// 0x4C7658
static void _setIntensityTables()
{
    for (int index = 0; index < 256; index++) {
        if (_mappedColor[index] != 0) {
            _setIntensityTableColor(index);
        } else {
            memset(intensityColorTable[index], 0, 256);
        }
    }
}

// 0x4C769C
static void _setMixTableColor(int color)
{
    for (int otherColor = 0; otherColor < 256; otherColor++) {
        if (_mappedColor[color] && _mappedColor[otherColor]) {
            int colorRgb = Color2RGB(color);
            int otherColorRgb = Color2RGB(otherColor);

            int colorR = (colorRgb & 0x7C00) >> 10;
            int colorG = (colorRgb & 0x3E0) >> 5;
            int colorB = colorRgb & 0x1F;

            int otherColorR = (otherColorRgb & 0x7C00) >> 10;
            int otherColorG = (otherColorRgb & 0x3E0) >> 5;
            int otherColorB = otherColorRgb & 0x1F;

            int addedR = colorR + otherColorR;
            int addedG = colorG + otherColorG;
            int addedB = colorB + otherColorB;

            int maxAddedChannel = addedR;
            if (addedG > maxAddedChannel) {
                maxAddedChannel = addedG;
            }
            if (addedB > maxAddedChannel) {
                maxAddedChannel = addedB;
            }

            int additiveColor;
            if (maxAddedChannel <= 0x1F) {
                int paletteIndex = (addedR << 10) | (addedG << 5) | addedB;
                additiveColor = _colorTable[paletteIndex];
            } else {
                int overflow = maxAddedChannel - 0x1F;

                int normalizedR = addedR - overflow;
                int normalizedG = addedG - overflow;
                int normalizedB = addedB - overflow;

                if (normalizedR < 0) {
                    normalizedR = 0;
                }
                if (normalizedG < 0) {
                    normalizedG = 0;
                }
                if (normalizedB < 0) {
                    normalizedB = 0;
                }

                int saturatedPaletteIndex = (normalizedR << 10) | (normalizedG << 5) | normalizedB;
                int saturatedColor = _colorTable[saturatedPaletteIndex];

                int intensity = (int)((((double)maxAddedChannel + (-31.0)) * 0.0078125 + 1.0) * 65536.0);
                additiveColor = _calculateColor(intensity, saturatedColor);
            }

            colorMixAddTable[color][otherColor] = additiveColor;

            int multipliedR = (colorR * otherColorR) >> 5;
            int multipliedG = (colorG * otherColorG) >> 5;
            int multipliedB = (colorB * otherColorB) >> 5;

            int multiplyPaletteIndex = (multipliedR << 10) | (multipliedG << 5) | multipliedB;
            colorMixMulTable[color][otherColor] = _colorTable[multiplyPaletteIndex];
        } else {
            if (_mappedColor[otherColor]) {
                colorMixAddTable[color][otherColor] = otherColor;
                colorMixMulTable[color][otherColor] = otherColor;
            } else {
                colorMixAddTable[color][otherColor] = color;
                colorMixMulTable[color][otherColor] = color;
            }
        }
    }
}

// 0x4C78E4
bool colorPaletteLoad(const char* path)
{
    if (gColorFileNameMangler != nullptr) {
        path = gColorFileNameMangler(path);
    }

    File* stream = fileOpen(path, "rb");
    if (stream == nullptr) {
        _errorStr = _aColor_cColorTa;
        return false;
    }

    for (int index = 0; index < 256; index++) {
        unsigned char r;
        unsigned char g;
        unsigned char b;

        // NOTE: Uninline.
        fileRead(&r, sizeof(r), 1, stream);

        // NOTE: Uninline.
        fileRead(&g, sizeof(g), 1, stream);

        // NOTE: Uninline.
        fileRead(&b, sizeof(b), 1, stream);

        if (r <= 0x3F && g <= 0x3F && b <= 0x3F) {
            _mappedColor[index] = 1;
        } else {
            r = 0;
            g = 0;
            b = 0;
            _mappedColor[index] = 0;
        }

        _cmap[index * 3] = r;
        _cmap[index * 3 + 1] = g;
        _cmap[index * 3 + 2] = b;
    }

    // NOTE: Uninline.
    fileRead(_colorTable, 0x8000, 1, stream);

    unsigned int type = 0;
    // NOTE: Uninline.
    fileRead(&type, sizeof(type), 1, stream);

    // NOTE: The value is "NEWC". Original code uses cmp opcode, not stricmp,
    // or comparing characters one-by-one.
    if (type == 'NEWC') {
        // NOTE: Uninline.
        fileRead(intensityColorTable, sizeof(intensityColorTable), 1, stream);

        // NOTE: Uninline.
        fileRead(colorMixAddTable, sizeof(colorMixAddTable), 1, stream);

        // NOTE: Uninline.
        fileRead(colorMixMulTable, sizeof(colorMixMulTable), 1, stream);
    } else {
        _setIntensityTables();

        for (int index = 0; index < 256; index++) {
            _setMixTableColor(index);
        }
    }

    _rebuildColorBlendTables();

    // NOTE: Uninline.
    fileClose(stream);

    return true;
}

// 0x4C7AB4
char* _colorError()
{
    return _errorStr;
}

// 0x4C7B44
static void _buildBlendTable(unsigned char* ptr, unsigned char ch)
{
    int r, g, b;
    int i, j;
    int mixedR, mixedG, mixedB;
    unsigned char* beg;

    beg = ptr;

    r = (Color2RGB(ch) & 0x7C00) >> 10;
    g = (Color2RGB(ch) & 0x3E0) >> 5;
    b = (Color2RGB(ch) & 0x1F);

    for (i = 0; i < 256; i++) {
        ptr[i] = i;
    }

    ptr += 256;

    int b_1 = b;
    int blendWeight = 6;
    int g_1 = g;
    int r_1 = r;

    int b_2 = b_1;
    int g_2 = g_1;
    int r_2 = r_1;

    for (j = 0; j < 7; j++) {
        for (i = 0; i < 256; i++) {
            mixedR = (Color2RGB(i) & 0x7C00) >> 10;
            mixedG = (Color2RGB(i) & 0x3E0) >> 5;
            mixedB = (Color2RGB(i) & 0x1F);
            int index = 0;
            index |= (r_2 + mixedR * blendWeight) / 7 << 10;
            index |= (g_2 + mixedG * blendWeight) / 7 << 5;
            index |= (b_2 + mixedB * blendWeight) / 7;
            ptr[i] = _colorTable[index];
        }
        blendWeight--;
        ptr += 256;
        r_2 += r_1;
        g_2 += g_1;
        b_2 += b_1;
    }

    int shadeStep = 0;
    for (j = 0; j < 6; j++) {
        int shadeIntensity = shadeStep / 7 + 0xFFFF;

        for (i = 0; i < 256; i++) {
            ptr[i] = _calculateColor(shadeIntensity, ch);
        }

        shadeStep += 0x10000;
        ptr += 256;
    }
}

// 0x4C7D90
static void _rebuildColorBlendTables()
{
    int i;

    for (i = 0; i < 256; i++) {
        if (_blendTable[i]) {
            _buildBlendTable(_blendTable[i], i);
        }
    }
}

// 0x4C7DC0
unsigned char* _getColorBlendTable(int ch)
{
    unsigned char* ptr;

    if (_blendTable[ch] == nullptr) {
        ptr = (unsigned char*)internal_malloc(4100);
        *(int*)ptr = 1;
        _blendTable[ch] = ptr + 4;
        _buildBlendTable(_blendTable[ch], ch);
    }

    ptr = _blendTable[ch];
    *(int*)((unsigned char*)ptr - 4) = *(int*)((unsigned char*)ptr - 4) + 1;

    return ptr;
}

// 0x4C7E20
void _freeColorBlendTable(int color)
{
    unsigned char* blendTable = _blendTable[color];
    if (blendTable != nullptr) {
        int* count = (int*)(blendTable - sizeof(int));
        *count -= 1;
        if (*count == 0) {
            internal_free(count);
            _blendTable[color] = nullptr;
        }
    }
}

// 0x4C7E6C
void colorSetBrightness(double value)
{
    gBrightness = value;

    for (int i = 0; i < 64; i++) {
        double value = pow(i, gBrightness);
        _currentGammaTable[i] = (unsigned char)std::clamp(value, 0.0, 63.0);
    }

    _setSystemPalette(_systemCmap);
}

// 0x4C89CC
bool _initColors()
{
    if (_colorsInited) {
        return true;
    }

    _colorsInited = true;

    colorSetBrightness(1.0);

    if (!colorPaletteLoad("color.pal")) {
        return false;
    }

    _setSystemPalette(_cmap);

    return true;
}

// 0x4C8A18
void _colorsClose()
{
    for (int index = 0; index < 256; index++) {
        _freeColorBlendTable(index);
    }
}

} // namespace fallout
