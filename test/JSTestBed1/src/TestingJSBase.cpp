/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "TestingJSBase.h"

using namespace std;
using namespace ci;
using namespace chr;

using namespace jsp;

void TestingJSBase::setup()
{
    JSP::forceGC();
    performSetup();
}

void TestingJSBase::shutdown()
{
    performShutdown();
    JSP::forceGC();
}

void TestingJSBase::run(bool force)
{
    performRun(force);
}

// ---

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
