#include "map_edge.h"

#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <vector>

#include "db.h"
#include "debug.h"
#include "platform_compat.h"
#include "tile.h"

namespace fallout {

// One EDG record, stored as an axis-aligned box in pixel-offset space.
// Pixel-offset X is inverted relative to the game's tile X: larger pixel-X
// corresponds to smaller tile-X. We just take min/max so order doesn't matter.
struct EdgeBox {
    int minPx;
    int maxPx;
    int minPy;
    int maxPy;
};

static std::vector<EdgeBox> gEdgBoxes[ELEVATION_COUNT];
static bool gEdgLoaded = false;

// Tile index -> pixel-offset coordinate.
// This is the CE/sfall function; the EDG corner values are interpreted
// through it. Not the same as tileToScreenXY.
static void tileToPixelOffset(int tile, int& outX, int& outY)
{
    int x = tile % HEX_GRID_WIDTH;
    int y = tile / HEX_GRID_WIDTH + x / 2;
    y &= ~1;
    x = 2 * x + HEX_GRID_WIDTH - y;
    outY = 12 * y;
    outX = 16 * x;
}

bool mapEdgeIsLoaded()
{
    return gEdgLoaded;
}

void mapEdgeFree()
{
    for (int e = 0; e < ELEVATION_COUNT; e++) {
        gEdgBoxes[e].clear();
    }
    gEdgLoaded = false;
}

bool mapEdgeTileIsInBounds(int elevation, int tile)
{
    if (!gEdgLoaded) return true;
    if (elevation < 0 || elevation >= ELEVATION_COUNT) return true;
    if (gEdgBoxes[elevation].empty()) return true;
    if (tile < 0 || tile >= HEX_GRID_WIDTH * HEX_GRID_HEIGHT) return false;

    int px, py;
    tileToPixelOffset(tile, px, py);

    for (const auto& b : gEdgBoxes[elevation]) {
        if (px >= b.minPx && px <= b.maxPx
            && py >= b.minPy && py <= b.maxPy) {
            return true;
        }
    }
    return false;
}

void mapEdgeLoad(const char* mapName)
{
    mapEdgeFree();

    char fname[COMPAT_MAX_FNAME];
    compat_splitpath(mapName, nullptr, nullptr, fname, nullptr);
    if (fname[0] == '\0') return;

    char edgPath[COMPAT_MAX_PATH];
    snprintf(edgPath, sizeof(edgPath), "MAPS\\%s.EDG", fname);

    File* stream = fileOpen(edgPath, "rb");
    if (stream == nullptr) return;

    int magic = 0;
    int version = 0;
    int reserved = 0;

    if (fileReadInt32(stream, &magic) == -1 || magic != 'EDGE') {
        fileClose(stream);
        return;
    }
    if (fileReadInt32(stream, &version) == -1 || version != 1) {
        fileClose(stream);
        return;
    }
    if (fileReadInt32(stream, &reserved) == -1) {
        fileClose(stream);
        return;
    }

    // File layout after the 12-byte header: repeating records of
    // 4 big-endian int32 corner tile indices, followed by a 4-byte
    // levelIndicator for the NEXT record. The final record has no
    // trailing indicator; we detect it via the all-zeros pattern that
    // xfileRead produces past EOF (it returns 0 items read, not -1).
    const int kMaxRecords = 64;
    int currentElev = 0;

    for (int rec = 0; rec < kMaxRecords; rec++) {
        int corners[4];
        bool readFailed = false;
        for (int i = 0; i < 4; i++) {
            if (fileReadInt32(stream, &corners[i]) == -1) {
                readFailed = true;
                break;
            }
        }
        if (readFailed) break;

        int levelIndicator = 0;
        if (fileReadInt32(stream, &levelIndicator) == -1) break;

        if (corners[0] == 0 && corners[1] == 0 && corners[2] == 0
            && corners[3] == 0 && levelIndicator == 0) {
            break;
        }

        if (currentElev >= 0 && currentElev < ELEVATION_COUNT) {
            int px0, py0, px1, py1, px2, py2, px3, py3;
            tileToPixelOffset(corners[0], px0, py0);
            tileToPixelOffset(corners[1], px1, py1);
            tileToPixelOffset(corners[2], px2, py2);
            tileToPixelOffset(corners[3], px3, py3);

            EdgeBox box;
            box.minPx = std::min(px0, px2);
            box.maxPx = std::max(px0, px2);
            box.minPy = std::min(py1, py3);
            box.maxPy = std::max(py1, py3);

            gEdgBoxes[currentElev].push_back(box);

            debugPrint("mapEdgeLoad: elev=%d box px[%d..%d] py[%d..%d]\n",
                currentElev, box.minPx, box.maxPx, box.minPy, box.maxPy);
        }

        currentElev = levelIndicator;
    }

    fileClose(stream);
    gEdgLoaded = true;

    debugPrint("mapEdgeLoad: loaded %s (zones %d/%d/%d)\n", edgPath,
        (int)gEdgBoxes[0].size(),
        (int)gEdgBoxes[1].size(),
        (int)gEdgBoxes[2].size());
}

} // namespace fallout