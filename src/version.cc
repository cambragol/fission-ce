#include "version.h"
#include "sfall_config.h"
#include "settings.h"

#include <stdio.h>

namespace fallout {

// 0x4B4580
void versionGetVersion(char* dest, size_t size)
{
    // modConfig custom version string.
    const char* versionString = nullptr;
    if (!settings.mod_settings.version_string.empty()) {
        versionString = settings.mod_settings.version_string.c_str();
    }
    snprintf(dest, size, (versionString ? versionString : "FALLOUT II %d.%02d"), VERSION_MAJOR, VERSION_MINOR);
}

} // namespace fallout
