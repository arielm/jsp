/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "TestingJSBase.h"
#include "Context.h"

using namespace std;
using namespace ci;
using namespace chr;

using namespace context;
using namespace jsp;

TestingJSBase::TestingJSBase()
:
Proxy(jsProto())
{}

void TestingJSBase::setup()
{
    forceGC();
}

void TestingJSBase::shutdown()
{
    forceGC();
}

void TestingJSBase::forceGC()
{
    JSP::forceGC(rt);
}

void TestingJSBase::beginGCZeal(bool gcBefore)
{
    if (gcBefore)
    {
        JSP::forceGC(rt);
    }
    
    /*
     * WARNING: BOTH SETTINGS ARE PROBLEMATIC WHEN TESTING GC AND TRACING!!!
     *
     * E.G. THEY CAN CAUSE OBJECTS TO APPEAR CREATED "DIRECTLY ON THE TENURED-HEAP", OR "DIRECTLY AS BLACK"...
     */
    
    if (true)
    {
        JSP::setGCZeal(cx, 11); // QUOTING /js/src/gc/jsgc.cpp: "Verify post write barriers between instructions"
    }
    else
    {
        JSP::setGCZeal(cx, 2, 1); // QUOTING /js/src/gc/jsgc.cpp: "GC every F allocations"
    }
}

void TestingJSBase::endGCZeal(bool gcAfter)
{
    JSP::setGCZeal(cx, 0);
    
    if (gcAfter)
    {
        JSP::forceGC(rt);
    }
}

bool TestingJSBase::fail(const string &file, int line, const string &reason)
{
    LOGI << "FAILURE";
    
    if (!reason.empty())
    {
        LOGI << ": " << reason;
    }
    
    if (!file.empty())
    {
        LOGI << " [" << file; // TODO: POSSIBILITY TO EXCLUDE FULL-PATH (RELATIVE TO SOME DEFINABLE "ROOT PATH")
        
        if (line > 0)
        {
            LOGI << " | LINE: " << line;
        }
        
        LOGI << "]";
    }
    
    LOGI << endl;
    
    return false;
}
