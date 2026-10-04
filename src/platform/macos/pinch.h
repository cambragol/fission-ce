#ifndef PINCH_H
#define PINCH_H

namespace fallout {

// Install an NSEvent monitor for magnify gestures. Must be called after SDL
// has created its window so the Cocoa run loop is live.
void pinchInit();

} // namespace fallout

#endif /* PINCH_H */