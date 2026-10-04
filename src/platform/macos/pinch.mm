#include "pinch.h"

#import <Cocoa/Cocoa.h>

#include "map.h"

namespace fallout {

void pinchInit()
{
    // Local monitor sees all NSEvents destined for this app before they are
    // dispatched. Returning nil consumes the event, which is what we want:
    // SDL doesn't use magnify events on this platform anyway.
    [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskMagnify
        handler:^NSEvent*(NSEvent* event) {
            if (event.magnification != 0.0) {
                mapHandlePinch((float)event.magnification);
            }
            if (event.magnification != 0.0) {
    NSLog(@"pinch: mag=%.4f", event.magnification);
    mapHandlePinch((float)event.magnification);
}
            return nil;
        }];
}

} // namespace fallout