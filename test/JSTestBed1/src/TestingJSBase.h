/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#pragma once

#include "TestingBase.h"

#include "jsp/WrappedObject.h"
#include "jsp/WrappedValue.h"
#include "jsp/CloneBuffer.h"
#include "jsp/Barker.h"
#include "jsp/Proxy.h"

class TestingJSBase : public TestingBase, public jsp::Proxy
{
public:
    void setup() final;
    void shutdown() final;
    void run(bool force) final;

    virtual void performSetup() {}
    virtual void performShutdown() {}
    virtual void performRun(bool force) = 0;

    bool evaluateBoolean(const std::string &source);
    std::string evaluateString(const std::string &source);
    
    std::string write(const JS::HandleValueArray& args);

    /*
     * SOURCE: http://en.cppreference.com/w/cpp/container/vector/vector
     */
    template<typename T>
    static const std::string write(const std::vector<T> &v)
    {
        char comma[3] = {'\0', ' ', '\0'};
        
        std::stringstream s;
        s.put('[');
        
        for (const auto &e : v)
        {
            s << comma << e;
            comma[0] = ',';
        }
        
        s.put(']');
        return s.str();
    }
    
    static bool fail(const std::string &file = "", int line = 0, const std::string &reason = "");
};

// ---

#define JSP_BEGIN(TITLE) LOGI << std::endl << TITLE << std::endl;
#define JSP_END() LOGI << std::endl;
#define JSP_TEST(CONDITION, FN) if ((CONDITION)) { JSP_BEGIN(#FN); JSP::forceGC(); FN(); JSP_END(); }

#define JSP_CHECK(CONDITION, ...) (CONDITION) ? true : TestingJSBase::fail(__FILE__, __LINE__, ##__VA_ARGS__)
