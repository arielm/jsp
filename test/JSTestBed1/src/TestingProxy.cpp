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

void TestingProxy::performRun(bool force)
{
    JSP_TEST(force || true, testPeers1);
    JSP_TEST(force || true, testNativeCalls1);
    JSP_TEST(force || true, testHandler1);
}

// ---

void TestingProxy::testPeers1()
{
    {
        Proxy vanilla1; // peers.Proxy[1]
        Proxy singleton("ScriptManager", true); // peers.ScriptManager
        
        try
        {
            /*
             * ALLOWED
             */
            executeScript("peers.Proxy[0].bar = 'baz';");
            JSP_CHECK(get<STRING>(peer, "bar") == "baz");
            
            /*
             * INTENTIONALLY NOT ALLOWED
             */
            executeScript("peers.Proxy[1] = 255; peers.ScriptManager = 456");
            
            /*
             * SHOULD NOT BE ALLOWED AS WELL (I.E. ONLY GENERATE A "WARNING")
             */
            executeScript("peers.foo = 123");
        }
        catch (exception &e)
        {
            LOGI << e.what() << endl;
        }
        
        JSP_CHECK(toSource(get<OBJECT>(globalHandle(), "peers")) == "({Proxy:[{bar:\"baz\"}, {}], ScriptManager:{}, foo:123})");
    }
    
    JSP_CHECK(toSource(get<OBJECT>(globalHandle(), "peers")) == "({Proxy:[{bar:\"baz\"}, ,], foo:123})");
}

// ---

static bool staticMethod1(const CallArgs &args)
{
    if (args.hasDefined(0) && args[0].isNumber())
    {
        args.rval().set(NumberValue(args[0].toNumber() * -1));
        return true;
    }
    
    return false;
}

bool TestingProxy::instanceMethod1(const CallArgs &args)
{
    if (args.hasDefined(0) && args[0].isNumber())
    {
        args.rval().set(NumberValue(args[0].toNumber() * instanceValue1));
        return true;
    }
    
    return false;
}

void TestingProxy::testNativeCalls1()
{
    registerNativeCall("staticMethod1", BIND_STATIC1(staticMethod1));
    registerNativeCall("instanceMethod1", BIND_INSTANCE1(&TestingProxy::instanceMethod1, this));
    
    registerNativeCall("lambda1", [=](const CallArgs &args)->bool
    {
        if (args.hasDefined(0) && args[0].isNumber())
        {
            args.rval().set(NumberValue(args[0].toNumber() * instanceValue1));
            return true;
        }

        return false;
    });
    
    executeScript("var target = peers.Proxy[0]; print(target.staticMethod1(77), target.instanceMethod1(11), target.lambda1(33))");
    
    // ---

    unregisterNativeCall("staticMethod1");
    executeScript("try { print(target.staticMethod1(33)); } catch(e) { print(e);}");
}

// ---

struct Record
{
    vector<string> values;
    
    Record(const vector<string> &values)
    :
    values(values)
    {}
    
    string write() const
    {
        string buffer;
        int count = 0;
        
        for (const auto &value : values)
        {
            if (count++) buffer += ' ';
            buffer += value;
        }
        
        return buffer;
    }
};

class Handler1 : public Proxy
{
public:
    vector<Record> records;
    
    string getRecord(size_t index)
    {
        if (index < records.size())
        {
            return records[index].write();
        }
        
        return "";
    }
    
    // ---
    
    JSObject* newArray(size_t length = 0) final
    {
        records.emplace_back(Record({"newArray", ci::toString(length)}));
        return Proto::newArray(length);
    }
    
    bool setElement(HandleObject array, int index, HandleValue value) final
    {
        records.emplace_back(Record({"setElement", ci::toString(index), jsp::toString(value)}));
        return Proto::setElement(array, index, value);
    }
};

void TestingProxy::testHandler1()
{
    Handler1 handler;
    setHandler(&handler);
    
    RootedObject array(cx, newArray());
    setElements(array, vector<FLOAT64> {1.33, 2.33});
    set(array, 2, "foo");
    
    setHandler(nullptr);
    
    // ---
    
    JSP_CHECK(handler.getRecord(0) == "newArray 0");
    JSP_CHECK(handler.getRecord(1) == "setElement 0 1.33");
    JSP_CHECK(handler.getRecord(2) == "setElement 1 2.33");
    JSP_CHECK(handler.getRecord(3) == "setElement 2 foo");
    
    JSP_CHECK(toSource(array) == "[1.33, 2.33, \"foo\"]");
}
