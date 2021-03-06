/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "TestingJS.h"

#include "chronotext/Context.h"

using namespace std;
using namespace ci;
using namespace chr;

using namespace jsp;

void TestingJS::performRun(bool force)
{
    if (force || false)
    {
        dumpObject(globalHandle());
    }
    
    if (force || false)
    {
        executeScript(InputSource::getAsset("helpers.js"));
        executeScript(InputSource::getAsset("test.js"));

        call(globalHandle(), "start");
        LOGI << writeDetailed(get<OBJECT>(globalHandle(), "foo")) << endl;
        
        executeScript("print(1, 'אריאל מלכא', {x: 5, y: null})");
        executeScript("error tester", __FILE__, __LINE__);
    }
    
    // ---
    
    if (force || true)
    {
        JSP_TEST(force || true, testParsing1)
        JSP_TEST(force || true, testParsing2)
        JSP_TEST(force || true, testStringify)
        JSP_TEST(force || true, testToSource)
    }
    
    if (force || false)
    {
        testThreadSafety();
    }

    if (force || false)
    {
        testEvaluationScope();
        testFunctionScope();
    }

    if (force || false)
    {
        testCustomScriptExecution();
    }
    
    if (force || false)
    {
        testShapes1();
        testShapes2();
        testAtoms();
        testReservedSlots();
        testJSID();
    }
    
    if (force || true)
    {
        JSP_TEST(force || true, testGetter1)
        JSP_TEST(force || true, testSetter1)
        JSP_TEST(force || true, testGetterSetter1)
    }
    
    if (force || true)
    {
        JSP_TEST(force || true, testCustomConstruction1)
        JSP_TEST(force || true, testCustomConstruction2)
        JSP_TEST(force || true, testNewObject)
    }
    
    if (force || true)
    {
        JSP_TEST(force || true, testReadOnlyProperty1)
        JSP_TEST(force || true, testReadOnlyProperty2)
        
        JSP_TEST(force || true, testPermanentProperty1)
        JSP_TEST(force || true, testPermanentProperty2)
        
        JSP_TEST(force || true, testObjectReset1)
        JSP_TEST(force || true, testObjectReset2)
    }
    
    if (force || true)
    {
        JSP_TEST(force || true, testGetProperty1)
        JSP_TEST(force || true, testGetElement1)
        
        JSP_TEST(force || true, testGetProperties1)
        JSP_TEST(force || true, testGetProperties2)
        JSP_TEST(force || true, testGetElements1)
        JSP_TEST(force || true, testToValue1)
        JSP_TEST(force || true, testToValue2)
        JSP_TEST(force || true, testSetElements1)
        JSP_TEST(force || true, testSetElements2)
    }
    
    if (force || true)
    {
        JSP_TEST(force || true, testBulkArrayOperations1)
        JSP_TEST(force || true, testBulkArrayOperations2)
        
        JSP_TEST(force || true, testBulkArrayOperations3)
        JSP_TEST(force || true, testBulkArrayOperations4)
    }
    
    if (force || true)
    {
        JSP_TEST(force || true, testArrayElementCount)
    }
}

#pragma mark ---------------------------------------- ARRAYS ----------------------------------------

/*
 * DEMONSTRATED:
 *
 * - HOW TO OBTAIN THE "TRUE" ELEMENT-COUNT OF A JS-ARRAY
 * - HOW TO WORK WITH ForOfIterator
 */
void TestingJS::testArrayElementCount()
{
    RootedObject array(cx, newArray());
    
    set(array, 0, -255);
    set(array, 1, true);
    set(array, 2, nullptr);
    set(array, 3, "foo");
    
    deleteElement(array, 1);

    /*
     * Array.length IS NOT REFLECTING THE "TRUE" ELEMENT-COUNT
     */
    JSP_CHECK(getLength(array) == 4);

    // ---
    
    uint32_t elementCount = 0;
    
    RootedValue iterable(cx, ObjectOrNullValue(array));
    ForOfIterator it(cx);
    
    if (it.init(iterable))
    {
        bool done = false;
        RootedValue value(cx);

        while (it.next(&value, &done) && !done)
        {
            if (!value.isUndefined())
            {
                elementCount++;
            }
        }
    }
    
    JSP_CHECK(elementCount == 3);
}

#pragma mark ---------------------------------------- GETTING / SETTING PROPERTIES AND ELEMENTS ----------------------------------------

/*
 * TESTING THE BEHAVIOR OF JS_GetProperty FOR NON-DEFINED PROPERTIES:
 *
 * - IT "RETURNS" A VALUE OF TYPE "UNDEFINED"
 * - IT'S POSSIBLE TO CHECK THE TYPE OF THE RETURNED VALUE
 */
void TestingJS::testGetProperty1()
{
    RootedObject object(cx, newPlainObject());
    RootedValue result(cx);
    
    if (JS_GetProperty(cx, object, "notDefined", &result))
    {
        JSP_CHECK(result.isUndefined());
        
        JSP_CHECK(!result.isNull());
        JSP_CHECK(!result.isObject());
        JSP_CHECK(!result.isNumber());
        JSP_CHECK(!result.isInt32());
        JSP_CHECK(!result.isString());
    }
    else
    {
        JSP_CHECK(false);
    }
}

/*
 * TESTING THE BEHAVIOR OF JS_GetElement FOR NON-DEFINED ELEMENTS: SIMILAR TO JS_GetProperty
 */
void TestingJS::testGetElement1()
{
    RootedObject array(cx, newArray());
    RootedValue result(cx);
    
    if (JS_GetElement(cx, array, 999, &result))
    {
        JSP_CHECK(result.isUndefined());
        
        JSP_CHECK(!result.isNull());
        JSP_CHECK(!result.isObject());
        JSP_CHECK(!result.isNumber());
        JSP_CHECK(!result.isInt32());
        JSP_CHECK(!result.isString());
    }
    else
    {
        JSP_CHECK(false);
    }
}

// ---

void TestingJS::testGetProperties1()
{
    RootedObject object(cx, evaluateObject("({x: 25.5, area: 33.33, balance: -255, count: 0xff123456, alive: true, parent: null, child: {}, name: 'foo', xxx: undefined})"));

    JSP_CHECK(get<FLOAT32>(object, "x") == 25.5f);
    JSP_CHECK(get<FLOAT64>(object, "area") == 33.33);
    JSP_CHECK(get<INT32>(object, "balance") == -255);
    JSP_CHECK(get<UINT32>(object, "count") == 0xff123456);
    JSP_CHECK(get<BOOLEAN>(object, "alive") == true);
    JSP_CHECK(get<OBJECT>(object, "parent") == nullptr);
    JSP_CHECK(get<OBJECT>(object, "child")->getClass() == &JSObject::class_);
    JSP_CHECK(get<STRING>(object, "name") == "foo");

    /*
     * DEFAULT VALUES
     */
    JSP_CHECK(get<FLOAT32>(object, "notDefined") == 0);
    JSP_CHECK(get<FLOAT64>(object, "notDefined") == 0);
    JSP_CHECK(get<INT32>(object, "notDefined") == 0);
    JSP_CHECK(get<UINT32>(object, "notDefined") == 0);
    JSP_CHECK(get<BOOLEAN>(object, "notDefined") == false);
    JSP_CHECK(get<OBJECT>(object, "notDefined") == nullptr);
    JSP_CHECK(get<STRING>(object, "notDefined") == "");
    
    JSP_CHECK(get<INT32>(object, "xxx", -333) == -333); // CUSTOM DEFAULT VALUE
}

void TestingJS::testGetProperties2()
{
    RootedObject object(cx, evaluateObject("({bah: {}, whatever: 'bar', doh: 'baz'})"));

    RootedObject o(cx, get<OBJECT>(object, "bah"));
    testProperties(object, {"bah", "whatever", "doh"}, o, "bar", "baz");
}

void TestingJS::testProperties(HandleObject object, const vector<string> &properties, HandleObject o, const string &s1, const char *s2)
{
    JSP_CHECK(get<OBJECT>(object, properties[0].data()) == o);
    JSP_CHECK(get<OBJECT>(object, "notDefined") != o);

    JSP_CHECK(get<STRING>(object, properties[1].data()) == s1);
    JSP_CHECK(get<STRING>(object, "notDefined") != s1);
    
    JSP_CHECK(get<STRING>(object, properties[2].data()) == s2);
    JSP_CHECK(get<STRING>(object, "notDefined") != s2);
}

// ---

void TestingJS::testGetElements1()
{
    RootedObject array(cx, evaluateObject("([25.5, 33.33, -255, 0xff123456, true, null, {}, 'foo', , {}, 'bar', 'baz'])"));
    int index = 0;

    JSP_CHECK(get<FLOAT32>(array, index++) == 25.5f);
    JSP_CHECK(get<FLOAT64>(array, index++) == 33.33);
    JSP_CHECK(get<INT32>(array, index++) == -255);
    JSP_CHECK(get<UINT32>(array, index++) == 0xff123456);
    JSP_CHECK(get<BOOLEAN>(array, index++) == true);
    JSP_CHECK(get<OBJECT>(array, index++) == nullptr);
    JSP_CHECK(get<OBJECT>(array, index++)->getClass() == &JSObject::class_);
    JSP_CHECK(get<STRING>(array, index++) == "foo");

    /*
     * DEFAULT VALUES
     */
    JSP_CHECK(get<FLOAT32>(array, 999) == 0);
    JSP_CHECK(get<FLOAT64>(array, 999) == 0);
    JSP_CHECK(get<INT32>(array, 999) == 0);
    JSP_CHECK(get<UINT32>(array, 999) == 0);
    JSP_CHECK(get<BOOLEAN>(array, 999) == false);
    JSP_CHECK(get<OBJECT>(array, 999) == nullptr);
    JSP_CHECK(get<STRING>(array, 999) == "");

    JSP_CHECK(get<INT32>(array, index++, -333) == -333); // CUSTOM DEFAULT VALUE
    
    // ---
    
    RootedObject o(cx, get<OBJECT>(array, index));
    JSP_CHECK(get<OBJECT>(array, index++) == o);
    JSP_CHECK(get<OBJECT>(array, 999) != o);
    
    string s1 = "bar";
    JSP_CHECK(get<STRING>(array, index++) == s1);
    JSP_CHECK(get<STRING>(array, 999) != s1);

    const char *s2 = "baz";
    JSP_CHECK(get<STRING>(array, index++) == s2);
    JSP_CHECK(get<STRING>(array, 999) != s2);
}

// ---

void TestingJS::testToValue1()
{
    RootedObject array(cx, newArray());
    int index = 0;

    /*
     * TODO: toValue(void) COULD PRODUCE AN UNDEFINED VALUE
     */
    
    RootedValue f(cx, toValue(25.5f));
    JS_SetElement(cx, array, index++, f);
    
    RootedValue d(cx, toValue(33.33));
    JS_SetElement(cx, array, index++, d);
    
    RootedValue i(cx, toValue(-255));
    JS_SetElement(cx, array, index++, i);
    
    RootedValue ui(cx, toValue(0xff123456));
    JS_SetElement(cx, array, index++, ui);

    RootedValue b(cx, toValue(true));
    JS_SetElement(cx, array, index++, b);
    
    RootedValue n(cx, toValue(nullptr));
    JS_SetElement(cx, array, index++, n);

    RootedValue o(cx, toValue(newPlainObject()));
    JS_SetElement(cx, array, index++, o);
    
    RootedValue s(cx, toValue("foo"));
    JS_SetElement(cx, array, index++, s);
    
    JSP_CHECK(toSource(array) == "[25.5, 33.33, -255, 4279383126, true, null, {}, \"foo\"]");
}

void TestingJS::testToValue2()
{
    RootedObject array(cx, newArray());
    
    RootedObject o(cx, newPlainObject());
    setValues1(array, {0, 1, 2}, o, "bar", "baz");
    
    JSP_CHECK(toSource(array) == "[{}, \"bar\", \"baz\"]");
}

void TestingJS::setValues1(HandleObject array, const vector<int> &indices, HandleObject o, const string &s1, const char *s2)
{
    RootedValue rooted(cx);
    int index = 0;

    rooted = toValue(o);
    JS_SetElement(cx, array, indices[index++], rooted);

    rooted = toValue(s1);
    JS_SetElement(cx, array, indices[index++], rooted);
    
    rooted = toValue(s2);
    JS_SetElement(cx, array, indices[index++], rooted);
}

// ---

void TestingJS::testSetElements1()
{
    RootedObject array(cx, newArray());
    int index = 0;
    
    /*
     * TODO: set(array, index, void) COULD SET AN UNDEFINED VALUE
     */
    
    set(array, index++, 25.5f);
    set(array, index++, 33.33);
    set(array, index++, -255);
    set(array, index++, 0xff123456);
    set(array, index++, true);
    set(array, index++, nullptr);
    set(array, index++, newPlainObject());
    set(array, index++, "foo");

    JSP_CHECK(toSource(array) == "[25.5, 33.33, -255, 4279383126, true, null, {}, \"foo\"]");
}

void TestingJS::testSetElements2()
{
    RootedObject array(cx, newArray());
    
    RootedObject o(cx, newPlainObject());
    setValues2(array, {0, 1, 2}, o, "bar", "baz");
    
    JSP_CHECK(toSource(array) == "[{}, \"bar\", \"baz\"]");
}

void TestingJS::setValues2(HandleObject array, const vector<int> &indices, HandleObject o, const string &s1, const char *s2)
{
    RootedValue rooted(cx);
    int index = 0;
    
    set(array, indices[index++], o);
    set(array, indices[index++], s1);
    set(array, indices[index++], s2);
}

// ---

void TestingJS::testBulkArrayOperations1()
{
    {
        RootedObject array(cx, evaluateObject("([1.33, , 3.33])"));
        
        auto elements = getElements<FLOAT64>(array);
        JSP_CHECK(write(elements) == "[1.33, 0, 3.33]");
    }

    {
        RootedObject array(cx, evaluateObject("([-4096, , 8192])"));
        
        auto elements = getElements<INT32>(array, 555); // USING A CUSTOM DEFAULT-VALUE
        JSP_CHECK(write(elements) == "[-4096, 555, 8192]");
    }
    
    {
        RootedObject array(cx, evaluateObject("([0xff123456, , 8192])"));
        
        auto elements = getElements<UINT32>(array);
        JSP_CHECK(write(elements) == "[4279383126, 0, 8192]");
    }
    
    {
        RootedObject array(cx, evaluateObject("([true, , false])"));

        auto elements = getElements<BOOLEAN>(array);
        JSP_CHECK(write(elements) == "[1, 0, 0]");
    }

    {
        /*
         * USAGE OF Proto::getElements<OBJECT>(HandleObject sourceArray) IS DISCOURAGED:
         * - NOT GC-SAFE IN TERM OF OUTPUT-VALUES
         *
         * RECOMMENDED ALTERNATIVE:
         * - Proto::getElements(HandleObject sourceArray, AutoValueVector &elements)
         */

        RootedObject array(cx, evaluateObject("([{}, , []])"));
        auto elements = getElements<OBJECT>(array);
        
        setLength(array, 0);
        appendElements(array, elements);
        JSP_CHECK(toSource(array) == "[{}, null, []]");
    }
    
    {
        RootedObject array(cx, evaluateObject("(['one', , 'three'])"));
        
        auto elements = getElements<STRING>(array, "INVALID"); // USING A CUSTOM DEFAULT-VALUE
        JSP_CHECK(write(elements) == "[one, INVALID, three]");
    }
    
    // ---
    
    {
        RootedObject array(cx, newArray());
        JSP_CHECK(getElements<UINT32>(array).empty());
    }
    
    JSP_CHECK(getElements<UINT32>(NullPtr()).empty());
}

void TestingJS::testBulkArrayOperations2()
{
    {
        RootedObject array(cx, newArray());
        
        JSP_CHECK(appendElements(array, vector<FLOAT32> {1.5f, 2.5f, 3.5f}) == 3);
        JSP_CHECK(toSource(array) == "[1.5, 2.5, 3.5]");
    }
    
    {
        RootedObject array(cx, newArray());
        
        JSP_CHECK(appendElements(array, vector<FLOAT64> {1.33, 2.33, 3.33}) == 3);
        JSP_CHECK(toSource(array) == "[1.33, 2.33, 3.33]");
    }
    
    {
        RootedObject array(cx, newArray());
        
        JSP_CHECK(appendElements(array, vector<INT32> {-4096, 256, 8192}) == 3);
        JSP_CHECK(toSource(array) == "[-4096, 256, 8192]");
    }
    
    {
        RootedObject array(cx, newArray());
        
        JSP_CHECK(appendElements(array, vector<UINT32> {0xff123456, 256, 8192}) == 3);
        JSP_CHECK(toSource(array) == "[4279383126, 256, 8192]");
    }
    
    {
        RootedObject array(cx, newArray());
        
        JSP_CHECK(appendElements(array, vector<BOOLEAN> {true, false, false}) == 3);
        JSP_CHECK(toSource(array) == "[true, false, false]");
    }
    
    {
        /*
         * USAGE OF Proto::appendElements<OBJECT>(HandleObject targetArray, const vector<OBJECT> &elements) IS DISCOURAGED:
         * - NOT GC-SAFE IN TERM OF INPUT-VALUES
         *
         * RECOMMENDED ALTERNATIVE:
         * - Proto::appendElements(HandleObject targetArray, const HandleValueArray &elements)
         */
        
        RootedObject array(cx, newArray());

        JSP_CHECK(appendElements(array, vector<OBJECT> {newPlainObject(), nullptr, newArray()}) == 3);
        JSP_CHECK(toSource(array) == "[{}, null, []]");
    }
    
    {
        RootedObject array(cx, newArray());

        JSP_CHECK(appendElements(array, vector<STRING> {"one", "two", "three"}) == 3);
        JSP_CHECK(toSource(array) == "[\"one\", \"two\", \"three\"]");
    }
    
    // ---

    {
        RootedObject array(cx, newArray());
        JSP_CHECK(!appendElements(array, vector<INT32> {}));
    }
    
    JSP_CHECK(!appendElements(NullPtr(), vector<INT32> {1, 2, 3}));
    
    // ---
    
    {
        RootedObject array(cx, evaluateObject("([1, 2, 3])"));
        
        JSP_CHECK(appendElements(array, vector<INT32> {4, 5}) == 2);
        JSP_CHECK(toSource(array) == "[1, 2, 3, 4, 5]");
    }
}

// ---

/*
 * CHECKING JAVASCRIPT'S POLICY WITH ARRAYS IN REGARD TO "HOLES" AND UNDEFINED-VALUES
 *
 * NOTE: "(void 0)" IS WHAT Array.toSource() WRITES FOR UNDEFINED-VALUES
 */
void TestingJS::testBulkArrayOperations3()
{
    AutoValueVector contents(cx);

    contents.append(UndefinedValue());
    contents.append(NumberValue(123));
    contents.append(UndefinedValue());
    
    RootedObject array(cx, newArray(contents));
    JSP_CHECK(toSource(array) == "[(void 0), 123, (void 0)]"); // WE COULD HAVE EXPECTED [, 123, ,]
    
    // ---
    
    executeScript("var array1 = []; array1.length = 3; array1[1] = 123;");
    JSP_CHECK(toSource(get<OBJECT>(globalHandle(), "array1")) == "[, 123, ,]"); // AS EXPECTED
    
    executeScript("var array2 = []; for (var i = 0; i < array1.length; i++) array2[i] = array1[i];"); // NO WARNINGS
    JSP_CHECK(toSource(get<OBJECT>(globalHandle(), "array2")) == "[(void 0), 123, (void 0)]"); // WE COULD HAVE EXPECTED [, 123, ,]
    
    //
    
    executeScript("array1.push(33.33); array1.push(array1[0])"); // PRODUCES A WARNING: ReferenceError: reference to undefined property array1[0]
    JSP_CHECK(toSource(get<OBJECT>(globalHandle(), "array1")) == "[, 123, , 33.33, (void 0)]");
}

/*
 * APPLYING THE PRINCIPLES VERIFIED IN testBulkArrayOperations3()
 */
void TestingJS::testBulkArrayOperations4()
{
    RootedObject array1(cx, evaluateObject("([1, 33.33, {}, , [], null, 'foo'])")); // THE 4th ELEMENT IS A "HOLE"
    
    AutoValueVector elements1(cx);
    JSP_CHECK(getElements(array1, elements1) == 7);
    
    RootedObject array2(cx, newArray());
    JSP_CHECK(appendElements(array2, elements1) == 7);
    
    JSP_CHECK(toSource(array2) == "[1, 33.33, {}, (void 0), [], null, \"foo\"]"); // UNLIKE array1: THE 4th ELEMENT IS AN UNDEFINED-VALUE

    // ---
    
    RootedObject array3(cx, newArray());
    setLength(array3, 1); // array3 CONTAINS A "HOLE"
    
    AutoValueVector elements3(cx);
    JSP_CHECK(getElements(array3, elements3) == 1);
    
    RootedObject array4(cx, evaluateObject("([1, 2])"));
    JSP_CHECK(appendElements(array4, elements3) == 1);

    JSP_CHECK(toSource(array4) == "[1, 2, (void 0)]"); // THE ELEMENT APPENDED IS AN UNDEFINED-VALUE
    
    // ---
    
    RootedValue value(cx, NumberValue(99));
    JSP_CHECK(getElement(array3, 0, &value) && value.isUndefined()); // ACCESSING A "HOLE" PRODUCES AN UNDEFINED-VALUE
    
    append<FLOAT64>(array3, 55.55);
    appendElement(array3, value);
    
    JSP_CHECK(toSource(array3) == "[, 55.55, (void 0)]");
}

#pragma mark ---------------------------------------- READ-ONLY AND PERMANENT PROPERTIES ----------------------------------------

/*
 * OPEN QUESTION: HOW TO DEFINE A PROPERTY WHICH WOULD BE READ-ONLY AND PERMANENT ONLY FROM THE JS SIDE?
 *
 * 1) THE SOLUTION PROBABLY LIES AT THE "SHAPE" LEVEL...
 
 * 2) USING JS_SetAllNonReservedSlotsToUndefined() PROVIDES SOME INSIGHTS:
 *    - SEE testObjectReset1() AND testObjectReset2()
 *
 * 3) NOTE THAT JS_ClearNonGlobalObject() IS NOT RECOMMENDED:
 *   - SEE https://bugzilla.mozilla.org/show_bug.cgi?id=1043281
 *
 * 4) INSIGHTFUL:
 *    - http://perfectionkills.com/understanding-delete
 *      - E.G.
 *        - "var foo = 33" WILL CREATE A NON-DELETABLE PROPERTY
 *        - BUT "bar = 55" WON'T!
 *    - http://www.2ality.com/2013/08/protecting-objects.html
 */

/*
 * FINDINGS:
 * 
 * 1) ONCE-DEFINED: READ-ONLY PROPERTIES CAN'T BE SET, EVEN VIA C++
 *
 * 2) FAILURE IS NOT DETECTED:
 *    - PROBABLY BECAUSE IT IS ONLY A "WARNING" (I.E. UNLESS IN "STRICT" MODE)
 *    - UNLIKE WITH PROPERTY DELETION: THERE IS NO JS_DefineProperty2
 */
void TestingJS::testReadOnlyProperty1()
{
    RootedValue value1(cx, NumberValue(55));
    JS_DefineProperty(cx, globalHandle(), "readonly1", value1, JSPROP_ENUMERATE | JSPROP_READONLY);
    
    RootedValue value2(cx, toValue("foo"));
    
    if (JS_SetProperty(cx, globalHandle(), "readonly1", value2))
    {
        RootedValue value3(cx);
        
        if (JS_GetProperty(cx, globalHandle(), "readonly1", &value3) && value3.isNumber())
        {
            JSP_CHECK(true, "FAILURE TO SET PROPERTY readonly1 IS NOT DETECTED");
        }
    }
    
    LOGI << "UNABLE TO SET (READ-ONLY) PROPERTY NOR TO DETECT FAILURE VIA C++" << endl;
}

/*
 * FINDINGS:
 *
 * 1) READ-ONLY PROPERTIES CAN BE DELETED VIA JS
 *    - UNLESS DEFINED AS PERMANENT
 */
void TestingJS::testReadOnlyProperty2()
{
    RootedValue value(cx, NumberValue(999));
    JS_DefineProperty(cx, globalHandle(), "readonly2", value, JSPROP_ENUMERATE | JSPROP_READONLY);
    
    executeScript("delete this.readonly2");
    
    if (hasOwnProperty(globalHandle(), "readonly2"))
    {
        JSP_CHECK(true, "PROPERTY readonly2 IS DELETABLE");
    }
    
    LOGI << "ABLE TO DELETE (READ-ONLY | NON-PERMANENT) PROPERTY VIA JS" << endl;
}

/*
 * FINDINGS:
 *
 * 1) PERMANENT PROPERTIES CAN'T BE DELETED, EVEN VIA C++
 *
 * 2) JS_DeleteProperty2 (I.E. JS_DeleteProperty) MUST BE USED
 *    - OTHERWISE: IT'S NOT POSSIBLE TO DETECT FAILURE
 */
void TestingJS::testPermanentProperty1()
{
    RootedValue value(cx, NumberValue(123));
    JS_DefineProperty(cx, globalHandle(), "permanent1", value, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT);
    
    bool success = false;
    
    if (!JS_DeleteProperty2(cx, globalHandle(), "permanent1", &success) && success)
    {
        JSP_CHECK(true, "PROPERTY permanent1 IS NOT DELETABLE");
    }
    
    LOGI << "UNABLE TO DELETE (PERMANENT) PROPERTY VIA C++" << endl;
}

/*
 * FINDINGS:
 *
 * 1) PERMANENT PROPERTIES CAN'T BE REDEFINED, EVEN VIA C++
 */
void TestingJS::testPermanentProperty2()
{
    RootedValue value(cx, NumberValue(123));
    JS_DefineProperty(cx, globalHandle(), "permanent2", value, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT);
    
    JS_DefineProperty(cx, globalHandle(), "permanent2", UndefinedHandleValue, JSPROP_ENUMERATE);
    
    bool success = false;
    
    if (!JS_DeleteProperty2(cx, globalHandle(), "permanent2", &success) && success)
    {
        JSP_CHECK(true, "PROPERTY permanent2 IS NOT DELETABLE");
    }
    
    LOGI << "UNABLE TO SET REDEFINE (PERMANENT) PROPERTY VIA C++" << endl;
}

// ---

void TestingJS::testObjectReset1()
{
    RootedObject object(cx);
    
    object = evaluateObject("({'foo': 456, bar: [1, {baz: 'xxx'}, 3]})");
    define(object, "permanent", 123, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT);

    /*
     * WILL SET (AMONG OTHERS) THE permanent (READ-ONLY) PROPERTY TO UNDEFINED
     */
    JS_SetAllNonReservedSlotsToUndefined(cx, object);
    JSP_CHECK(toSource(object) == "({foo:(void 0), bar:(void 0), permanent:(void 0)})");
    
    /*
     * HOWEVER: THE permanent PROPERTY STILL CAN'T BE DELETED
     */
    JSP_CHECK(!deleteProperty(object, "permanent"));

}

void TestingJS::testObjectReset2()
{
    if (false)
    {
        /*
         * WARNING: THIS WILL WIPE-OUT MUCH MORE THAN EXPECTED, E.G.
         * 
         * CONSTRUCTORS LIKE: Object, Date, ETC.
         * OBJECTS LIKE: JSON, ETC.
         * FUNCTIONS LIKE: escape(), decodeURI(), ETC.
         */
        JS_SetAllNonReservedSlotsToUndefined(cx, globalHandle());

        /*
         * AND OF COURSE MUCH OF THE STUFF DEFINED WITHIN Manager::init(), AS FOLLOWS:
         */
        
        try
        {
            executeScript("print(123)"); // TypeError: print is not a function
        }
        catch (exception &e)
        {}
        
        try
        {
            executeScript("var barker = new Barker('xxx');"); // TypeError: Barker is not a constructor
        }
        catch (exception &e)
        {}
    }
}

#pragma mark ---------------------------------------- CUSTOM CONSTRUCTION ----------------------------------------

const JSClass TestingJS::CustomClass1 =
{
    "CustomObject1",
    0,
    JS_PropertyStub,       // add
    JS_DeletePropertyStub, // delete
    JS_PropertyStub,       // get
    JS_StrictPropertyStub, // set
    JS_EnumerateStub,
    JS_ResolveStub,
    JS_ConvertStub,
};

const JSClass TestingJS::CustomClass2 =
{
    "CustomObject2",
    0,
    JS_PropertyStub,        // add
    JS_DeletePropertyStub,  // delete
    JS_PropertyStub,        // get
    JS_StrictPropertyStub,  // set
    JS_EnumerateStub,
    JS_ResolveStub,
    JS_ConvertStub,
    nullptr,
    nullptr,
    nullptr,
    CustomConstructor       // construct
};

bool TestingJS::CustomConstructor(JSContext *cx, unsigned argc, Value *vp)
{
    auto args = CallArgsFromVp(argc, vp);
    
    if (!args.isConstructing() || !args.calleev().isObject())
    {
        JS_ReportError(cx, "CONSTRUCTION FAILED");
        return false;
    }
    
    /*
     * CALLEE ANALYSIS: IS IT A FUNCTION OR AN OBJECT?
     */
    
    JSObject &callee = args.callee();
    const JSClass *classp;
    
    if (callee.is<JSFunction>())
    {
        if (!callee.as<JSFunction>().isNative())
        {
            JS_ReportError(cx, "CONSTRUCTION FAILED");
            return false;
        }
        
        /*
         * CALLED VIA JS
         */
        classp = &CustomClass1; // IMPOSSIBLE TO INFER JSClass
    }
    else
    {
        if (!callee.getClass()->construct)
        {
            JS_ReportError(cx, "CONSTRUCTION FAILED");
            return false;
        }
        
        /*
         * CALLED VIA C++
         */
        classp = js::Jsvalify(callee.getClass()); // I.E. CustomClass2
    }
    
    LOGI << "CONSTRUCTING " << classp->name << " WITH " << args.length() << " ARGUMENTS" << endl;
    
    JSObject *obj = JS_NewObjectForConstructor(cx, classp, args);
    
    if (!obj)
    {
        JS_ReportError(cx, "CONSTRUCTION FAILED");
        return false;
    }
    
    args.rval().setObject(*obj);
    return true;
}

/*
 * INSTANTIATION WITH ARGUMENTS, VIA JSNative CONSTRUCTOR
 */
void TestingJS::testCustomConstruction1()
{
    RootedObject prototype(cx, JS_InitClass(cx, globalHandle(), NullPtr(), &CustomClass1, CustomConstructor, 0, nullptr, nullptr, nullptr, nullptr));
    dumpObject(prototype);
    
    try
    {
        /*
         * REQUIRED FOR JS-SIDE INSTANTIATION:
         * CustomConstructor PASSED AS JSNative CONSTRUCTOR-ARGUMENT TO JS_InitClass()
         */
        evaluateObject("new CustomObject1(1, 'foo')");
    }
    catch (exception &e)
    {
        JSP_CHECK(false);
    }
   
    // ---
    
    AutoValueVector args(cx);
    args.append(toValue(255));
    args.append(toValue(nullptr));
    args.append(toValue("hello"));
    
    RootedValue value(cx);
    
    if (JS_GetProperty(cx, globalHandle(), "CustomObject1", &value))
    {
        if (value.isObject())
        {
            RootedObject constructor(cx, &value.toObject());
            JSP_CHECK(JS_New(cx, constructor, args));
        }
    }
}

/*
 * INSTANTIATION WITH ARGUMENTS, VIA construct HOOK
 */
void TestingJS::testCustomConstruction2()
{
    RootedObject prototype(cx, JS_InitClass(cx, globalHandle(), NullPtr(), &CustomClass2, nullptr, 0, nullptr, nullptr, nullptr, nullptr));
    dumpObject(prototype);
    
    /*
     * JUST TO ILLUSTRATE THE POSSIBILITY OF USING AutoValueArray...
     *
     * BETTER ALTERNATIVES:
     * - FOR MULTIPLE ARGUMENTS: AutoValueVector OR AutoWrappedValueVector
     * - FOR SINGLE ARGUMENTS: RootedValue
     */
    AutoValueArray<1> args(cx);
    args[0].setNumber(33.0);
    
    /*
     * REQUIRED IF JS_InitClass() WAS NOT CALLED WITH JSNative CONSTRUCTOR-ARGUMENT:
     * CustomConstructor DEFINED AS construct HOOK IN JSClass DECLARATION
     */
    JSP_CHECK(JS_New(cx, prototype, args));
    
    // ---
    
    RootedValue value(cx);
    
    if (JS_GetProperty(cx, globalHandle(), "CustomObject2", &value))
    {
        if (value.isObject())
        {
            RootedObject constructor(cx, &value.toObject());
            JSP_CHECK(JS_New(cx, constructor, args));
        }
    }
}

// ---

void TestingJS::testNewObject()
{
    /*
     * PASSING A SINGLE RootedValue IS VALID:
     * https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/JSAPI_reference/JS::HandleValueArray
     */
    
    RootedValue arg1(cx, toValue(16.67));
    JSP_CHECK(toSource(newObject("Number", arg1)) == "(new Number(16.67))");
    
    RootedValue arg2(cx, toValue("2014-09-27T21:14:32.695Z"));
    JSP_CHECK(toSource(newObject("Date", arg2)) == "(new Date(1411852472695))");

    // ---
    
    /*
     * PASSING NO ARGUMENTS IS OKAY TOO
     */
    
    JSObject *barker = newObject("Barker");
    JSP_CHECK(Barker::bark(barker));
    
    // ---

    /*
     * SILENTLY FAILS
     */
    JSP_CHECK(!newObject("xxx"));

    /*
     * JS COMPLAINS THAT "function print() ... is not a constructor"
     */
    JSP_CHECK(!newObject("print"));
    
    // ---
    
    /*
     * INSTANTIATING PURE JS CUSTOM OBJECT...
     */
    
    executeScript("\
                  var Person = function(name) { this.name = name; };\
                  Person.prototype.sayHello = function() { return 'my name is ' + this.name; };\
                  ");
    
    RootedValue arg(cx, toValue("Luka"));
    RootedObject person(cx, newObject("Person", arg));
    
    RootedValue result(cx, call(person, "sayHello"));
    JSP_CHECK(compare(result, "my name is Luka"));
}

#pragma mark ---------------------------------------- CUSTOM GETTERS / SETTERS ----------------------------------------

/*
 * REFERENCES:
 *
 * - https://github.com/arielm/Spidermonkey/blob/chr_31/js/src/jsapi-tests/testDefineGetterSetterNonEnumerable.cpp
 */

static bool getter1(JSContext *cx, unsigned argc, Value *vp)
{
    auto args = CallArgsFromVp(argc, vp);
    
    LOGI << "getter1 CALLED" << endl;
    
    args.rval().setInt32(33);
    return true;
}

void TestingJS::testGetter1()
{
    RootedObject function(cx, JS_GetFunctionObject(JS_NewFunction(cx, getter1, 0, 0, NullPtr(), "get")));
    
    JS_DefineProperty(cx, globalHandle(), "oneR",
                      JS::UndefinedHandleValue, JSPROP_GETTER | JSPROP_PERMANENT,
                      JS_DATA_TO_FUNC_PTR(JSPropertyOp, (JSObject*)function));
    
    // ---
    
    executeScript("var test = oneR; print(test)");
}

// ---

static bool setter1(JSContext *cx, unsigned argc, Value *vp)
{
    auto args = CallArgsFromVp(argc, vp);
    
    if (args.hasDefined(0))
    {
        LOGI << "setter1 CALLED WITH: " << JSP::toString(args[0]) << endl;
        
        args.rval().setUndefined();
        return true;
    }
    
    return false;
}

void TestingJS::testSetter1()
{
    RootedObject function(cx, JS_GetFunctionObject(JS_NewFunction(cx, setter1, 0, 0, NullPtr(), "set")));
    
    JS_DefineProperty(cx, globalHandle(), "oneW",
                      JS::UndefinedHandleValue, JSPROP_SETTER | JSPROP_PERMANENT,
                      nullptr,
                      JS_DATA_TO_FUNC_PTR(JSStrictPropertyOp, (JSObject*)function));
    
    // ---
    
    executeScript("oneW = 255");
}

// ---

static int getterSetterValue1 = 199;

static bool getterSetter1(JSContext *cx, unsigned argc, Value *vp)
{
    auto args = CallArgsFromVp(argc, vp);
    
    if (args.hasDefined(0))
    {
        LOGI << "getterSetter1: setter CALLED WITH: " << JSP::toString(args[0]) << endl;

        if (args[0].isInt32())
        {
            getterSetterValue1 = args[0].toInt32();
            
            args.rval().setUndefined();
            return true;
        }
    }
    else
    {
        LOGI << "getterSetter1: getter CALLED" << endl;
        
        args.rval().setInt32(getterSetterValue1);
        return true;
    }
    
    return false;
}

void TestingJS::testGetterSetter1()
{
    RootedObject getterFunction(cx, JS_GetFunctionObject(JS_NewFunction(cx, getterSetter1, 0, 0, NullPtr(), "get")));
    RootedObject setterFunction(cx, JS_GetFunctionObject(JS_NewFunction(cx, getterSetter1, 0, 0, NullPtr(), "set")));
    
    JS_DefineProperty(cx, globalHandle(), "oneRW",
                      JS::UndefinedHandleValue, JSPROP_GETTER | JSPROP_SETTER | JSPROP_PERMANENT,
                      JS_DATA_TO_FUNC_PTR(JSPropertyOp, (JSObject*)getterFunction),
                      JS_DATA_TO_FUNC_PTR(JSStrictPropertyOp, (JSObject*)setterFunction));
    
    // ---
    
    executeScript("var test = ++oneRW; print(test);");
}

#pragma mark ---------------------------------------- SHAPES ----------------------------------------

/*
 * MUST READ:
 * https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/Internals/Property_cache
 *
 * MORE:
 * https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/vm/Shape.h#L37-100
 * https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/vm/Shape.h#L469-514
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
                    vp.set(obj2->nativeGetSlot(shape->slot())); // PROBABLY THE BRIDGE TO ("NATURALLY" NON-ACCESSIBLE) "RESERVED SLOTS" FROM THE JS SIDE
                    
                    LOGI << JSP::write(id) << ": " << write(vp) << endl;
                }
            }
        }
    }
    
    LOGI << endl;
}

void TestingJS::testShapes2()
{
    JSObject *object1 = evaluateObject("({one: 1, two: 'deux', trois: [1, 2, 3]})");
    dumpObject(object1);
    LOGI << "-----" << endl;
    
    JSObject *object2 = evaluateObject("({one: 250, two: 'dos', trois: [0]})");
    dumpObject(object2);
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
 * - THE "IDENTIFIER MECHANISM" CAN BE USED TO DIFFERENTIATE BETWEEN ATOMS LIKE property1 AND property-two
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
    dumpAtom(atom1);
    dumpAtom(atom2);
    
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
    dumpObject(object);
    
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
    RootedObject object(cx, newPlainObject());
    
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
 * JS::Evaluate (USED BY Proto::evaluateObject) WILL FAIL IF NO obj PARAMETER IS PROVIDED
 * I.E. WE SHOULD CONTINUE USING THE GLOBAL-OBJECT BY DEFAULT
 *
 * FOLLOW-UP:
 * - MORE INFO TO DIGEST (TODO): https://github.com/ricardoquesada/Spidermonkey/blob/master/js/src/jsapi.h#L3671-3703
 */

void TestingJS::testEvaluationScope()
{
    JSObject *newObject = evaluateObject("({x: 250.33, foo: [1, 2, 3], bar: 'baz'})");
    dumpObject(newObject);
}

/*
 * CONCLUSIONS:
 *
 * SCOPE:
 * SEE COMMENTS REGARDING HOW JS_CallFunctionValue (USED BY Proto::call)
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
    dumpObject(containerObject);
    
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
        JS::RootedValue result1(cx, call(NullPtr(), function1, args));
        LOGI << "RESULT 1: " << write(result1) << endl; // RESULT: 0
    }
    
    if (getProperty(containerObject, "function2", &function2))
    {
        /*
         * IF containerObject IS PROVIDED:
         * this.someAmbiguousThing WILL RESOLVE TO THE "LOCAL VALUE"
         */
        RootedValue result2A(cx, call(containerObject, function2, args));
        LOGI << "RESULT 2A: " << write(result2A) << endl; // RESULT: 99
        
        /*
         * IF nullptr OR NOTHING (I.E. THE GLOBAL-OBJECT) IS PROVIDED:
         * this.someAmbiguousThing WILL RESOLVE TO THE "GLOBAL VALUE"
         */
        RootedValue result2B(cx, call(NullPtr(), function2, args));
        LOGI << "RESULT 2B: " << write(result2B) << endl; // RESULT: 66
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

// ---

void TestingJS::testParsing1()
{
    string source = utils::readText<string>(InputSource::getAsset("config.json"));
    
    initComplexJSON(source);
    string js = evaluateString("JSON.stringify(JSON.parse(complexJSON), null, 2)");

    JSObject *parsed = parse(source);
    string cpp = stringify(parsed);
    
    JSP_CHECK(js == cpp);
}

/*
 * "JSON.parse() does not allow trailing commas"
 *
 * REFERENCE: https://developer.mozilla.org/en/docs/Web/JavaScript/Reference/Global_Objects/JSON/parse
 */
void TestingJS::testParsing2()
{
    try
    {
        executeScript("JSON.parse('[1, 2, 3,]')");
        JSP_CHECK(false); // UNREACHABLE DUE TO SyntaxError
    }
    catch (exception &e)
    {}
    
    // ---

    JSObject *parsed = parse("[1, 2, 3,]");
    JSP_CHECK(!parsed); // NULL RESULT DUE TO SyntaxError
}

void TestingJS::initComplexJSON(const string &source)
{
    RootedValue value(cx, toValue(source));
    setProperty(globalHandle(), "complexJSON", value);
}

//

/*
 * WORKS BECAUSE THERE ARE NO "CYCLIC VALUES" IN OBJECT
 * OTHERWISE: RETURNS EMPTY-STRING AND REPORTS JS-EXCEPTION ("TypeError: cyclic object value")
 */
void TestingJS::testStringify()
{
    initComplexJSObject();
    
    /*
     * USING stringify VIA JS
     */
    string js = evaluateString("JSON.stringify(complexObject, null, 2)");
    
    /*
     * USING stringify VIA C++
     */
    string cpp = stringify(get<OBJECT>(globalHandle(), "complexObject"));
    
    JSP_CHECK(js == cpp);
}

/*
 * WORKS BECAUSE OBJECT IS NOT "HUGE"
 * OTHERWISE: RETURNS EMPTY-STRING AND REPORTS JS-EXCEPTION ("InternalError: allocation size overflow")
 */
void TestingJS::testToSource()
{
    initComplexJSObject();
    
    /*
     * USING toSource() VIA JS
     */
    string js = evaluateString("complexObject.toSource()");
    
    /*
     * USING toSource() VIA C++
     */
    string cpp = toSource(get<OBJECT>(globalHandle(), "complexObject"));
    
    JSP_CHECK(js == cpp);
}

void TestingJS::initComplexJSObject()
{
    if (!hasOwnProperty(globalHandle(), "complexObject"))
    {
        executeScript(InputSource::getAsset("handlebars.js"));
        JSObject *complexObject = get<OBJECT>(globalHandle(), "Handlebars");
        
        set(globalHandle(), "complexObject", complexObject);
    }
}
