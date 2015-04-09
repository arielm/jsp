/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#pragma once

#include "TestingJSBase.h"

class TestingWrappedValue : public TestingJSBase
{
public:
    void performSetup() final;
    void performShutdown() final;
    void performRun(bool force = false) final;
    
    // ---
    
    void testStackCreationAndAssignment();
    void testAutomaticConversion();
    
    void testObjectStackRooting1();
    void testObjectStackRooting2();
    void testStringStackRooting1();
    void testStringStackRooting2();

    // ---
    
    void testValueComparison();
    void testObjectComparison();
    void testAutomaticComparison();

    // ---
    
    void testBooleanComparison();
    void testBooleanCasting();

    // ---
    
    void testStringComparison1();
    void compareConstChars(const char *s1, const char *s2);
    void compareConstStrings(const std::string &s1, const std::string &s2);
    
    void testStringComparison2();
    void testStringCasting();

    // ---
    
    void testRootedComparison();
    void testHeapComparison();
    
    void testAutoWrappedValueVector();
};
