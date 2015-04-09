/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "TestingWrappedValue.h"

#include "chronotext/Context.h"

using namespace std;
using namespace ci;
using namespace chr;

using namespace jsp;

void TestingWrappedValue::performSetup()
{
    WrappedValue::LOG_VERBOSE = false;
    WrappedObject::LOG_VERBOSE = false;
}

void TestingWrappedValue::performShutdown()
{
    WrappedValue::LOG_VERBOSE = false;
    WrappedObject::LOG_VERBOSE = false;
}

void TestingWrappedValue::performRun(bool force)
{
    if (force || true)
    {
        JSP_TEST(force || true, testStackCreationAndAssignment);
        JSP_TEST(force || true, testAutomaticConversion);
    }
    
    if (force || true)
    {
        JSP_TEST(force || true, testObjectStackRooting1);
        JSP_TEST(force || true, testObjectStackRooting2);
        JSP_TEST(force || true, testStringStackRooting1);
        JSP_TEST(force || true, testStringStackRooting2);
    }
    
    if (force || true)
    {
        JSP_TEST(force || true, testValueComparison);
        JSP_TEST(force || true, testObjectComparison);
        
        JSP_TEST(force || true, testBooleanComparison);
        JSP_TEST(force || true, testStringComparison);
        JSP_TEST(force || true, testAutomaticComparison);
    }
    
    if (force || true)
    {
        JSP_TEST(force || true, testRootedComparison);
        JSP_TEST(force || true, testHeapComparison);
    }
    
    if (force || true)
    {
        JSP_TEST(force || true, testAutoWrappedValueVector);
    }
}

// ---

void TestingWrappedValue::testStackCreationAndAssignment()
{
    WrappedValue wrapped;
    JSP_CHECK(wrapped.isUndefined());
    
    wrapped = NumberValue(55.55);
    JSP_CHECK(wrapped.toDouble() == 55.55);
    
    wrapped = Int32Value(-255);
    JSP_CHECK(wrapped.toInt32() == -255);
    
    wrapped = NumberValue(0xff123456);
    JSP_CHECK(wrapped.toNumber() == 0xff123456);

    wrapped = FalseValue();
    JSP_CHECK(wrapped.isFalse());
    
    wrapped = NullValue();
    JSP_CHECK(wrapped.isNull());

    // --
    
    wrapped = ObjectOrNullValue(Barker::create("ASSIGNED-TO-VALUE 1"));
    JSP_CHECK(JSP::writeGCDescriptor(wrapped) == 'n'); // I.E. IN NURSERY
    
    JSP::forceGC();
    JSP_CHECK(!JSP::isHealthy(wrapped)); // REASON: GC-THING NOT ROOTED
    
    // ---
    
    wrapped = toValue("assigned-to-value 1");
    JSP_CHECK(JSP::writeGCDescriptor(wrapped) == 'W'); // I.E. TENURED
    
    JSP::forceGC();
    JSP_CHECK(!JSP::isHealthy(wrapped)); // REASON: GC-THING NOT ROOTED
}

void TestingWrappedValue::testAutomaticConversion()
{
    WrappedValue wrapped;
    
    wrapped = 2.5f;
    JSP_CHECK(float(wrapped.toDouble()) == 2.5f);

    wrapped = 55.55;
    JSP_CHECK(wrapped.toDouble() == 55.55);

    wrapped = -255;
    JSP_CHECK(wrapped.toInt32() == -255);
    
    wrapped = 0xff123456;
    JSP_CHECK(wrapped.toNumber() == 0xff123456);
    
    wrapped = false;
    JSP_CHECK(wrapped.isFalse());
    
    wrapped = nullptr;
    JSP_CHECK(wrapped.isNull());
    
    wrapped = newPlainObject();
    JSP_CHECK(wrapped.isObject());
    
    wrapped = "from const char*";
    JSP_CHECK(js::StringEqualsAscii(wrapped.toString()->ensureLinear(cx), "from const char*"));
    
    wrapped = string("from string");
    JSP_CHECK(js::StringEqualsAscii(wrapped.toString()->ensureLinear(cx), "from string"));
}

// ---

void TestingWrappedValue::testObjectStackRooting1()
{
    RootedObject rootedObject(cx, Barker::create("STACK-ROOTED STANDALONE"));
    
    JSP::forceGC();
    JSP_CHECK(Barker::bark("STACK-ROOTED STANDALONE")); // REASON: GC-THING ROOTED
}

void TestingWrappedValue::testObjectStackRooting2()
{
    Rooted<WrappedValue> rootedWrapped(cx, Barker::create("STACK-ROOTED VIA-VALUE"));
    
    JSP::forceGC();
    JSP_CHECK(Barker::bark("STACK-ROOTED VIA-VALUE")); // REASON: GC-THING ROOTED
}

void TestingWrappedValue::testStringStackRooting1()
{
    RootedString rootedString(cx, toJSString("stack-rooted standalone"));
    WrappedValue wrapped(StringValue(rootedString));
    
    JSP::forceGC();
    JSP_CHECK(JSP::isHealthy(wrapped)); // REASON: GC-THING ROOTED
}

void TestingWrappedValue::testStringStackRooting2()
{
    Rooted<WrappedValue> rootedWrapped(cx, "stack-rooted via-value");
    
    JSP::forceGC();
    JSP_CHECK(JSP::isHealthy(rootedWrapped.get())); // REASON: GC-THING ROOTED
}

// ---

void TestingWrappedValue::testValueComparison()
{
    WrappedValue wrapped1 = NumberValue(33.33);
    JSP_CHECK(NumberValue(33.33) == wrapped1);
    JSP_CHECK(wrapped1 == NumberValue(33.33)); // NO TEMPORARIES, THANKS TO WrappedValue::operator==(const Value&)
    
    WrappedValue wrapped2;
    JSP_CHECK(wrapped2 != wrapped1); // NO TEMPORARIES, THANKS TO WrappedValue::operator==(const WrappedValue&)
    
    wrapped2 = NumberValue(33.33);
    JSP_CHECK(wrapped2 == wrapped1); // NO TEMPORARIES, THANKS TO WrappedValue::operator!=(const WrappedValue&)
}

void TestingWrappedValue::testObjectComparison()
{
    JSObject *object1 = Barker::create("BARKER 1");
    JSObject *object2 = Barker::create("BARKER 2");

    WrappedValue wrapped1(object1);
    WrappedValue wrapped2(object2);
    
    JSP_CHECK(wrapped1 == object1); // NO TEMPORARIES, THANKS TO WrappedValue::operator==(const JSObject*)
    
    JSP_CHECK(wrapped1 != object2); // NO TEMPORARIES, THANKS TO WrappedValue::operator!=(const JSObject*)
    JSP_CHECK(wrapped1 != wrapped2);

    wrapped2 = object1;
    JSP_CHECK(wrapped1 == wrapped2);
}

void TestingWrappedValue::testBooleanComparison()
{
    WrappedValue wrapped2 = true;
    JSP_CHECK(wrapped2 == true); // THANKS TO WrappedValue::operator==(bool)
    JSP_CHECK(wrapped2 != false); // THANKS TO WrappedValue::operator!=(bool)
    
    JSP_CHECK(wrapped2); // THANKS TO WrappedValue::operator const bool ()
    
    wrapped2 = false;
    JSP_CHECK(!wrapped2); // THANKS TO WrappedValue::operator const bool ()
}

void TestingWrappedValue::testStringComparison()
{
    WrappedValue wrapped = "foo";
    JSP_CHECK(wrapped == "foo"); // THANKS TO WrappedValue::operator==(const char*)
    JSP_CHECK(wrapped != "FOO"); // THANKS TO WrappedValue::operator!=(const char*)

    JSP_CHECK(WrappedValue("bar") == toValue("bar")); // THANKS TO WrappedValue::operator==(const Value&)
    JSP_CHECK(WrappedValue("BAR") != toValue("bar")); // THANKS TO WrappedValue::operator!=(const Value&)
}

void TestingWrappedValue::testAutomaticComparison()
{
    WrappedValue wrapped = 33.33;
    JSP_CHECK(wrapped == 33.33);
    JSP_CHECK(wrapped != 55.55);
    
    wrapped = -255;
    JSP_CHECK(wrapped == -255);
    JSP_CHECK(wrapped != 127);
    
    JSP_CHECK(wrapped != "foo");
    JSP_CHECK(wrapped != nullptr);
    
    wrapped = 0xff123456; // VALUES > 0x7fffffff ARE PROPERLY CONSIDERED AS UNSIGNED
    JSP_CHECK(wrapped == 0xff123456);
    JSP_CHECK(wrapped != 0x12345678);

    JSP_CHECK(wrapped); // XXX

    wrapped = 0;
    JSP_CHECK(!wrapped); // XXX
}

// ---

void TestingWrappedValue::testRootedComparison()
{
    Rooted<WrappedValue> rootedWrapped1A(cx, 123);
    Rooted<WrappedValue> rootedWrapped1B(cx, 123);
    JSP_CHECK(rootedWrapped1A == rootedWrapped1B);

    Rooted<WrappedValue> rootedWrapped2A(cx, "hello");
    Rooted<WrappedValue> rootedWrapped2B(cx, "hello");
    JSP_CHECK(rootedWrapped2A == rootedWrapped2B);
    
    JSObject *barker = Barker::create("BARKER 3");
    Rooted<WrappedValue> rootedWrapped3A(cx, barker);
    Rooted<WrappedValue> rootedWrapped3B(cx, barker);
    JSP_CHECK(rootedWrapped3A == rootedWrapped3B);
}

void TestingWrappedValue::testHeapComparison()
{
    Heap<WrappedValue> heapWrapped1A(123);
    Heap<WrappedValue> heapWrapped1B(123);
    JSP_CHECK(heapWrapped1A == heapWrapped1B, "EQUALITY");
    
    Heap<WrappedValue> heapWrapped2A("hello");
    Heap<WrappedValue> heapWrapped2B("hello");
    JSP_CHECK(heapWrapped2A == heapWrapped2B, "EQUALITY");
    
    JSObject *barker = Barker::create("BARKER 4");
    Heap<WrappedValue> heapWrapped3A(barker);
    Heap<WrappedValue> heapWrapped3B(barker);
    JSP_CHECK(heapWrapped3A == heapWrapped3B, "EQUALITY");
}

// ---

void TestingWrappedValue::testAutoWrappedValueVector()
{
    AutoWrappedValueVector args;
    
    args.append(123);
    args.append("foo");
    args.append(Barker::create("AUTO-WRAPPED"));
    args.append(33.33);
    
    /*
     * STRING AND BARKER ARE ROOTED VIA THE AutoWrappedValueVector
     */
    JSP::forceGC();
    
    call(globalHandle(), "print", args); // TODO: "CAPTURE" LOG
}

/*
WrappedValue wrapped;

if (true)
{
    RootedString rooted(cx, toJSString("not atomized"));
    wrapped = StringValue(rooted);
    
    JSP::forceGC();
    
    if (JSP_CHECK(JSP::isHealthy(wrapped.get()), "HEALTHY VALUE"))
    {
        JSP_CHECK(JSP::writeGCDescriptor(wrapped) == 'B');
    }
}
else
{
//  TODO: TEST StringIsPermanentAtom(JSString *str) IN barrier.h

    string atomized = "atomized";
    wrapped = StringValue(Atomize(cx, atomized.data(), atomized.size()));
    
    JSP::forceGC();
    
    if (JSP_CHECK(!JSP::isHealthy(wrapped.get()), "UNHEALTHY VALUE")) // REASON: ATOMS ARE NOT ROOTED BY DEFAULT
    {
        JSP_CHECK(JSP::writeGCDescriptor(wrapped) == 'P');
    }
}
*/
