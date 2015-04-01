/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "TestingProxy.h"

#include "chronotext/Context.h"

using namespace std;
using namespace ci;
using namespace chr;

using namespace jsp;

void TestingProxy::performRun(bool force)
{
    JSP_TEST(force || false, testPeers1);
    JSP_TEST(force || true, testNativeCalls1);
}

// ---

void TestingProxy::testPeers1()
{
    Proxy vanilla1;
    Proxy singleton("ScriptManager", true);
    
    try
    {
        /*
         * ALLOWED
         */
        executeScript("peers.Proxy[0].bar = 'baz';");

        /*
         * INTENTIONALLY NOT ALLOWED
         */
        executeScript("peers.Proxy[1] = 255; peers.ScriptManager = 456");
        
        /*
         * SHOULD NOT BE ALLOWED AS WELL (I.E. ONLY GENERATE A "WARNING")
         *
         * TODO: FIND OUT HOW (SPOILER: NON-TRIVIAL)
         */
        executeScript("peers.foo = 123");

    }
    catch (exception &e)
    {
        LOGI << e.what() << endl;
    }
    
    LOGI << toSource(get<OBJECT>(globalHandle(), "peers")) << endl;
}

// ---

static bool staticMethod1(CallArgs args)
{
    if (args.hasDefined(0) && args[0].isNumber())
    {
        args.rval().set(NumberValue(args[0].toNumber() * -1));
        return true;
    }
    
    return false;
}

bool TestingProxy::instanceMethod1(CallArgs args)
{
    if (args.hasDefined(0) && args[0].isNumber())
    {
        args.rval().set(NumberValue(args[0].toNumber() * instanceValue1));
        return true;
    }
    
    return false;
}

void TestingProxy::testNativeCalls1()
{
    registerNativeCall(globalHandle(), "staticMethod1", BIND_STATIC1(staticMethod1));
    registerNativeCall(globalHandle(), "instanceMethod1", BIND_INSTANCE1(&TestingProxy::instanceMethod1, this));
    
    registerNativeCall(globalHandle(), "lambda1", [=](CallArgs args)->bool
    {
        if (args.hasDefined(0) && args[0].isNumber())
        {
            args.rval().set(NumberValue(args[0].toNumber() * instanceValue1));
            return true;
        }

        return false;
    });
    
    executeScript("print(staticMethod1(77), instanceMethod1(11), lambda1(33))");
}
