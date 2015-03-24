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
    void setup() final;
    void shutdown() final;
    void run(bool force = false) final;
    
    // ---
    
    void handleObject1(JS::HandleObject object, const std::string &source);
    
    void handleWrappedBarker1(JS::Handle<jsp::WrappedObject> wrappedBarker);
    void handleMutableWrappedBarker1(JS::MutableHandle<jsp::WrappedObject> wrappedBarker);
    
    // ---
    
    void testAnalysis1();
    void testAnalysis2();
    void testAnalysis3();

    void testWrappedObjectAssignment1();
    void testWrappedObjectAssignment2();
    void testWrappedObjectAssignment3();

    void testBarkerFinalization1();
    void testHeapWrappedObject1();
    void testWrappedBarker1();
    void testRootedWrappedBarker1();
    void testHeapWrappedBarker1();
    void testHeapWrappedJSBarker1();
    
    void testBarkerPassedToJS1();
    void testHeapWrappedBarkerPassedToJS1();
};
