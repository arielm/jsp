/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#import "Application.h"

@implementation Application

- (BOOL) application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    bridge = [[CinderBridge alloc] init];
    
    bridge.viewControllerProperties = [NSDictionary dictionaryWithObjectsAndKeys:
      [NSNumber numberWithInt:kEAGLRenderingAPIOpenGLES1], kGLViewControllerPropertyRenderingAPI,
      [NSNumber numberWithInt:UIInterfaceOrientationMaskLandscape], kGLViewControllerPropertyInterfaceOrientationMask,
      [NSNumber numberWithInt:GLKViewDrawableDepthFormatNone], kGLViewControllerPropertyDepthFormat,
      nil];
    
    window = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
    window.backgroundColor = [UIColor blackColor];
    
    [window setRootViewController:bridge.viewController];
    [window makeKeyAndVisible];
    
    return YES;
}

- (void) dealloc
{
    [window release];
    [bridge release];
    
    [super dealloc];
}

@end
