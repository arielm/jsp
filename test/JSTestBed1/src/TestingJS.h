/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#pragma once

#include "TestingJSBase.h"

class TestingJS : public TestingJSBase
{
public:
    void run(bool force = false) final;
    
    // ---

    void testShapes1();
    void testShapes2();
    void testAtoms();
    void testReservedSlots();
    void testJSID();

    void dumpIds(JSObject *object);
    
    JSObject* createObject1();
    JSObject* createObject2();
    JSObject* createArray();

    // ---
    
    void testThreadSafety();
    
    void testEvaluationScope();
    void testFunctionScope();
    void testCustomScriptExecution();
    
    void testStringify();
    void testToSource();
    void initComplexJSObject(chr::InputSource::Ref inputSource);
};
