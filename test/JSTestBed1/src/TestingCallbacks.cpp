/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "TestingCallbacks.h"

#include "chronotext/Context.h"

using namespace std;
using namespace ci;
using namespace chr;

using namespace jsp;

void TestingCallbacks::setup()
{
    WrappedValue::LOG_VERBOSE = true;
    WrappedObject::LOG_VERBOSE = true;
}

void TestingCallbacks::shutdown()
{
    WrappedValue::LOG_VERBOSE = false;
    WrappedObject::LOG_VERBOSE = false;
}

void TestingCallbacks::run(bool force)
{
    if (force || true)
    {
        JSP_TEST(force || false, testMethodDispatch1);
        JSP_TEST(force || false, testMethodDispatch2);
        JSP_TEST(force || false, testMethodDispatchExtended);
        
        JSP_TEST(force || false, testInstanceMethod1);
        JSP_TEST(force || false, testInstanceMethod2);
        
#if defined(CINDER_MAC) && defined(DEBUG)
        JSP_TEST(force || false, testDefinedFunctionRooting1);
        JSP_TEST(force || false, testDefinedFunctionRooting2);
#endif
        
        JSP_TEST(force || true, testRegistrationMacro);
    }
}

// ---

/*
 * REFERENCE: https://github.com/arielm/Spidermonkey/blob/chr_31/js/src/jsapi-tests/testCallNonGenericMethodOnProxy.cpp
 */

static bool methodDispatch(JSContext *cx, unsigned argc, Value *vp)
{
    auto args = CallArgsFromVp(argc, vp);
    
    JSObject &callee = args.callee();
    JSFunction *function = &callee.as<JSFunction>();
    
    LOGI << &callee << " | " << toString(function->atom()->chars()) << endl;
    
    args.rval().setUndefined();
    return true;
}

/*
 * DISADVANTAGE OF testMethodDispatch1 COMPARED TO testMethodDispatch2:
 * customMethodA and customMethodB CAN'T BE INVOKED FROM THE JS SIDE
 */

void TestingCallbacks::testMethodDispatch1()
{
    RootedFunction customMethodA(cx, JS_NewFunction(cx, methodDispatch, 0, 0, globalHandle(), "customMethodA"));
    RootedFunction customMethodB(cx, JS_NewFunction(cx, methodDispatch, 0, 0, globalHandle(), "customMethodB"));
    
    call(globalHandle(), customMethodA);
    call(globalHandle(), customMethodB);
}

void TestingCallbacks::testMethodDispatch2()
{
    RootedFunction customMethodC(cx, JS_DefineFunction(cx, globalHandle(), "customMethodC", methodDispatch, 0, 0));
    call(globalHandle(), customMethodC);
    
    JS_DefineFunction(cx, globalHandle(), "customMethodD", methodDispatch, 0, 0);
    executeScript("customMethodD();");
}

// ---

/*
 * USING "EXTENDED SLOTS" IN ORDER TO BIND BETWEEN A JSFunction AND SOME UNIQUE-ID
 *
 * REFERENCES:
 *
 * https://github.com/arielm/Spidermonkey/blob/chr_31/js/src/jsfriendapi.cpp#L468-481
 * https://github.com/arielm/Spidermonkey/blob/chr_31/js/src/jsfriendapi.cpp#L536-549
 * https://github.com/arielm/Spidermonkey/blob/chr_31/js/src/jsfun.h#L465-473
 */

static bool methodDispatchExtended(JSContext *cx, unsigned argc, Value *vp)
{
    auto args = CallArgsFromVp(argc, vp);
    
    JSObject &callee = args.callee();
    JSFunction *function = &callee.as<JSFunction>();
    
    LOGI << &callee << " | " << toString(function->atom()->chars()) << " | " << JSP::write(GetFunctionNativeReserved(function, 0)) << endl;
    
    args.rval().setUndefined();
    return true;
}

void TestingCallbacks::testMethodDispatchExtended()
{
    RootedFunction customMethodE(cx, DefineFunctionWithReserved(cx, globalHandle(), "customMethodE", methodDispatchExtended, 0, 0));
    
    SetFunctionNativeReserved(customMethodE, 0, NumberValue(12345));
    call(globalHandle(), customMethodE);
    
    SetFunctionNativeReserved(customMethodE, 0, NumberValue(999));
    executeScript("customMethodE();");
}

// ---

/*
 * TODO: EVALUATE JS <-> C++ BINDING SOLUTIONS
 *
 * - http://kripken.github.io/emscripten-site/docs/porting/connecting_cpp_and_javascript/WebIDL-Binder.html#webidl-binder
 * - http://kripken.github.io/emscripten-site/docs/porting/connecting_cpp_and_javascript/embind.html#embind
 * - https://github.com/arielm/Spidermonkey/blob/chr_31/js/src/ctypes/libffi/README
 */

map<int32_t, function<bool(CallArgs args)>> TestingCallbacks::callbacks1 {};
int32_t TestingCallbacks::callbackCount1 = 0;

static bool dispatchCallback1(JSContext *cx, unsigned argc, Value *vp)
{
    auto args = CallArgsFromVp(argc, vp);
    JSObject &callee = args.callee();
    
    JSFunction *function = &callee.as<JSFunction>();
    int32_t key = GetFunctionNativeReserved(function, 0).toInt32();
    
    return TestingCallbacks::callbacks1[key](args);
}

template<class F>
void TestingCallbacks::registerCallback1(HandleObject object, const string &name, F&& f)
{
    int32_t key = ++callbackCount1;
    callbacks1.emplace(key, bind(forward<F>(f), this, placeholders::_1));
    
    RootedFunction function(cx, DefineFunctionWithReserved(cx, object, name.data(), dispatchCallback1, 0, 0));
    SetFunctionNativeReserved(function, 0, NumberValue(key));
}

bool TestingCallbacks::instanceMethod1(CallArgs args)
{
    if (args.hasDefined(0) && args[0].isNumber())
    {
        args.rval().set(NumberValue(args[0].toNumber() * instanceValue1));
        return true;
    }
    
    return false;
}

void TestingCallbacks::testInstanceMethod1()
{
    registerCallback1(globalHandle(), "instanceMethod1", &TestingCallbacks::instanceMethod1);
    executeScript("print(instanceMethod1(33))");
}

// ---

/*
 * TODO: SEE TODO-LIST IN Proxy.h (BEFORE THE DECLARATION OF registerCallback(), ETC.)
 */

bool TestingCallbacks::instanceMethod2(CallArgs args)
{
    if (args.hasDefined(0) && args[0].isNumber())
    {
        args.rval().set(NumberValue(args[0].toNumber() * instanceValue2));
        return true;
    }
    
    return false;
}

void TestingCallbacks::testInstanceMethod2()
{
//  registerCallback(globalHandle(), "instanceMethod2", &TestingCallbacks::instanceMethod2, this);
//  executeScript("print(instanceMethod2(33))");
}

// ---

/*
 * DEMONSTRATING THAT A DEFINED-FUNCTION IS ROOTED AS-LONG-AS THE HOST-OBJECT IS ALIVE
 * LIMITATION: FOR OSX + DEBUG ONLY
 *
 * TODO: FIND-OUT WHY JSP::isHealthy(JSFunction*) IS NOT WORKING
 */

void TestingCallbacks::testDefinedFunctionRooting1()
{
    JSFunction *customMethodF1 = JS_DefineFunction(cx, globalHandle(), "customMethodF1", methodDispatch, 0, 0);
    
    JSP::forceGC(); // WILL NOT AFFECT GLOBAL-OBJECT
    JSP_CHECK(boost::ends_with(JSP::writeDetailed(customMethodF1), "[B]")); // DEFINED-FUNCTION IS ALIVE
}

void TestingCallbacks::testDefinedFunctionRooting2()
{
    JSFunction *customMethodF2 = nullptr;
    
    {
        RootedObject object(cx, Barker::construct("HOST-OBJECT"));
        customMethodF2 = JS_DefineFunction(cx, object, "customMethodF2", methodDispatch, 0, 0);
        
        Barker::forceGC(); // WILL NOT AFFECT (ROOTED) BARKER
        JSP_CHECK(boost::ends_with(JSP::writeDetailed(customMethodF2), "[B]")); // DEFINED-FUNCTION IS ALIVE
    }
    
    Barker::forceGC(); // WILL FINALIZE BARKER
    JSP_CHECK(boost::ends_with(JSP::writeDetailed(customMethodF2), "[P]")); // DEFINED-FUNCTION IS DEAD
}

// ---

void TestingCallbacks::testRegistrationMacro()
{
    REGISTER_CALLBACK(globalHandle(), TestingCallbacks, instanceMethod2);
    executeScript("print(instanceMethod2(44))");
}
