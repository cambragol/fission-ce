#ifndef MAP_EDGE_H
#define MAP_EDGE_H

namespace fallout {

void mapEdgeLoad(const char* mapName);
void mapEdgeFree();
bool mapEdgeIsLoaded();

// True if the given tile is inside any EDG zone for this elevation.
// Returns true (no constraint) when EDG isn't loaded or this elevation
// has no zones.
bool mapEdgeTileIsInBounds(int elevation, int tile);

} // namespace fallout

#endif /* MAP_EDGE_H */