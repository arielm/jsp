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
    WrappedValue::LOG_VERBOSE = false;
    WrappedObject::LOG_VERBOSE = false;
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
        JSP_TEST(force || true, testBarkerJSFunctionality);
        JSP_TEST(force || true, testBarkerMixedFunctionality);
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
        JSP_TEST(force || true, testHeapWrappedJSBarker2);
    }
}

// ---

void TestingRooting2::handleObject1(HandleObject object, const string &source)
{
    JSP::forceGC();
    
    JSP_CHECK(JSP::isHealthy(object.get()));
    JSP_CHECK(source == toSource(object));
}

void TestingRooting2::handleWrappedBarker1(Handle<WrappedObject> wrappedBarker)
{
    JSP::forceGC();
    JSP_CHECK(Barker::bark(wrappedBarker));
}

void TestingRooting2::handleMutableWrappedBarker1(MutableHandle<WrappedObject> wrappedBarker)
{
    JSObject *barker = wrappedBarker.get();
    wrappedBarker.set(nullptr);
    
    JSP::forceGC();
    JSP_CHECK(!Barker::bark(barker));
}

// ---

static bool nativeCallback(JSContext *cx, unsigned argc, Value *vp)
{
    CallArgsFromVp(argc, vp).rval().set(NumberValue(33));
    return true;
}

void TestingRooting2::testAnalysis1()
{
    JSObject *object = Barker::create("UNROOTED");
    
    JSP_CHECK(JSP::isInsideNursery(object));
    JSP_CHECK(JSP::writeGCDescriptor(object) == 'n');
    
    JSP::forceGC();
    
    JSP_CHECK(!JSP::isHealthy(object)); // REASON: object NOT ROOTED
    JSP_CHECK(JSP::writeGCDescriptor(object) == 'P');
}

void TestingRooting2::testAnalysis2()
{
    RootedObject object(cx, Barker::create("ROOTED"));
    JSFunction *function = JS_DefineFunction(cx, object, "someFunction", nativeCallback, 0, 0);
    
    LOGI << JSP::writeDetailed(function) << endl;
    JSP_CHECK(!JSP::isInsideNursery(function)); // XXX: IS function TENURED BECAUSE object IS ROOTED, OR IS IT ALWAYS THE CASE FOR NEW JSFunctions?
    
    JSP::forceGC();
    
    JSP_CHECK(JSP::isHealthy(function)); // REASON: function ROOTED (VIA object)
    JSP_CHECK(JSP::writeGCDescriptor(function) == 'B');

    JS_DeleteProperty(cx, object, "someFunction");
    JSP::forceGC();

    JSP_CHECK(!JSP::isHealthy(function)); // REASON: function NOT ROOTED ANYMORE
    JSP_CHECK(JSP::writeGCDescriptor(function) == 'P');
}

void TestingRooting2::testAnalysis3()
{
    JSString *s = toJSString("whatever");

    LOGI << JSP::writeDetailed(s) << endl;
    JSP_CHECK(!JSP::isInsideNursery(s)); // XXX: IT SEEMS THAT NEW JSStrings ARE ALWAYS TENURED

    JSP::forceGC();
    
    JSP_CHECK(!JSP::isHealthy(s)); // REASON: s NOT ROOTED
    JSP_CHECK(JSP::writeGCDescriptor(s) == 'P');
}

// ---

void TestingRooting2::testBarkerJSFunctionality()
{
    /*
     * FULL JS-SIDE FUNCTIONALITY:
     *
     * - CREATING UNROOTED BARKER
     * - ACCESSING id AND name PROPERTIES
     * - BARKING
     * - FORCING-GC (BARKER FINALIZED WHILE IN THE NURSERY)
     * - OBSERVING FINALIZATION
     */
    
    executeScript("\
                  print(new Barker('js-created unrooted 1').id, Barker.getInstance('js-created unrooted 1').name);\
                  Barker.bark('js-created unrooted 1');\
                  forceGC();\
                  print(Barker.isFinalized('js-created unrooted 1'));\
                  ");
}

void TestingRooting2::testBarkerMixedFunctionality()
{
    /*
     * MIXED FUNCTIONALITY:
     *
     * - C++ SIDE:
     *   - CREATING UNROOTED BARKER
     *   - ACCESSING id AND name
     *
     * - JS-SIDE:
     *   - BARKING
     *   - FORCING-GC (BARKER FINALIZED WHILE IN THE NURSERY)
     *   - OBSERVING FINALIZATION
     */
    
    LOGI << Barker::getId(Barker::create("CPP-CREATED UNROOTED 1")) << " " << Barker::getName(Barker::getInstance("CPP-CREATED UNROOTED 1")) << endl;
    
    executeScript("\
                  Barker.bark('CPP-CREATED UNROOTED 1');\
                  forceGC();\
                  print(Barker.isFinalized('CPP-CREATED UNROOTED 1'));\
                  ");
}

// ---

void TestingRooting2::testWrappedObjectAssignment1()
{
    JSObject *barkerA = Barker::create("ASSIGNED 1A");
    
    {
        WrappedObject wrapped; // ASSIGNMENT 1 (NO-OP)
        wrapped = Barker::create("ASSIGNED 1B"); // ASSIGNMENT 2
        wrapped = barkerA; // ASSIGNMENT 3
        
        Rooted<WrappedObject> rootedWrapped(cx, wrapped); // WILL PROTECT wrapped (AND THEREFORE barkerA) FROM GC
        
        JSP::forceGC();
        JSP_CHECK(Barker::bark(&rootedWrapped)); // REASON: BARKER ROOTED
    }
    
    JSP::forceGC();
    JSP_CHECK(!Barker::isHealthy("ASSIGNED 1A")); // REASON: BARKER NOT STACK-ROOTED ANYMORE
}

void TestingRooting2::testWrappedObjectAssignment2()
{
    JSObject *barker = Barker::create("ASSIGNED 2");
    
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
    JSObject *barker = Barker::create("ASSIGNED 3");
    
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
 * TODO:
 *
 * 1) BRING-BACK THE POSSIBILITY TO ANALYSE OBJECTS DURING SPIDERMONKEY'S FINALIZATION-CALLBACK
 * 2) USE IT TO DEMONSTRATE THE "INTERESTING FACT" MENTIONED IN testBarkerFinalization1()
 * 3) USE IT TO SOLVE "MYSTERY" MENTIONED IN testHeapWrappedObject1()
 */

void TestingRooting2::testBarkerFinalization1()
{
    Barker::create("FINALIZATION 1");
    JSP_CHECK(JSP::isInsideNursery(Barker::getInstance("FINALIZATION 1"))); // CREATED IN THE NURSERY, AS INTENDED
    
    /*
     * INTERESTING FACT:
     *
     * WHEN AN OBJECT IS GARBAGE-COLLECTED WHILE IN THE NURSERY, SPIDERMONKEY
     * DOES NOT CONSIDER IT AS "ABOUT TO BE FINALIZED", AND SIMPLY MAKES IT POISONED
     *
     * THIS IS PROBABLY THE REASON WHY CLASSES WITH A FINALIZE-CALLBACK ARE
     * ALLOCATING OBJECTS DIRECTLY IN THE TENURED-HEAP
     */
    
    JSP::forceGC();
    JSP_CHECK(Barker::isFinalized("FINALIZATION 1"));
}

/*
 * TODO: THE FOLLOWING SHOULD BE FURTHER INVESTIGATED REGARDING "GENERIC JS-OBJECT CREATION"
 *
 * 1) THE NewObjectGCKind() METHOD DEFINED IN jsobj.cpp
 *    - IT SEEMS TO DEFINE A CREATED-OBJECT'S "FINALIZE" METHOD
 *      - E.G. gc::FINALIZE_OBJECT4
 *
 * 2) APPEARING IN THE (INSIGHTFUL NewObject() METHOD) DEFINED IN jsobj.cpp:
 *    gc::InitialHeap heap = GetInitialHeap(newKind, clasp);
 *
 * 3) SPIDERMONKEY'S "PROBES" MECHANISM (DEFINED /js/src/vm/Probes), NOTABLY:
 *    - bool CreateObject(ExclusiveContext *cx, JSObject *obj)
 *    - bool FinalizeObject(JSObject *obj)
 */

void TestingRooting2::testHeapWrappedObject1()
{
    /*
     * MYSTERY: OBJECT APPEARS TO BE ALLOCATED DIRECTLY IN THE TENURED-HEAP!?
     *
     *
     * FACTS:
     *
     * 1) IT DOESN'T MATTER IF {} OR "new Object({}) IS USED
     *
     * 2) THE "GENERIC JS-OBJECT" CLASS DOES NOT APPEAR TO HAVE A FINALIZE-CALLBACK
     *    - ASSUMING THIS IS THE CLASS USED HERE
     *
     * 3) A BARKER CREATED SIMILARELY VIA evaluateObject() WOULD BE ALLOCATED IN THE NURSERY
     *    - SEE testHeapWrappedJSBarker1()
     */
    
    JSObject *object = evaluateObject("({foo: 'baz', bar: 1.5})");
    JSP_CHECK(!JSP::isInsideNursery(object));
    
    {
        if (true)
        {
            Heap<WrappedObject> heapWrapped(object); // ENCLOSING object IN Heap<WrappedObject> PROTECTS IT FROM GC AND ALLOWS TO FOLLOW "MOVED POINTERS"
            handleObject1(heapWrapped, toSource(heapWrapped.get()));
        }
        else
        {
            JSP::forceGC(); // object WILL BE FINALIZED
            JSP_CHECK(!JSP::isHealthy(object));
            
            return;
        }
    }
    
    JSP::forceGC();
    JSP_CHECK(!JSP::isHealthy(object));
}

void TestingRooting2::testWrappedBarker1()
{
    WrappedObject wrapped(Barker::create("WRAPPED 1"));
    
    /*
     * BARKING VIA handleWrappedObject1() DOES NOT MAKE SENSE SINCE wrapped IS:
     * - NOT ENCLOSED IN Rooted<WrappedObject>
     * - NOT ENCLOSED IN Heap<WrappedObject>
     */
    
    JSP_CHECK(Barker::bark(&wrapped));
    
    JSP::forceGC();
    JSP_CHECK(Barker::isFinalized("WRAPPED 1"));
}

void TestingRooting2::testRootedWrappedBarker1()
{
    JSObject *object = Barker::create("ROOTED-WRAPPED 1"); // CREATED IN THE NURSERY, AS INTENDED
    
    {
        Rooted<WrappedObject> rootedWrapped(cx, object);
        handleWrappedBarker1(rootedWrapped);
    }
    
    JSP::forceGC();
    JSP_CHECK(Barker::isFinalized("ROOTED-WRAPPED 1"));
}

void TestingRooting2::testHeapWrappedBarker1()
{
    JSObject *object = Barker::create("HEAP-WRAPPED 1"); // CREATED IN THE NURSERY, AS INTENDED
    
    {
        Heap<WrappedObject> heapWrapped(object);
        handleWrappedBarker1(heapWrapped); // AUTOMATIC-CONVERSION FROM Heap<WrappedObject> TO Handle<WrappedObject>
    }
    
    JSP::forceGC();
    JSP_CHECK(Barker::isFinalized("HEAP-WRAPPED 1"));
}

/*
 * EXAMPLE OF SOME "EXPECTED LOG" FOR testHeapWrappedBarker1
 *
 * TODO:
 *
 * 1) CONSIDER INTEGRATION IN THE FORTHCOMING "RECORDING / TESTING" SYSTEM:
 *    - INCLUDING FEATURES LIKE "SMART TEMPLATE VARIABLE ANALYSIS", ETC.
 *
 * 2) BEFOREHAND: SIMILAR SOLUTIONS SHOULD BE INVESTIGATED, E.G. /js/src/gdb/
 */

/*
testHeapWrappedBarker1
Barker CONSTRUCTED: 0x10be00000 {Barker c} [n] | HEAP-WRAPPED 1
jsp::WrappedObject::WrappedObject(JSObject *) 0x7fff5fbfb0b0 | object: 0x10be00000 {Barker c} [n]
jsp::WrappedObject::WrappedObject() 0x7fff5fbfb0b8 | object:
jsp::WrappedObject::WrappedObject(const jsp::WrappedObject &) 0x7fff5fbfafb0 | object: 0x10be00000 {Barker c} [n]
void jsp::WrappedObject::operator=(const jsp::WrappedObject &) 0x7fff5fbfb0b8 | object: 0x10be00000 {Barker c} [n]
void jsp::WrappedObject::postBarrier() 0x7fff5fbfb0b8 | object: 0x10be00000 {Barker c} [n]
jsp::WrappedObject::~WrappedObject() 0x7fff5fbfafb0 | object: 0x10be00000 {Barker c} [n]
jsp::WrappedObject::~WrappedObject() 0x7fff5fbfb0b0 | object: 0x10be00000 {Barker c} [n]
JSP::forceGC() | BEGIN
Barker TRACED: 0x10cf43130 {Barker c} [W] | HEAP-WRAPPED 1
Barker TRACED: 0x10cf43130 {Barker c} [B] | HEAP-WRAPPED 1
JSP::forceGC() | END
Barker BARKED: 0x10cf43130 {Barker c} [B] | HEAP-WRAPPED 1
void jsp::WrappedObject::relocate() 0x7fff5fbfb0b8 | object: 0x10cf43130 {Barker c} [B]
jsp::WrappedObject::~WrappedObject() 0x7fff5fbfb0b8 | object: 0x10cf43130 {Barker c} [B]
JSP::forceGC() | BEGIN
Barker FINALIZED: 0x10cf43130 [P] | HEAP-WRAPPED 1
JSP::forceGC() | END
*/

void TestingRooting2::testHeapWrappedJSBarker1()
{
    JSObject *object = evaluateObject("new Barker('heap-wrapped-js 1')");
    JSP_CHECK(JSP::isInsideNursery(object)); // CREATED IN THE NURSERY, AS INTENDED
    
    {
        Heap<WrappedObject> heapWrapped(object);
        
        /*
         * GC WILL MOVE THE BARKER TO THE TENURED-HEAP, TURNING object INTO A DANGLING POINTER
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
         *        DURING WrappedObject::postBarrier(), WHICH IS AUTOMATICALLY CALLED WHILE ENCLOSED IN A Heap<WrappedObject>
         */
        
        JSP::forceGC();
        
        JSP_CHECK(!JSP::isHealthy(object)); // ACCESSING THE BARKER VIA object WOULD BE A GC-HAZARD
        JSP_CHECK(Barker::bark(&heapWrapped)); // PASSING THROUGH Heap<WrappedObject> LEADS TO THE MOVED BARKER
    }
    
    JSP::forceGC();
    JSP_CHECK(Barker::isFinalized("heap-wrapped-js 1"));
}

// ---

/*
 * IT IS NECESSARY TO USE ROOTED VALUES WHEN PASSING ARGUMENTS TO A JS-FUNCTION CALLED FROM C++,
 * WHICH IN TURNS AFFECTS THE CAPABILITY TO TEST NON-ROOTED OBJECTS FROM ONE "SIDE" (I.E. JS OR C++) TO THE OTHER
 *
 * ANOTHER APPROACH MUST THEREFORE BE USED, AS DEMONSTRATED IN THE FOLLOWING:
 * - testBarkerJSFunctionality()
 * - testBarkerMixedFunctionality()
 */

void TestingRooting2::testBarkerPassedToJS1()
{
    executeScript("function handleBarker1(barker) { barker.bark(); }");

    {
        RootedValue arg(cx, Barker::create("PASSED-TO-JS 1"));
        call(globalHandle(), "handleBarker1", arg);
    }
    
    JSP::forceGC();
    JSP_CHECK(Barker::isFinalized("PASSED-TO-JS 1")); // REASON: BARKER NOT ROOTED ANYMORE
}

void TestingRooting2::testHeapWrappedJSBarker2()
{
    executeScript("new Barker('HEAP-WRAPPED 2')"); // CREATED IN THE NURSERY, AS INTENDED

    {
        Heap<WrappedValue> heapWrapped(Barker::getInstance("HEAP-WRAPPED 2"));
        
        JSP::forceGC();
        JSP_CHECK(Barker::bark(&heapWrapped));
    }
    
    JSP::forceGC();
    JSP_CHECK(Barker::isFinalized("HEAP-WRAPPED 2"));
}
