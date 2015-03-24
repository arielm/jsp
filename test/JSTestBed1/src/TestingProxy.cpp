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

void TestingProxy::run(bool force)
{
    JSP_TEST(force || true, testCallbacks1);
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

void TestingProxy::testCallbacks1()
{
    registerCallback(globalHandle(), "staticMethod1", BIND_STATIC_CALLBACK(staticMethod1));
    registerCallback(globalHandle(), "instanceMethod1", BIND_INSTANCE_CALLBACK(&TestingProxy::instanceMethod1, this));
    
    registerCallback(globalHandle(), "lambda1", [=](CallArgs args)->bool
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
