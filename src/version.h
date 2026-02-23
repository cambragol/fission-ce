#ifndef VERSION_H
#define VERSION_H

#include <stddef.h>

namespace fallout {

// The size of buffer for version string.
#define VERSION_MAX (32)

#define VERSION_MAJOR (0)
#define VERSION_MINOR (.9.1) // update this through PRs?
#define VERSION_RELEASE ('B') // update this with action?

void versionGetVersion(char* dest, size_t size);

} // namespace fallout

#endif /* VERSION_H */
