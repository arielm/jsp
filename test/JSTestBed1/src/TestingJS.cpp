/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "TestingJS.h"
#include "Context.h"

using namespace std;
using namespace ci;
using namespace chr;

using namespace context;
using namespace jsp;

void TestingJS::run(bool force)
{
    executeScript(InputSource::getAsset("helpers.js"));
    executeScript(InputSource::getAsset("test.js"));

    if (false)
    {
        JSP::dumpObject(globalHandle());
    }
    
    if (true)
    {
        callFunction(globalHandle(), "start");
        LOGI << JSP::writeDetailed(get<OBJECT>(globalHandle(), "foo")) << endl;
    }

    if (true)
    {
        executeScript("print(1, 'אריאל מלכא', {x: 5, y: null})");
        executeScript("error tester", __FILE__, __LINE__);
    }
    
    // ---
    
    if (false)
    {
        testStringify();
        testToSource();
    }
    
    if (false)
    {
        testThreadSafety();
    }

    if (false)
    {
        testEvaluationScope();
        testFunctionScope();
    }

    if (false)
    {
        testCustomScriptExecution();
    }
    
    if (false)
    {
        testShapes1();
        testShapes2();
        testAtoms();
        testReservedSlots();
        testJSID();
        testGetProperty();
        testGetElement();
    }
}

#pragma mark ---------------------------------------- SHAPES ----------------------------------------

/*
 * MUST READ:
 * https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/Internals/Property_cache
 *
 * MORE:
 * https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/vm/Shape.h#L37-100
 * https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/vm/Shape.h#L469-514
 *
 */

void TestingJS::testShapes1()
{
    JSObject *object = evaluateObject("({one: 1, two: 'deux', trois: [1, 2, 3]})");
    
    RootedObject obj(cx, object);
    AutoIdVector idv(cx);
    
    if (GetPropertyNames(cx, obj, JSITER_OWNONLY, &idv))
    {
        RootedId id(cx);
        RootedObject obj2(cx); // MOST OF THE TIME EQUAL TO obj (UNLESS SOME "PROXYING" IS TAKING PLACE)
        js::RootedShape shape(cx);
        
        for (size_t i = 0; i < idv.length(); ++i)
        {
            id = idv[i];
            
            if (JSObject::lookupGeneric(cx, obj, id, &obj2, &shape))
            {
                JS_ASSERT(obj2->isNative()); // WILL IT WORK FOR "NEW OBJECT PROTOTYPES" DEFINED ON THE JS SIDE? (TODO: CHECK)
                
                if (shape->hasSlot())
                {
                    RootedValue vp(cx);
                    vp.set(obj2->nativeGetSlot(shape->slot())); // PROBLABLY THE BRIDGE TO ("NATURALLY" NON-ACCESSIBLE) "RESERVED SLOTS" FROM THE JS SIDE
                    
                    LOGI << JSP::write(id) << ": " << JSP::write(vp) << endl;
                }
            }
        }
    }
    
    LOGI << endl;
}

void TestingJS::testShapes2()
{
    JSObject *object1 = evaluateObject("({one: 1, two: 'deux', trois: [1, 2, 3]})");
    JSP::dumpObject(object1);
    LOGI << "-----" << endl;
    
    JSObject *object2 = evaluateObject("({one: 250, two: 'dos', trois: [0]})");
    JSP::dumpObject(object2);
    LOGI << "-----" << endl;
}

#pragma mark ---------------------------------------- ATOMS ----------------------------------------

/*
 * INVESTIGATING ATOMS:
 * - ATOMS ARE UNIQUE AND IMMUTABLE JS STRINGS STORED INTERNALLY BY SPIDERMONKEY
 *   - EFFICIENT "MAPPING" CAPABILITIES (LOOKUP, STORAGE, ETC.) ARE PROVIDED BY THE SYSTEM
 * - EACH JS OBJECT PROPERTY-NAME IS ASSOCIATED WITH AN ATOM (I.E. THE "SLOT" OF THE PROPERTY HAVE AN "ATOM JSID")
 * - THE CONCEPT OF "SYMBOL" IN ECMA 6 SEEMS RELATED TO ATOMS:
 *   https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Symbol
 *
 *
 * WISDOM ACQUIRED WHILE WORKING ON ToSourceTraverser.cpp:
 * - THE "IDENTIFIER MECHANISM" CAN BE USED TO DIFFERENTE BETWEEN ATOMS LIKE property1 AND property-two
 *   - THE LATTER IS NOT "COMPILABLE" (UNLESS QUOTED), AND THEREFORE IS NOT CONSIDERED AS AN "IDENTIFIER"
 *   - SEE "bool IsIdentifier(JSLinearString *str)" IN "frontend/BytecodeCompiler.h"
 */

void TestingJS::testAtoms()
{
    createObject1();
    createObject2();
    
    string property1 = "property2";
    JSAtom *atom1 = Atomize(cx, property1.data(), property1.size());
    
    string property2 = "property-two";
    JSAtom *atom2 = Atomize(cx, property2.data(), property2.size());
    
    /*
     * DEMONSTRATING THAT AN ATOM IS A JS STRING
     */
    JSP::dumpAtom(atom1);
    JSP::dumpAtom(atom2);
    
    // ---
    
    jsid id1 = js::AtomToId(atom1);
    jsid id2 = js::AtomToId(atom2);
    
    /*
     * DEMONSTRATING THAT A JSID CAN BE AN ATOM
     */
    LOGI << JSP::write(id1) << endl;
    LOGI << JSP::write(id2) << endl;
}

#pragma mark ---------------------------------------- RESERVED SLOTS ----------------------------------------

JSClass TestClass1
{
    "TestClass1",
    JSCLASS_HAS_RESERVED_SLOTS(1),
    JS_PropertyStub,
    JS_DeletePropertyStub,
    JS_PropertyStub,
    JS_StrictPropertyStub,
    JS_EnumerateStub,
    JS_ResolveStub,
    JS_ConvertStub
};

/*
 * INVESTIGATING RESERVED SLOTS:
 * - https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/JSAPI_reference/JS_GetReservedSlot
 * - http://mxr.mozilla.org/mozilla-central/ident?i=JSCLASS_HAS_RESERVED_SLOTS
 *
 *
 * FINDINGS: RESERVED SLOTS HAVE NO JSID
 *
 * PROS: PROBABLY THE FASTEST WAY TO ACCESS A JS OBJECT'S VAUE
 * CONS: NOT "NATURALLY" ACCESSIBLE FROM JS - UPDATE: SEE JSObjet::nativeGetSlot() USED IN testShapes1()
 *
 * FOLLOW-UP:
 * - THERE MIGHT BE A BETTER ALTERNATIVE TO PREDEFINED-SLOTS:
 *   - SIMPLY USING NATIVE GETTERS/SETTERS, E.G. AS FOLLOWS:
 *     - https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/JSAPI_Cookbook#Defining_a_property_with_a_getter_and_setter
 *     - https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/JSAPI_Cookbook#Defining_a_constant_property
 *   - BASICALLY:
 *     - OUR NATIVE "HANDLER" COULD STORE PROPERTIES IN ANY POSSIBLE WAY
 *     - WE'RE NOT BOUND TO RESERVED-SLOTS ANYMORE
 *     - HOWEVER:
 *       - IF WE STORE THE VALUE AS NATIVE PRIMITIVES IN C++ MEMORY
 *         - WE MUST BE SURE TO RELEASE THE MEMORY WHEN OUR JS OBJECT IS GARBAGE-COLLECTED
 */

void TestingJS::testReservedSlots()
{
    RootedObject object(cx, JS_NewObject(cx, &TestClass1, NullPtr(), NullPtr()));
    
    JS_SetReservedSlot(object, 0, DoubleValue(33.33));
    
    RootedValue value2(cx, NumberValue(999));
    JS_SetProperty(cx, object, "property2", value2);
    
    /*
     * DEMONSTRATES THAT THE RESERVED SLOT HAVE NO JSID
     */
    JSP::dumpObject(object);
    
    /*
     * DEMONSTRATES THAT THE RESERVED SLOT IS NOT (AUTOMATICALLY) ACCESSIBLE VIA JS
     */
    LOGI << toSource(object) << endl;
}

#pragma mark ---------------------------------------- JSID ----------------------------------------

/*
 * INVESTIGATING JSID:
 * - https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/JSAPI_reference/jsid
 *
 *
 * FINDINGS:
 * - EACH PROPERTY IN A JS-OBJECT IS ASSOCIATED WITH A "SLOT"
 *   - EACH SLOT CAN BE ASSOCIATED WITH A JSID
 * - A JSID CAN BE OF THE FOLLOWING TYPES:
 *   - STRING (OR ATOM) (USED FOR THE STRING-PROPERTIES OF AN OBJECT)
 *   - INT (USED FOR THE NUMERIC-PROPERTIES OF AN OBJECT, E.G. ARRAY INDICES)
 *   - OBJECT
 *   - VOID
 *
 * THIS TEST DEMONSTRATES THAT SPIDERMONKEY IS CACHING PROPERTY-NAMES SO THAT
 * A PROPERTY WITH THE SAME NAME IN TWO DIFFERENT JS OBJECT REFERS TO THE SAME JSID
 */

void TestingJS::testJSID()
{
    /*
     * DEMONSTRATES THAT PROPERTY-NAMES ARE CACHED:
     * A PROPERTY WITH THE SAME NAME IN TWO DIFFERENT JS OBJECTS WILL REFER TO THE SAME "ATOM JSID"
     */
    dumpIds(createObject1());
    dumpIds(createObject2());
    
    // ---
    
    RootedId id1(cx);
    if (!IndexToId(cx, 1, &id1)) throw;
    
    RootedId id2(cx);
    if (!IndexToId(cx, 2, &id2)) throw;
    
    /*
     * DEMONSTRATES HOW AN "INT JSID" IS GENERATED, USING THE FORMULA DEFINED IN "public/Id.h"
     * ((i << 1) | JSID_TYPE_INT);
     */
    LOGI << JSP::write(id1) << endl; // PRINTS: jsid 0x3 = 1
    LOGI << JSP::write(id2) << endl; // PRINTS: jsid 0x5 = 2
}

#pragma mark ---------------------------------------- JS_GetProperty AND JS_GetElement ----------------------------------------

/*
 * HOW SPIDERMONKEY IS FETCHING A VALUE FROM A JS OBJECT, STEP-BY-STEP:
 *
 *
 * bool JS_GetProperty(JSContext *cx, JSObject *objArg, const char *name, MutableHandleValue vp)
 * https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/jsapi.cpp#L3408-3414
 *
 * JSAtom* js::Atomize(ExclusiveContext *cx, const char *bytes, size_t length, InternBehavior ib)
 * https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/jsatom.cpp#L354-380
 *
 * jsid AtomToId(JSAtom *atom)
 * https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/jsatominlines.h#L31-41
 *
 * bool JS_GetPropertyById(JSContext *cx, JSObject *objArg, jsid idArg, MutableHandleValue vp)
 * https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/jsapi.cpp#L3346-3350
 *
 * bool JS_ForwardGetPropertyTo(JSContext *cx, JSObject *objArg, jsid idArg, JSObject *onBehalfOfArg, MutableHandleValue vp)
 * https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/jsapi.cpp#L3352-3367
 *
 *   IN COMMON WITH JS_GetElement:
 *
 *   bool getGeneric(JSContext *cx, js::HandleObject obj, js::HandleObject receiver, js::HandleId id, js::MutableHandleValue vp)
 *   https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/jsobj.h#L996-1009
 *
 *   bool baseops::GetProperty(JSContext *cx, HandleObject obj, HandleObject receiver, HandleId id, MutableHandleValue vp)
 *   https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/jsobj.cpp#L4277-4282
 *
 *   bool GetPropertyHelperInline(JSContext *cx,
 *        typename MaybeRooted<JSObject*, allowGC>::HandleType obj,
 *        typename MaybeRooted<JSObject*, allowGC>::HandleType receiver,
 *        typename MaybeRooted<jsid, allowGC>::HandleType id,
 *        typename MaybeRooted<Value, allowGC>::MutableHandleType vp)
 *   https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/jsobj.cpp#L4172-4275
 *
 *   bool LookupPropertyWithFlagsInline(ExclusiveContext *cx,
 *        typename MaybeRooted<JSObject*, allowGC>::HandleType obj,
 *        typename MaybeRooted<jsid, allowGC>::HandleType id,
 *        unsigned flags,
 *        typename MaybeRooted<JSObject*, allowGC>::MutableHandleType objp,
 *        typename MaybeRooted<Shape*, allowGC>::MutableHandleType propp)
 *   https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/jsobj.cpp#L3802-3846
 *
 *   bool LookupOwnPropertyWithFlagsInline(ExclusiveContext *cx,
 *        typename MaybeRooted<JSObject*, allowGC>::HandleType obj,
 *        typename MaybeRooted<jsid, allowGC>::HandleType id,
 *        unsigned flags,
 *        typename MaybeRooted<JSObject*, allowGC>::MutableHandleType objp,
 *        typename MaybeRooted<Shape*, allowGC>::MutableHandleType propp,
 *        bool *donep)
 *   https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/jsobj.cpp#L3728-3786
 *
 * Shape* js::ObjectImpl::nativeLookup(ExclusiveContext *cx, jsid id)
 * https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/vm/ObjectImpl.cpp#L318-324
 *
 * bool NativeGetInline(JSContext *cx,
 *      typename MaybeRooted<JSObject*, allowGC>::HandleType obj,
 *      typename MaybeRooted<JSObject*, allowGC>::HandleType receiver,
 *      typename MaybeRooted<JSObject*, allowGC>::HandleType pobj,
 *      typename MaybeRooted<Shape*, allowGC>::HandleType shape,
 *      typename MaybeRooted<Value, allowGC>::MutableHandleType vp)
 * https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/jsobj.cpp#L4037-4096
 *
 * bool hasSlot() const
 * https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/vm/Shape.h#L1161
 *
 * const Value &nativeGetSlot(uint32_t slot) const
 * https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/vm/ObjectImpl.h#L1353-1356
 */

void TestingJS::testGetProperty()
{
    RootedObject object(cx, createObject2());
    RootedValue value(cx);
    
    if (JS_GetProperty(cx, object, "property1", &value))
    {
        LOGI << JSP::writeDetailed(value) << endl;
    }
}

/*
 * HOW SPIDERMONKEY IS FETCHING A VALUE FROM A JS ARRAY, STEP-BY-STEP:
 *
 *
 * bool JS_GetElement(JSContext *cx, JSObject *objArg, uint32_t index, MutableHandleValue vp)
 * https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/jsapi.cpp#L3369-L3373
 *
 * bool JS_GetElementIfPresent(JSContext *cx, JSObject *objArg, uint32_t index, JSObject *onBehalfOfArg, MutableHandleValue vp, bool* present)
 * https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/jsapi.cpp#L3375-3387
 *
 * bool JSObject::getElement(JSContext *cx, js::HandleObject obj, js::HandleObject receiver, uint32_t index, js::MutableHandleValue vp)
 * https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/jsobjinlines.h#L558-570
 *
 *   IN COMMON WITH JS_GetProperty:
 *
 *   bool getGeneric(JSContext *cx, js::HandleObject obj, js::HandleObject receiver, js::HandleId id, js::MutableHandleValue vp)
 *   https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/jsobj.h#L996-1009
 *
 *   bool baseops::GetProperty(JSContext *cx, HandleObject obj, HandleObject receiver, HandleId id, MutableHandleValue vp)
 *   https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/jsobj.cpp#L4277-4282
 *
 *   bool GetPropertyHelperInline(JSContext *cx,
 *        typename MaybeRooted<JSObject*, allowGC>::HandleType obj,
 *        typename MaybeRooted<JSObject*, allowGC>::HandleType receiver,
 *        typename MaybeRooted<jsid, allowGC>::HandleType id,
 *        typename MaybeRooted<Value, allowGC>::MutableHandleType vp)
 *   https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/jsobj.cpp#L4172-4275
 *
 *   bool LookupPropertyWithFlagsInline(ExclusiveContext *cx,
 *        typename MaybeRooted<JSObject*, allowGC>::HandleType obj,
 *        typename MaybeRooted<jsid, allowGC>::HandleType id,
 *        unsigned flags,
 *        typename MaybeRooted<JSObject*, allowGC>::MutableHandleType objp,
 *        typename MaybeRooted<Shape*, allowGC>::MutableHandleType propp)
 *   https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/jsobj.cpp#L3802-3846
 *
 *   bool LookupOwnPropertyWithFlagsInline(ExclusiveContext *cx,
 *        typename MaybeRooted<JSObject*, allowGC>::HandleType obj,
 *        typename MaybeRooted<jsid, allowGC>::HandleType id,
 *        unsigned flags,
 *        typename MaybeRooted<JSObject*, allowGC>::MutableHandleType objp,
 *        typename MaybeRooted<Shape*, allowGC>::MutableHandleType propp,
 *        bool *donep)
 *   https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/jsobj.cpp#L3728-3786
 *
 * bool containsDenseElement(uint32_t idx)
 * https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/vm/ObjectImpl.h#L1023-1026
 *
 * bool IsImplicitDenseElement(Shape *prop)
 * https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/vm/Shape.h#L1675-1679
 *
 * const Value &getDenseElement(uint32_t idx)
 * https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/vm/ObjectImpl.h#L1018-1022
 */

void TestingJS::testGetElement()
{
    RootedObject array(cx, createArray());
    RootedValue value(cx);
    
    if (JS_GetElement(cx, array, 2, &value))
    {
        LOGI << JSP::writeDetailed(value) << endl;
    }
}

#pragma mark ---------------------------------------- MISC ----------------------------------------

void TestingJS::dumpIds(JSObject *object)
{
    AutoIdVector idv(cx);
    
    if (GetPropertyNames(cx, object, JSITER_OWNONLY, &idv))
    {
        for (size_t i = 0; i < idv.length(); ++i)
        {
            LOGI << JSP::write(idv[i]) << endl;
        }
    }
}

JSObject* TestingJS::createObject1()
{
    RootedObject object(cx, newObject());
    
    RootedValue value1(cx, NumberValue(33.33));
    JS_SetProperty(cx, object, "property1", value1);
    
    RootedValue value2(cx, NumberValue(66.67));
    JS_SetProperty(cx, object, "property-two", value2);
    
    return object;
}

JSObject* TestingJS::createObject2()
{
    return evaluateObject("({'property-two': 456, property1: 123})");
}

JSObject* TestingJS::createArray()
{
    return evaluateObject("(['e0', 'e1', 'e2'])");
}

#pragma mark ---------------------------------------- THREAD SAFETY ----------------------------------------

class TestTask : public Task
{
    TestingJS *target;
    
public:
    TestTask(TestingJS *target) : target(target) {}
    
    void run()
    {
        sleep(1000);
        
        /*
         * SUPPOSED TO CAUSE AN ASSERTION-FAILURE WHEN SPIDERMONKEY IS BUILT WITH "JS_THREADSAFE"
         *
         * TODO:
         * - FIND-OUT WHY IT CRASHES ON SPIDERMONKEY 31
         *   I.E. INSTEAD OF ALWAYS CAUSING AN ASSERTION-FAILURE
         *   E.G. RELATED TO CurrentThreadCanAccessRuntime(cx->runtime())
         */
        
        target->testAtoms(); // FORBIDDEN: A JS-CONTEXT MUST BE ACCESSED FROM THE THREAD THAT CREATED IT
    }
};

void TestingJS::testThreadSafety()
{
    taskManager().addTask(make_shared<TestTask>(this));
}

#pragma mark ---------------------------------------- MISC ----------------------------------------

/*
 * CONCLUSIONS:
 *
 * SCOPE:
 * JS::Evaluate (USED BY Manager::evaluateObject) WILL FAIL IF NO obj PARAMETER IS PROVIDED
 * I.E. WE SHOULD CONTINUE USING THE GLOBAL-OBJECT BY DEFAULT
 *
 * FOLLOW-UP:
 * - MORE INFO TO DIGEST (TODO): https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/jsapi.h#L3671-3703
 */

void TestingJS::testEvaluationScope()
{
    JSObject *newObject = evaluateObject("({x: 250.33, foo: [1, 2, 3], bar: 'baz'})");
    JSP::dumpObject(newObject);
}

/*
 * CONCLUSIONS:
 *
 * SCOPE:
 * SEE COMMENTS REGARDING HOW JS_CallFunctionValue (USED BY Manager::callFunction)
 * WILL BEHAVE DEPENDING ON THE obj PARAMETER
 */

void TestingJS::testFunctionScope()
{
    /*
     * GLOBAL SCOPE
     */
    executeScript("var someAmbiguousThing = 2, someGlobalThing = 0");
    
    string source = "({\
    someAmbiguousThing: 3,\
    function1: function(parameter) { return parameter * someGlobalThing; },\
    function2: function(parameter) { return parameter * this.someAmbiguousThing; }\
    })";
    
    RootedObject containerObject(cx, evaluateObject(source));
    JSP::dumpObject(containerObject);
    
    // ---
    
    AutoValueArray<1> args(cx);
    args[0].setNumber(33.0);
    
    RootedValue function1(cx);
    RootedValue function2(cx);
    
    if (getProperty(containerObject, "function1", &function1))
    {
        /*
         * IF containerObject, OR NullPtr(), OR NOTHING (I.E. THE GLOBAL-OBJECT) IS PROVIDED:
         * someGlobalThing WILL BE PROPERLY RESOLVED
         */
        JS::RootedValue result1(cx, callFunction(NullPtr(), function1, args));
        LOGI << "RESULT 1: " << JSP::write(result1) << endl; // RESULT: 0
    }
    
    if (getProperty(containerObject, "function2", &function2))
    {
        /*
         * IF containerObject IS PROVIDED:
         * this.someAmbiguousThing WILL RESOLVE TO THE "LOCAL VALUE"
         */
        RootedValue result2A(cx, callFunction(containerObject, function2, args));
        LOGI << "RESULT 2A: " << JSP::write(result2A) << endl; // RESULT: 99
        
        /*
         * IF nullptr OR NOTHING (I.E. THE GLOBAL-OBJECT) IS PROVIDED:
         * this.someAmbiguousThing WILL RESOLVE TO THE "GLOBAL VALUE"
         */
        RootedValue result2B(cx, callFunction(NullPtr(), function2, args));
        LOGI << "RESULT 2B: " << JSP::write(result2B) << endl; // RESULT: 66
    }
}

void TestingJS::testCustomScriptExecution()
{
    auto inputSource = InputSource::getFileInDocuments("temp.js");
    string source = utils::readText<string>(inputSource);
    string filePathHint = inputSource->getFilePathHint();
    
    OwningCompileOptions options(cx);
    options.setNoScriptRval(true); // TODO: SHOULD BE FORCED, INSIDE Proto::exec
    options.setVersion(JSVersion::JSVERSION_ECMA_5);
    options.setUTF8(false);
    options.setFileAndLine(cx, filePathHint.data(), 1);
    
    exec(source, options);
}

/*
 * WORKS BECAUSE THERE ARE NO "CYCLIC VALUES" IN OBJECT
 * OTHERWISE: RETURN A STRING WITH "cyclic object value"
 */
void TestingJS::testStringify()
{
//  initComplexJSObject(InputSource::getAsset("complexObject.bin")); // TODO: CREATE complexObject.bin VIA CloneBuffer::write()
    
    /*
     * USING stringify VIA JS:
     * POSSIBILITY TO HANDLE "CYCLIC VALUES" VIA THE "REPLACER" ARGUMENT
     */
    executeScript("print(JSON.stringify(complexObject, null, 2))");
    
    /*
     * USING stringify VIA C++:
     * CURRENTLY NO WAY TO DEFINE A "REPLACER" FUNCTION (TODO)
     */
    JSObject *object = get<OBJECT>(globalHandle(), "complexObject");
    LOGI << stringify(object) << endl;
}

/*
 * WORKS BECAUSE OBJECT IS NOT "HUGE"
 * OTHERWISE: OUT-OF-MEMORY JS-ERROR
 */
void TestingJS::testToSource()
{
//  initComplexJSObject(InputSource::getAsset("complexObject.bin")); // TODO: CREATE complexObject.bin VIA CloneBuffer::write()
    
    /*
     * USING toSource() VIA JS
     */
    executeScript("print(complexObject)");
    
    /*
     * USING toSource() VIA C++
     */
    JSObject *object = get<OBJECT>(globalHandle(), "complexObject");
    LOGI << toSource(object) << endl;
}

void TestingJS::initComplexJSObject(InputSource::Ref inputSource)
{
    try
    {
        if (hasProperty(globalHandle(), "complexObject"))
        {
            deleteProperty(globalHandle(), "complexObject");
        }
        
        JSObject *complexObject = CloneBuffer::read(inputSource->loadDataSource());
        set(globalHandle(), "complexObject", complexObject);
    }
    catch (exception &e)
    {
        LOGI << e.what() << endl;
    }
}
