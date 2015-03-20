/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#pragma once

#include "TestingBase.h"

#include "jsp/Proxy.h"

class TestingJSBase : public TestingBase, public jsp::Proxy
{
public:
    TestingJSBase();
    
    void setup() override;
    void shutdown() override;
    void run(bool force) override {}
    
    static bool fail(const std::string &file = "", int line = 0, const std::string &reason = "");
};

// ---

#define JSP_BEGIN(TITLE) LOGI << TITLE << std::endl;
#define JSP_END() LOGI << std::endl;
#define JSP_TEST(CONDITION, FN) if ((CONDITION)) { JSP_BEGIN(#FN); JSP::forceGC(); FN(); JSP_END(); }

#define JSP_CHECK(CONDITION, ...) (CONDITION) ? true : TestingJSBase::fail(__FILE__, __LINE__, ##__VA_ARGS__)
