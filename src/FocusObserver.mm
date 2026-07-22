#include <Geode/Geode.hpp>
#import <Foundation/NSNotification.h>
#import <dispatch/dispatch.h>

extern NSString* const NSApplicationDidBecomeActiveNotification;
extern NSString* const NSApplicationDidUnhideNotification;

@interface NSCursor : NSObject
+ (void)hide;
+ (void)unhide;
@end

using namespace geode::prelude;

void onAppBecameActive();

@interface FocusObserver : NSObject
@end

@implementation FocusObserver
- (void)appBecameActive:(NSNotification*)note {
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.3 * NSEC_PER_SEC)),
dispatch_get_main_queue(), ^{
        onAppBecameActive();
    });
}
@end

static FocusObserver* g_focusObserver = nil;

$execute {
    g_focusObserver = [[FocusObserver alloc] init];
    [[NSNotificationCenter defaultCenter] addObserver:g_focusObserver
    selector:@selector(appBecameActive:)
    name:NSApplicationDidBecomeActiveNotification
    object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:g_focusObserver
    selector:@selector(appBecameActive:)
    name:NSApplicationDidUnhideNotification
    object:nil];
}

extern "C" void platformHideCursorNS() {
    [NSCursor hide];
}

extern "C" void platformShowCursorNS() {
    [NSCursor unhide];
}