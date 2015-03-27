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
    WrappedValue::LOG_VERBOSE = true;
    WrappedObject::LOG_VERBOSE = true;
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
        /*
         * TODO:
         *
         * 1) USE STACK-ROOTED, HEAP-ROOTED, ETC. IN COMPARISONS
         */
        
        JSP_TEST(force || true, testValueComparison);
        JSP_TEST(force || true, testObjectComparison);
        
        JSP_TEST(force || true, testBooleanComparison);
        JSP_TEST(force || true, testStringComparison);
        JSP_TEST(force || true, testAutomaticComparison);
    }

    /*
     * TODO:
     *
     * 1) TEST ARRAY-ASSIGNMENT IN Proto
     */
}

// ---

void TestingWrappedValue::testStackCreationAndAssignment()
{
    WrappedValue wrapped;
    JSP_CHECK(JSP::write(wrapped) == "undefined");
    
    wrapped = NumberValue(55.55);
    JSP_CHECK(JSP::write(wrapped) == "55.55");

    // --
    
    wrapped = Barker::construct("ASSIGNED-TO-VALUE 1");
    JSP_CHECK(JSP::writeGCDescriptor(wrapped) == 'n'); // I.E. IN NURSERY
    
    JSP::forceGC();
    JSP_CHECK(!JSP::isHealthy(wrapped.get()), "UNHEALTHY VALUE"); // REASON: GC-THING NOT ROOTED
    
    // ---
    
    wrapped = toValue("assigned-to-value 1");
    JSP_CHECK(JSP::writeGCDescriptor(wrapped) == 'W'); // I.E. TENURED
    
    JSP::forceGC();
    JSP_CHECK(!JSP::isHealthy(wrapped.get()), "UNHEALTHY VALUE"); // REASON: GC-THING NOT ROOTED
}

void TestingWrappedValue::testAutomaticConversion()
{
    WrappedValue wrapped;
    JSP_CHECK(JSP::write(wrapped) == "undefined");
    
    wrapped = 55.55;
    JSP_CHECK(JSP::write(wrapped) == "55.55");

    wrapped = 255;
    JSP_CHECK(JSP::write(wrapped) == "255");
    
    wrapped = false;
    JSP_CHECK(JSP::write(wrapped) == "false");
    
    wrapped = (JSObject*)nullptr; // TODO: CONSIDER HANDLING nullptr_t IN WrappedValue
    JSP_CHECK(JSP::write(wrapped) == "null");
    
    // --
    
    wrapped = Barker::construct("ASSIGNED-TO-VALUE 2");
    JSP_CHECK(JSP::writeGCDescriptor(wrapped) == 'n'); // I.E. IN NURSERY
    
    JSP::forceGC();
    JSP_CHECK(!JSP::isHealthy(wrapped.get()), "UNHEALTHY VALUE"); // REASON: GC-THING NOT ROOTED
    
    // ---
    
    wrapped = "assigned-to-value 2";
    JSP_CHECK(JSP::writeGCDescriptor(wrapped) == 'W'); // I.E. TENURED
    
    JSP::forceGC();
    JSP_CHECK(!JSP::isHealthy(wrapped.get()), "UNHEALTHY VALUE"); // REASON: GC-THING NOT ROOTED
}

// ---

void TestingWrappedValue::testObjectStackRooting1()
{
    RootedObject rootedObject(cx, Barker::construct("STACK-ROOTED STANDALONE"));
    WrappedValue wrapped(rootedObject.get());
    
    JSP::forceGC();
    JSP_CHECK(Barker::bark("STACK-ROOTED STANDALONE"), "HEALTHY BARKER"); // REASON: GC-THING ROOTED
}

void TestingWrappedValue::testObjectStackRooting2()
{
    Rooted<WrappedValue> rootedWrapped(cx, Barker::construct("STACK-ROOTED VIA-VALUE"));
    
    JSP::forceGC();
    JSP_CHECK(Barker::bark("STACK-ROOTED VIA-VALUE"), "HEALTHY BARKER"); // REASON: GC-THING ROOTED
}

void TestingWrappedValue::testStringStackRooting1()
{
    RootedString rootedString(cx, toJSString("stack-rooted standalone"));
    WrappedValue wrapped(StringValue(rootedString));
    
    JSP::forceGC();
    
    if (JSP_CHECK(JSP::isHealthy(wrapped.get()), "HEALTHY VALUE")) // REASON: GC-THING ROOTED
    {
        JSP_CHECK(JSP::writeGCDescriptor(wrapped) == 'B');
    }
}

void TestingWrappedValue::testStringStackRooting2()
{
    Rooted<WrappedValue> rootedWrapped(cx, toValue("stack-rooted via-value"));
    
    JSP::forceGC();
    
    if (JSP_CHECK(JSP::isHealthy(rootedWrapped.get()), "HEALTHY VALUE")) // REASON: GC-THING ROOTED
    {
        JSP_CHECK(JSP::writeGCDescriptor(rootedWrapped.get()) == 'B');
    }
}

// ---

void TestingWrappedValue::testValueComparison()
{
    WrappedValue wrapped1 = NumberValue(33.33);
    JSP_CHECK(NumberValue(33.33) == wrapped1, "EQUALITY");
    JSP_CHECK(wrapped1 == NumberValue(33.33), "EQUALITY"); // NO TEMPORARIES, THANKS TO WrappedValue::operator==(const Value&)
    
    WrappedValue wrapped2;
    JSP_CHECK(wrapped2 != wrapped1, "INEQUALITY"); // NO TEMPORARIES, THANKS TO WrappedValue::operator==(const WrappedValue&)
    
    wrapped2 = NumberValue(33.33);
    JSP_CHECK(wrapped2 == wrapped1, "EQUALITY"); // NO TEMPORARIES, THANKS TO WrappedValue::operator!=(const WrappedValue&)
}

void TestingWrappedValue::testObjectComparison()
{
    JSObject *object1 = Barker::construct("BARKER 1");
    JSObject *object2 = Barker::construct("BARKER 2");

    WrappedValue wrapped1(object1);
    WrappedValue wrapped2(object2);
    
    JSP_CHECK(wrapped1 == object1, "EQUALITY"); // NO TEMPORARIES, THANKS TO WrappedValue::operator==(const JSObject*)
    
    JSP_CHECK(wrapped1 != object2, "INEQUALITY"); // NO TEMPORARIES, THANKS TO WrappedValue::operator!=(const JSObject*)
    JSP_CHECK(wrapped1 != wrapped2, "INEQUALITY");

    wrapped2 = object1;
    JSP_CHECK(wrapped1 == wrapped2, "EQUALITY");
}

void TestingWrappedValue::testBooleanComparison()
{
    WrappedValue wrapped2 = true;
    JSP_CHECK(wrapped2 == true, "EQUALITY"); // THANKS TO WrappedValue::operator==(bool)
    JSP_CHECK(wrapped2 != false, "INEQUALITY"); // THANKS TO WrappedValue::operator!=(bool)
    
    JSP_CHECK(wrapped2, "TRUE"); // THANKS TO WrappedValue::operator const bool ()
    
    wrapped2 = false;
    JSP_CHECK(!wrapped2, "FALSE"); // THANKS TO WrappedValue::operator const bool ()
}

void TestingWrappedValue::testStringComparison()
{
    WrappedValue wrapped = "foo";
    JSP_CHECK(wrapped == "foo", "EQUALITY"); // THANKS TO WrappedValue::operator==(const char*)
    JSP_CHECK(wrapped != "FOO", "INEQUALITY"); // THANKS TO WrappedValue::operator!=(const char*)

    JSP_CHECK(WrappedValue("bar") == toValue("bar"), "EQUALITY"); // THANKS TO WrappedValue::operator==(const Value&)
    JSP_CHECK(WrappedValue("BAR") != toValue("bar"), "INEQUALITY"); // THANKS TO WrappedValue::operator!=(const Value&)
}

void TestingWrappedValue::testAutomaticComparison()
{
    WrappedValue wrapped = 33.33;
    JSP_CHECK(wrapped == 33.33, "EQUALITY");
    JSP_CHECK(wrapped != 55.55, "INEQUALITY");
    
    wrapped = 255;
    JSP_CHECK(wrapped == 255, "EQUALITY");
    JSP_CHECK(wrapped != 127, "INEQUALITY");
    
    JSP_CHECK(wrapped != "foo", "FALSE");
    JSP_CHECK(wrapped != (JSObject*)nullptr, "FALSE"); // TODO: CONSIDER HANDLING nullptr_t IN WrappedValue
    
    wrapped = 0xff123456; // VALUES > 0x7fffffff ARE PROPERLY CONSIDERED AS UNSIGNED (TODO: TEST ON 32-BIT SYSTEMS)
    JSP_CHECK(wrapped == 0xff123456, "EQUALITY");
    JSP_CHECK(wrapped != 0x12345678, "INEQUALITY");

    JSP_CHECK(wrapped, "FALSE");

    wrapped = 0;
    JSP_CHECK(!wrapped, "FALSE");
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
