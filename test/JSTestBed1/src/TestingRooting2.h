/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#pragma once

#include "TestingJSBase.h"

class TestingRooting2 : public TestingJSBase
{
public:
    void performSetup() final;
    void performShutdown() final;
    void performRun(bool force = false) final;
    
    // ---
    
    void testHandleObject1(JS::HandleObject object);
    void testMutableHandleObject1(JS::MutableHandleObject object);
    
    // ---
    
    void testAnalysis1();
    void testAnalysis2();
    void testAnalysis3();
    
    void testBarkerJSFunctionality();
    void testBarkerMixedFunctionality();
    void testRootedBarker1();
    void testBarkerFinalization1();
    void testObjectAllocation1();

    void testWrappedObjectAssignment1();
    void testWrappedBarker1();
    void testRootedWrappedBarker1();
    void testHeapWrappedBarker1();
    void testHeapWrappedJSBarker1();
    
    void testBarkerPassedToJS1();
    void testHeapWrappedJSBarker2();
};
