#ifndef MAP_EDGE_H
#define MAP_EDGE_H

namespace fallout {

void mapEdgeLoad(const char* mapName);
void mapEdgeFree();
bool mapEdgeIsLoaded();

// True if this tile is inside any EDG zone for this elevation (in pixel-offset space).
// Returns true (no constraint) if EDG isn't loaded or has no zones for this elevation.
bool mapEdgeTileIsInBox(int elevation, int tile);

} // namespace fallout

#endif