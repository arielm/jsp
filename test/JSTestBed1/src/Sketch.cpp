/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "Sketch.h"

#include "TestingJS.h"
#include "TestingWrappedValue.h"
#include "TestingRooting2.h"
#include "TestingCallbacks.h"
#include "TestingProxy.h"

#include "jsp/Manager.h"

#include "chronotext/utils/GLUtils.h"

using namespace std;
using namespace ci;
using namespace chr;

void Sketch::setup()
{
    /*
     * SOME OF THE TESTS WILL FAIL, UNLESS RUNNING ON OSX + DEBUG, E.G.
     * - CHECKING IS A JS-OBJECT IS "HEALTHY"
     * - CHECKING IF A JS-OBJECT IS IN THE NURSERY, ETC.
     *
     * MAINLY BECAUSE TRUE "POISONING" IS ONLY TAKING PLACE UNDER CERTAIN CONDITIONS:
     * - SPIDERMONKEY MUST HAVE BEEN BUILT WITH "JS_DEBUG"
     * - RUNNING IN DEBUG MODE
     */

    jsp::Manager manager;
    {
        manager.init();
        
#if defined(CINDER_MAC) && defined(DEBUG)
        TestingBase::execute<TestingWrappedValue>(true);
        TestingBase::execute<TestingRooting2>(true);
#endif
        
        TestingBase::execute<TestingJS>(true);
        TestingBase::execute<TestingCallbacks>(true);
        TestingBase::execute<TestingProxy>(true);
        
        manager.shutdown();
    }
    
    // ---
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
}

void Sketch::draw()
{
    gl::setMatricesWindow(getWindowSize(), true);
    gl::clear(Color::gray(0.5f), false);
    
    gl::color(Color::white());
    utils::gl::drawGrid(getWindowBounds(), 64, Vec2f(0, clock()->getTime() * 60));
}
