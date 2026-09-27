#include "game_version.h"

#include "debug.h"

namespace fallout {

static FalloutVersion gFalloutVersion = FALLOUT_VERSION_2;

FalloutVersion falloutVersionGet()
{
    return gFalloutVersion;
}

void falloutVersionSet(FalloutVersion version)
{
    if (version == FALLOUT_VERSION_UNKNOWN) {
        return;
    }

    // Once we've committed to Fallout 1, stay there. F2-format archives like
    // fission.dat get opened after master.dat and must not flip us back.
    if (gFalloutVersion == FALLOUT_VERSION_1 && version == FALLOUT_VERSION_2) {
        return;
    }

    if (gFalloutVersion != version) {
        debugPrint("[GAME] Version set to %s\n",
            version == FALLOUT_VERSION_1 ? "Fallout 1" : "Fallout 2");
        gFalloutVersion = version;
    }
}

bool falloutVersionIsFallout1()
{
    return gFalloutVersion == FALLOUT_VERSION_1;
}

} // namespace fallout