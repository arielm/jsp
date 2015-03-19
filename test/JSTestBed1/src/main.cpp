/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "chronotext/Context.h"

#include "Sketch.h"

namespace chr
{
    CinderSketch* createSketch()
    {
        return new Sketch();
    }
}

#pragma mark ----------------------------------------   IOS   ----------------------------------------

#if defined(CINDER_COCOA_TOUCH)

#include "Application.h"

int main(int argc, char *argv[])
{
    @autoreleasepool
    {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([Application class]));
    }
}

#pragma mark ---------------------------------------- ANDROID ----------------------------------------

#elif defined(CINDER_ANDROID)

extern "C"
{}

#pragma mark ----------------------------------------   OSX   ----------------------------------------

#elif defined(CINDER_MAC)

#include "Application.h"

CINDER_APP_DESKTOP(Application, ci::app::RendererGl::AA_NONE)

#endif
