/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "Sketch.h"
#include "Context.h"

#include "TestingJS.h"
#include "TestingWrappedValue.h"
#include "TestingRooting2.h"
#include "TestingCallbacks.h"

#include "chronotext/utils/GLUtils.h"

using namespace std;
using namespace ci;
using namespace chr;

void Sketch::setup()
{
    context::setup();
    
    // ---
    
    /*
     * SOME OF THE TESTS ARE ONLY INTENDED FOR OSX + DEBUG:
     *
     * - "JS_DEBUG" AND "JSP_USE_PRIVATE_APIS" MUST BE DEFINED
     * - stderr "CAPTURE" MUST BE SUPPORTED
     * - ETC.
     */
    
#if defined(CINDER_MAC) && defined(DEBUG)
    TestingBase::execute<TestingWrappedValue>(true);
    TestingBase::execute<TestingRooting2>(true);
#endif
    
    TestingBase::execute<TestingJS>(true);
    TestingBase::execute<TestingCallbacks>(true);
    
    // ---
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
}

void Sketch::shutdown()
{
    context::shutdown();
}

void Sketch::draw()
{
    gl::setMatricesWindow(getWindowSize(), true);
    gl::clear(Color::gray(0.5f), false);
    
    gl::color(Color::white());
    utils::gl::drawGrid(getWindowBounds(), 64, Vec2f(0, clock()->getTime() * 60));
}
