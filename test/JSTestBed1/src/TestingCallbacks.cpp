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

void TestingCallbacks::performRun(bool force)
{
    JSP_TEST(force || true, testMethodDispatch1);
    JSP_TEST(force || true, testMethodDispatch2);
    JSP_TEST(force || true, testMethodDispatchExtended);
    
    JSP_TEST(force || true, testInstanceMethod1);
    JSP_TEST(force || true, testInstanceMethod2);
    
    JSP_TEST(force || true, testRegistrationMacros);
    JSP_TEST(force || true, testJSSideFunctionAssign);
    
#if defined(CINDER_MAC) && defined(DEBUG)
    JSP_TEST(force || true, testDefinedFunctionRooting1);
    JSP_TEST(force || true, testDefinedFunctionRooting2);
#endif
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
    JSAtom *atom = function->atom();
    
    LOGI << &callee << " | " << toChars(atom) << endl;
    
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
    JSAtom *atom = function->atom();
    
    LOGI << &callee << " | " << toChars(atom) << " | " << JSP::write(GetFunctionNativeReserved(function, 0)) << endl;
    
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

template<class C>
void TestingCallbacks::registerCallback1(HandleObject object, const string &name, C&& callback)
{
    int32_t key = ++callbackCount1;
    callbacks1.emplace(key, bind(forward<C>(callback), this, placeholders::_1));
    
    JSFunction *function = DefineFunctionWithReserved(cx, object, name.data(), dispatchCallback1, 0, 0);
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

map<string, int32_t> TestingCallbacks::names2 {};
map<int32_t, function<bool(CallArgs args)>> TestingCallbacks::callbacks2 {};
int32_t TestingCallbacks::callbackCount2 = 0;

static bool dispatchCallback2(JSContext *cx, unsigned argc, Value *vp)
{
    auto args = CallArgsFromVp(argc, vp);
    JSObject &callee = args.callee();
    
    JSFunction *function = &callee.as<JSFunction>();
    int32_t key = GetFunctionNativeReserved(function, 0).toInt32();
    
    return TestingCallbacks::callbacks2[key](args);
}

bool TestingCallbacks::registerCallback2(HandleObject object, const string &name, const function<bool(CallArgs args)> &fn)
{
    auto found = names2.find(name);
    
    if (found == names2.end())
    {
        int32_t key = ++callbackCount2;
        callbacks2.emplace(key, fn);
        names2.emplace(name, key);
        
        JSFunction *function = DefineFunctionWithReserved(cx, object, name.data(), dispatchCallback2, 0, 0);
        SetFunctionNativeReserved(function, 0, NumberValue(key));
        
        return true;
    }
    
    return false;
}

bool TestingCallbacks::unregisterCallback2(HandleObject object, const string &name)
{
    auto found = names2.find(name);
    
    if (found != names2.end())
    {
        int32_t key = found->second;
        callbacks2.erase(key);
        names2.erase(name);
        
        return JS_DeleteProperty(cx, object, name.data());
    }
    
    return false;
}

bool TestingCallbacks::instanceMethod2(CallArgs args)
{
    if (args.hasDefined(0) && args[0].isNumber())
    {
        args.rval().set(NumberValue(args[0].toNumber() * instanceValue2));
        return true;
    }
    
    return false;
}

static bool staticMethod1(CallArgs args)
{
    if (args.hasDefined(0) && args[0].isNumber())
    {
        args.rval().set(NumberValue(args[0].toNumber() * -1));
        return true;
    }
    
    return false;
}

void TestingCallbacks::testInstanceMethod2()
{
    registerCallback2(globalHandle(), "instanceMethod2", bind(&TestingCallbacks::instanceMethod2, this, placeholders::_1));
    executeScript("print(instanceMethod2(33))");
    
    try
    {
        unregisterCallback2(globalHandle(), "instanceMethod2");
        executeScript("print(instanceMethod2(11))"); // SHOULD FAIL
    }
    catch(exception &e)
    {}
}

// ---

void TestingCallbacks::testRegistrationMacros()
{
    registerCallback2(globalHandle(), "staticMethod1", BIND_STATIC1(staticMethod1));
    executeScript("print(staticMethod1(33))");
    
    registerCallback2(globalHandle(), "instanceMethod2", BIND_INSTANCE1(&TestingCallbacks::instanceMethod2, this));
    executeScript("print(instanceMethod2(44))");
}

void TestingCallbacks::testJSSideFunctionAssign()
{
    registerCallback2(globalHandle(), "staticMethod1", BIND_STATIC1(staticMethod1));
    
    /*
     * WORKS AS INTENDED:
     *
     * 1) foo.method1 PRINTS: function staticMethod1() { [native code] }
     * 2) foo.method1(33) PRINTS: -33
     */
    executeScript("var foo = {}; foo.method1 = this.staticMethod1; print(foo.method1); print(foo.method1(33))");
}

// ---

/*
 * OSX + DEBUG ONLY: DEMONSTRATING THAT A DEFINED-FUNCTION IS ROOTED AS-LONG-AS THE HOST-OBJECT IS ALIVE
 */

void TestingCallbacks::testDefinedFunctionRooting1()
{
    JSFunction *customMethodF1 = JS_DefineFunction(cx, globalHandle(), "customMethodF1", methodDispatch, 0, 0);
    
    JSP::forceGC(); // WILL NOT AFFECT GLOBAL-OBJECT
    JSP_CHECK(JSP::isHealthy(customMethodF1)); // DEFINED-FUNCTION IS ALIVE
}

void TestingCallbacks::testDefinedFunctionRooting2()
{
    JSFunction *customMethodF2 = nullptr;
    
    {
        RootedObject object(cx, Barker::create("HOST-OBJECT"));
        customMethodF2 = JS_DefineFunction(cx, object, "customMethodF2", methodDispatch, 0, 0);
        
        JSP::forceGC(); // WILL NOT AFFECT (ROOTED) BARKER
        JSP_CHECK(JSP::isHealthy(customMethodF2)); // DEFINED-FUNCTION IS ALIVE
    }
    
    JSP::forceGC(); // WILL FINALIZE BARKER
    JSP_CHECK(!JSP::isHealthy(customMethodF2)); // DEFINED-FUNCTION IS DEAD
}
