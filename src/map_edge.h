#ifndef MAP_EDGE_H
#define MAP_EDGE_H

namespace fallout {

void mapEdgeLoad(const char* mapName);
void mapEdgeFree();
bool mapEdgeIsLoaded();

// True if this tile is inside any EDG zone for this elevation (in pixel-offset space).
// Returns true (no constraint) if EDG isn't loaded or has no zones for this elevation.
bool mapEdgeTileIsInBox(int elevation, int tile);

// True if a viewport of the given size can fit inside any EDG zone for this
// elevation (ignoring where exactly; just checks the zone is large enough).
bool mapEdgeViewportFitsInAnyZone(int elevation, int viewWidth, int viewHeight);

} // namespace fallout

#endif