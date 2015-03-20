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
        
        JSP_TEST(force || true, testInstanceMethod1);
        JSP_TEST(force || true, testInstanceMethod2);
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
    
    callFunction(globalHandle(), customMethodA);
    callFunction(globalHandle(), customMethodB);
}

void TestingCallbacks::testMethodDispatch2()
{
    RootedFunction customMethodC(cx, defineFunction(globalHandle(), "customMethodC", methodDispatch));
    callFunction(globalHandle(), customMethodC);
    
    defineFunction(globalHandle(), "customMethodD", methodDispatch);
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
    callFunction(globalHandle(), customMethodE);
    
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
 * TODO: SEE THE FEW TODO'S IN Proto.h (BEFORE THE DECLARATION OF registerCallback(), ETC.)
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
    registerCallback(this, globalHandle(), "instanceMethod2", &TestingCallbacks::instanceMethod2);
    executeScript("print(instanceMethod2(33))");
}
