/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "TestingRooting2.h"

#include "chronotext/Context.h"

using namespace std;
using namespace ci;
using namespace chr;

using namespace jsp;

void TestingRooting2::performSetup()
{
    WrappedValue::LOG_VERBOSE = true;
    WrappedObject::LOG_VERBOSE = true;
}

void TestingRooting2::performShutdown()
{
    WrappedValue::LOG_VERBOSE = false;
    WrappedObject::LOG_VERBOSE = false;
}

void TestingRooting2::performRun(bool force)
{
    if (force || true)
    {
        JSP_TEST(force || true, testAnalysis1)
        JSP_TEST(force || true, testAnalysis2)
        JSP_TEST(force || true, testAnalysis3)
    }
    
    if (force || true)
    {
        JSP_TEST(force || true, testWrappedObjectAssignment1)
        JSP_TEST(force || true, testWrappedObjectAssignment2)
        JSP_TEST(force || true, testWrappedObjectAssignment3)
    }
    
    if (force || true)
    {
        JSP_TEST(force || true, testBarkerFinalization1)
        JSP_TEST(force || true, testHeapWrappedObject1)
        JSP_TEST(force || true, testWrappedBarker1)
        JSP_TEST(force || true, testRootedWrappedBarker1)
        JSP_TEST(force || true, testHeapWrappedBarker1)
        JSP_TEST(force || true, testHeapWrappedJSBarker1)
    }
    
    if (force || true)
    {
        JSP_TEST(force || true, testBarkerPassedToJS1);
        JSP_TEST(force || true, testBarkerJSGlobalFunctionality);
    }
}

// ---

void TestingRooting2::handleObject1(HandleObject object, const string &source)
{
    JSP::forceGC();
    
    if (JSP_CHECK(JSP::isHealthy(object.get()), "HEALTHY OBJECT"))
    {
        JSP_CHECK(source == toSource(object));
    }
}

void TestingRooting2::handleWrappedBarker1(Handle<WrappedObject> wrappedBarker)
{
    Barker::forceGC();
    JSP_CHECK(Barker::bark(wrappedBarker), "HEALTHY BARKER");
}

void TestingRooting2::handleMutableWrappedBarker1(MutableHandle<WrappedObject> wrappedBarker)
{
    JSObject *barker = wrappedBarker.get();
    wrappedBarker.set(nullptr);
    
    Barker::forceGC();
    JSP_CHECK(!Barker::bark(barker), "UNHEALTHY BARKER");
}

// ---

static bool nativeCallback(JSContext *cx, unsigned argc, Value *vp)
{
    CallArgsFromVp(argc, vp).rval().set(NumberValue(33));
    return true;
}

void TestingRooting2::testAnalysis1()
{
    JSObject *object = Barker::construct("UNROOTED");
    
    JSP_CHECK(JSP::isInsideNursery(object));
    JSP_CHECK(JSP::writeGCDescriptor(object) == 'n');
    
    Barker::forceGC();
    
    JSP_CHECK(!JSP::isHealthy(object)); // REASON: object NOT ROOTED
    JSP_CHECK(JSP::writeGCDescriptor(object) == 'P');
}

void TestingRooting2::testAnalysis2()
{
    RootedObject object(cx, Barker::construct("ROOTED"));
    JSFunction *function = JS_DefineFunction(cx, object, "someFunction", nativeCallback, 0, 0);
    
    JSP_CHECK(!JSP::isInsideNursery(function)); // TODO: FIND OUT WHY IT IS NOT CREATED IN THE NURSERY
    JSP_CHECK(JSP::writeGCDescriptor(function) == 'W');
    
    JSP::forceGC();
    
    JSP_CHECK(JSP::isHealthy(function)); // REASON: function ROOTED (VIA object)
    JSP_CHECK(JSP::writeGCDescriptor(function) == 'B');

    JS_DeleteProperty(cx, object, "someFunction");
    JSP::forceGC();

    JSP_CHECK(!JSP::isHealthy(function)); // REASON: function NOT ROOTED
    JSP_CHECK(JSP::writeGCDescriptor(function) == 'P');
}

void TestingRooting2::testAnalysis3()
{
    JSString *str = toJSString("whatever");
    
    JSP_CHECK(!JSP::isInsideNursery(str)); // TODO: FIND OUT WHY IT IS NOT CREATED IN THE NURSERY
    JSP_CHECK(JSP::writeGCDescriptor(str) == 'W');

    JSP::forceGC();
    
    JSP_CHECK(!JSP::isHealthy(str)); // REASON: str NOT ROOTED
    JSP_CHECK(JSP::writeGCDescriptor(str) == 'P');
}

// ---

void TestingRooting2::testWrappedObjectAssignment1()
{
    JSObject *barkerA = Barker::construct("ASSIGNED 1A");
    
    {
        WrappedObject wrapped; // ASSIGNMENT 1 (NO-OP)
        wrapped = Barker::construct("ASSIGNED 1B"); // ASSIGNMENT 2
        wrapped = barkerA; // ASSIGNMENT 3
        
        Rooted<WrappedObject> rootedWrapped(cx, wrapped); // WILL PROTECT wrapped (AND THEREFORE barkerA) FROM GC
        
        Barker::forceGC();
        JSP_CHECK(Barker::bark(rootedWrapped.address()), "HEALTHY BARKER"); // REASON: BARKER ROOTED
    }
    
    Barker::forceGC();
    JSP_CHECK(!Barker::isHealthy("ASSIGNED 1A"), "UNHEALTHY BARKER"); // REASON: BARKER NOT STACK-ROOTED ANYMORE
}

void TestingRooting2::testWrappedObjectAssignment2()
{
    JSObject *barker = Barker::construct("ASSIGNED 2");
    
    Rooted<WrappedObject> rootedWrapped(cx, barker); // ASSIGNMENT 1
    
    /*
     * ASSIGNMENT 2:
     *
     * THE WrappedObject ENCLOSING barker WILL BE SET TO NULL INSIDE handleMutableWrappedBarker1()
     *
     * barker WILL THEREFORE BE FINALIZED DURING THE TRIGGERED GC
     */
    handleMutableWrappedBarker1(&rootedWrapped);
}

/*
 * SIMILAR TO testWrappedObjectAssignment2()
 *
 * EXCEPT THAT WE HAD TO WORK HARD, BEHIND THE SCENES (E.G. IN WrappedObject), IN ORDER TO COPE WITH:
 *
 * 1) VALUE RE-ASSIGNABILITY, AUTOMATIC TRACING, ETC.
 *
 * 2) AUTOMATIC-CONVERSION FROM Heap<WrappedObject> TO Handle<WrappedObject>, ETC.
 */

void TestingRooting2::testWrappedObjectAssignment3()
{
    JSObject *barker = Barker::construct("ASSIGNED 3");
    
    Heap<WrappedObject> heapWrapped(barker); // ASSIGNMENT 1
    
    /*
     * ASSIGNMENT 2:
     *
     * THE WrappedObject ENCLOSING barker WILL BE SET TO NULL INSIDE handleMutableWrappedBarker1()
     *
     * barker WILL THEREFORE BE FINALIZED DURING THE TRIGGERED GC
     */
    handleMutableWrappedBarker1(heapWrapped); // AUTOMATIC-CONVERSION FROM Heap<WrappedObject> TO MutableHandle<WrappedObject>
}

// ---

/*
 * THE FOLLOWING SHOULD BE FURTHER INVESTIGATED REGARDING "GENERIC JS-OBJECT FINALIZATION" (TODO)
 *
 * 1) SPIDERMONKEY'S "PROBES" MECHANISM (DEFINED /js/src/vm/Probes), NOTABLY:
 *    bool FinalizeObject(JSObject *obj);
 *
 * 2) CHECK THE NewObjectGCKind() METHOD DEFINED IN jsobj.cpp
 *    - IT SEEMS TO DEFINE A CREATED-OBJECT'S "FINALIZE" METHOD
 *      - E.G. gc::FINALIZE_OBJECT4
 */

void TestingRooting2::testBarkerFinalization1()
{
    {
        Barker::construct("FINALIZATION 1"); // CREATED IN THE NURSERY, AS INTENDED
        
        Barker::forceGC();
        
        /*
         * INTERESTING FACT:
         *
         * WHEN AN OBJECT IS GARBAGE-COLLECTED WHILE IN THE NURSERY, SPIDERMONKEY
         * DOES NOT CONSIDER IT AS "ABOUT TO BE FINALIZED", AND SIMPLY MAKE IT POISONED
         *
         * THIS IS PROBABLY THE REASON WHY CLASSES WITH A FINALIZE-CALLBACK ARE
         * CONSTRUCTING OBJECTS DIRECTLY IN THE TENURED-HEAP
         */
        
        JSP_CHECK(Barker::isFinalized("FINALIZATION 1"), "FINALIZED BARKER");
    }
}

void TestingRooting2::testHeapWrappedObject1()
{
    /*
     * OBJECT APPEARS TO BE ALLOCATED DIRECTLY IN THE TENURED-HEAP, BUT IT COULD BE AN ILLUSION
     *
     * FACTS:
     * - IT DOESN'T MATTER IF {} OR "new Object({}) IS USED
     * - A "GENERIC JS-OBJECT" CLASS DOES NOT HAVE A finalize CALLBACK
     *   - WHICH IS A WELL-KNOWN NURSERY-SKIP REASON...
     *
     * POSSIBLY::
     * - THE OBJECT IS CREATED IN THE NURSERY
     * - JS::Evaluate() REQUIRES THE "RETURNED VALUE" TO BE ROOTED
     *   - WHICH IN TURN MOVES THE OBJECT TO THE TENURED-HEAP?
     *
     *
     * THE FOLLOWING SHOULD BE FURTHER INVESTIGATED REGARDING "GENERIC JS-OBJECT CREATION" (TODO)
     *
     * 1) APPEARING IN THE (INSIGHTFUL NewObject() METHOD) DEFINED IN jsobj.cpp:
     *    gc::InitialHeap heap = GetInitialHeap(newKind, clasp);
     *
     * 2) SPIDERMONKEY'S "PROBES" MECHANISM (DEFINED /js/src/vm/Probes), NOTABLY:
     *    bool CreateObject(ExclusiveContext *cx, JSObject *obj);
     */
    
    JSObject *object = evaluateObject("({foo: 'baz', bar: 1.5})", __FILE__, __LINE__);
    
    {
        if (true)
        {
            Heap<WrappedObject> heapWrapped(object); // ENCLOSING object IN Heap<WrappedObject> PROTECTS IT FROM GC AND ALLOWS TO FOLLOW "MOVED POINTERS"
            handleObject1(heapWrapped, toSource(heapWrapped.get()));
        }
        else
        {
            JSP::forceGC(); // object WILL BE FINALIZED
            JSP_CHECK(!JSP::isHealthy(object), "UNHEALTHY OBJECT");
            
            return;
        }
    }
    
    JSP::forceGC();
    JSP_CHECK(!JSP::isHealthy(object), "UNHEALTHY OBJECT");
}

void TestingRooting2::testWrappedBarker1()
{
    WrappedObject wrapped(Barker::construct("WRAPPED 1"));
    
    /*
     * BARKING VIA handleWrappedObject1() DOES NOT MAKE SENSE SINCE wrapped IS:
     * - NOT ENCLOSED IN Rooted<WrappedObject>
     * - NOT ENCLOSED IN Heap<WrappedObject>
     */
    
    JSP_CHECK(Barker::bark(&wrapped), "HEALTHY BARKER");
    
    Barker::forceGC();
    JSP_CHECK(Barker::isFinalized("WRAPPED 1"), "FINALIZED BARKER");
}

void TestingRooting2::testRootedWrappedBarker1()
{
    JSObject *object = Barker::construct("ROOTED-WRAPPED 1"); // CREATED IN THE NURSERY, AS INTENDED
    
    {
        Rooted<WrappedObject> rootedWrapped(cx, object);
        handleWrappedBarker1(rootedWrapped);
    }
    
    Barker::forceGC();
    JSP_CHECK(Barker::isFinalized("ROOTED-WRAPPED 1"), "FINALIZED BARKER");
}

void TestingRooting2::testHeapWrappedBarker1()
{
    JSObject *object = Barker::construct("HEAP-WRAPPED 1"); // CREATED IN THE NURSERY, AS INTENDED
    
    {
        Heap<WrappedObject> heapWrapped(object);
        handleWrappedBarker1(heapWrapped); // AUTOMATIC-CONVERSION FROM Heap<WrappedObject> TO Handle<WrappedObject>
    }
    
    Barker::forceGC();
    JSP_CHECK(Barker::isFinalized("HEAP-WRAPPED 1"), "FINALIZED BARKER");
}

/*
 * EXAMPLE OF SOME "EXPECTED LOG" FOR TestingRooting2::testHeapWrappedBarker1
 *
 * TODO:
 *
 * COULD BE INTEGRATED IN THE FORTHCOMING "RECORDING / TESTING" SYSTEM,
 * INCLUDING FEATURES LIKE "SMART TEMPLATE VARIABLE ANALYSIS", ETC.
 *
 * BEFOREHAND: SIMILAR SOLUTIONS SHOULD BE INVESTIGATED, E.G. /js/src/gdb/
 */

//testHeapWrappedBarker1
//Barker CONSTRUCTED: 0x10c700000 {Barker 8} [n] | HEAP-WRAPPED 1
//WrappedObject::WrappedObject(JSObject *) 0x7fff5fbfda70 | value: 0x10c700000 {Barker 8} [n]
//WrappedObject::WrappedObject(JSObject *) 0x7fff5fbfda80 | value:
//WrappedObject::WrappedObject(JSObject *) 0x7fff5fbfd958 | value: 0x10c700000 {Barker 8} [n]
//void WrappedObject::set(JSObject *) 0x7fff5fbfda80 | value: 0x10c700000 {Barker 8} [n]
//void WrappedObject::postBarrier() 0x7fff5fbfda80 | value: 0x10c700000 {Barker 8} [n]
//WrappedObject::~WrappedObject() 0x7fff5fbfd958
//WrappedObject::~WrappedObject() 0x7fff5fbfda70
//Barker GC-BEGIN
//Barker TRACED: 0x10d843130 {Barker 8} [W] | HEAP-WRAPPED 1
//void WrappedObject::trace(JSTracer *) 0x7fff5fbfda80 | value: 0x10d843130 {Barker 8} [W]
//Barker TRACED: 0x10d843130 {Barker 8} [B] | HEAP-WRAPPED 1
//Barker GC-END
//Barker BARKED: 0x10d843130 {Barker 8} [B] | HEAP-WRAPPED 1
//void WrappedObject::relocate() 0x7fff5fbfda80 | value: 0x10d843130 {Barker 8} [B]
//WrappedObject::~WrappedObject() 0x7fff5fbfda80
//Barker GC-BEGIN
//Barker FINALIZED: 0x10d843130 {Barker 8} [f] | HEAP-WRAPPED 1
//Barker GC-END

void TestingRooting2::testHeapWrappedJSBarker1()
{
    JSObject *object = evaluateObject("new Barker('heap-wrapped-js 1')", __FILE__, __LINE__); // CREATED IN THE NURSERY, AS INTENDED
    
    {
        Heap<WrappedObject> heapWrapped(object);
        
        /*
         * GC WILL MOVE THE BARKER TO THE "TENURED-HEAP", TURNING object INTO A DANGLING POINTER
         *
         * DURING GC:
         *
         * 1) THE BARKER'S trace CALLBACK IS INVOKED BY THE SYSTEM:
         *    - NOTIFYING THAT THE BARKER WAS MOVED THE TENURED-HEAP
         *      - AS EXPLAINED IN HeapAPI.h AND GCAPI.h:
         *        "All live objects in the nursery are moved to tenured at the beginning of each GC slice"
         *    - AT THIS STAGE, THE BARKER IS "WHITE":
         *      - AS EXPLAINED IN: https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/Internals/Garbage_collection
         *        - BLACK: "In common CS terminology, an object is black during the mark phase if it has been marked and its children
         *                  are gray (have been queued for marking). An object is black after the mark phase if it has been marked.
         *                  In SpiderMonkey, an object is black if its mark bit is set."
         *        - GRAY:  "In common CS terminology, an object is gray during the mark phase if it has been queued for marking.
         *                  In SpiderMonkey, an object is gray if it is a child of an object in the mark stack and it is not black.
         *                  Thus, gray objects are not represented explictly."
         *        - WHITE: "In common CS terminology, an object is white during the mark phase if it has not been seen yet.
         *                  An object is white after the mark phase if it has not been marked. In SpiderMonkey, an object is white if it is not gray or black;
         *                  i.e., it is not black and it is not a child of an object in the mark stack."
         *
         * 2) WrappedObject::trace IS INVOKED:
         *    - IN TURN, IT WILL CAUSE Barker::trace TO BE RE-INVOKED
         *      - AT THIS STAGE, THE BARKED WILL BE "BLACK"
         *    - WHY IS WrappedObject::trace INVOKED AT THE FIRST PLACE?
         *      - BECAUSE THE WrappedObject REGISTERED ITSELF TO OUR "CENTRALIZED EXTRA-ROOT-TRACING" SYSTEM
         *        DURING WrappedObject::postBarrier(), WHICH AUTOMATICALLY CALLED WHILE ENCLOSED IN A Heap<WrappedObject>
         */
        
        Barker::forceGC();
        
        JSP_CHECK(!JSP::isHealthy(object), "MOVED BARKER"); // ACCESSING THE BARKER VIA object WOULD BE A GC-HAZARD
        JSP_CHECK(Barker::bark(heapWrapped.address()), "HEALTHY BARKER"); // PASSING THROUGH Heap<WrappedObject> LEADS TO THE MOVED BARKER
    }
    
    Barker::forceGC();
    JSP_CHECK(Barker::isFinalized("heap-wrapped-js 1"), "FINALIZED BARKER");
}

// ---

/*
 * IT IS NECESSARY TO USE A ROOTED JS::Value WHEN PASSING ARGUMENTS TO A JS-FUNCTION CALLED FROM C++
 *
 * I.E. ANOTHER STRATEGY IS REQUIRED IN ORDER TO TEST "NON-ROOTED BARKERS CREATED ON THE C++ SIDE" FROM THE JS-SIDE
 */

void TestingRooting2::testBarkerPassedToJS1()
{
    executeScript("function handleBarker1(barker) { barker.bark(); }");

    {
        RootedValue value(cx, Barker::construct("PASSED-TO-JS 1"));
        
        /*
         * IT'S OKAY TO PASS A RootedValue:
         * https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/JSAPI_reference/JS::HandleValueArray
         */
        call(globalHandle(), "handleBarker1", value);
    }
    
    Barker::forceGC();
    JSP_CHECK(Barker::isFinalized("PASSED-TO-JS 1"), "FINALIZED BARKER"); // REASON: BARKER NOT ROOTED ANYMORE
}

void TestingRooting2::testBarkerJSGlobalFunctionality()
{
    /*
     * UNROOTED BARKER CREATED ON THE JS-SIDE:
     * - ACCESSING id AND name PROPERTIES FROM THE JS-SIDE, WITHOUT AFFECTING ROOTING
     */
    
    executeScript("print(new Barker('js-created unrooted 1').id, Barker.instances('js-created unrooted 1').name)");
    
    Barker::forceGC();
    JSP_CHECK(Barker::isFinalized("js-created unrooted 1"), "FINALIZED BARKER");

    // ---
    
    /*
     * UNROOTED BARKER CREATED ON THE C++ SIDE:
     * - BARKING FROM THE JS-SIDE, WITHOUT AFFECTING ROOTING
     */

    Barker::construct("CPP-CREATED UNROOTED 1");
    executeScript("Barker.instances('CPP-CREATED UNROOTED 1').bark();");
    
    Barker::forceGC();
    JSP_CHECK(Barker::isFinalized("CPP-CREATED UNROOTED 1"), "FINALIZED BARKER");
    
    /*
     * TODO:
     *
     * 1) IMPLEMENT Barker.isHealthy('name')
     * 2) IMPLEMENT Barker.bark('name')?
     * 3) CANCEL Barker::isFinalized()?
     * 4) THEN CONTINUE WITH THE FOLLOWING...
     */
    
    /*
    {
        Heap<WrappedValue> heapWrapped(Barker::construct("HEAP-WRAPPED 2").as<Value>());
        
        executeScript("Barker.forceGC(); Barker.instances('HEAP-WRAPPED 2').bark()");
    }
    
    executeScript("Barker.forceGC(); print(Barker.isHealthy('HEAP-WRAPPED 2'))");
    */
}
