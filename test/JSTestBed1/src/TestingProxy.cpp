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
    if (force || true)
    {
        JSP_TEST(force || true, testPeers1); // SHOULD BE EXECUTED FIRST BECAUSE IT ASSUMES peers.Proxy[0] IS THE LAST Proxy INSTANCE
        JSP_TEST(force || true, testPeers2);
    }
    
    if (force || true)
    {
        JSP_TEST(force || true, testNativeCalls1);
        JSP_TEST(force || true, testHandler1);
    }
    
    if (force || true)
    {
        JSP_TEST(force || true, testPeers3); // SHOULD BE EXECUTED LAST BECAUSE IT DELETES THE peers GLOBAL ARRAY
    }
}

// ---

void TestingProxy::testPeers1()
{
    {
        JSP_CHECK(getPeerId() == "peers.Proxy[0]");
        
        Proxy vanilla1;
        JSP_CHECK(vanilla1.getPeerId() == "peers.Proxy[1]");
        
        Proxy singleton("ScriptManager", true);
        JSP_CHECK(singleton.getPeerId() == "peers.ScriptManager");
        
        /*
         * ALLOWED
         */
        executeScript("peers.Proxy[1].alien = new Barker('ALIEN1')");
        
        /*
         * INTENTIONALLY NOT ALLOWED
         */
        executeScript("peers.Proxy[1] = 255; peers.ScriptManager = 456");

        JSP_CHECK(toSource(get<OBJECT>(globalHandle(), "peers")) == "({Proxy:[{}, {alien:(new Barker(\"ALIEN1\"))}], ScriptManager:{}})");
    }
    
    /*
     * AT THIS STAGE, THE JS-PEERS ASSOCIATED WITH vanilla1 AND singleton ARE:
     *
     * - NOT ACCESSIBLE FROM JS ANYMORE
     * - FINALIZED
     */
    JSP_CHECK(toSource(get<OBJECT>(globalHandle(), "peers")) == "({Proxy:[{}, ,]})");
    
    JSP::forceGC();
    JSP_CHECK(Barker::isFinalized("ALIEN1")); // PROOF THAT peers.Proxy[1] (ASSOCIATED WITH vanilla1) IS TRULY GONE
}

void TestingProxy::testPeers2()
{
    if (getPeerId() == "peers.Proxy[0]")
    {
        executeScript("peers.Proxy[0].callMeBack = function() { print('C++ PROXY IS CALLING BACK'); }");
        
        if (hasOwnProperty(peerHandle(), "callMeBack"))
        {
            call(peerHandle(), "callMeBack");
        }
    }
    else
    {
        JSP_CHECK(false);
    }
    
    // ---
    
    /*
     * XXX: NOT SURE IF NAMES WHICH ARE NOT JS-IDENTIFIERS SHOULD BE ALLOWED...
     */
    
    Proxy customNamed1("Contains spaces");
    JSP_CHECK(customNamed1.getPeerId() == "peers[\"Contains spaces\"][0]");
    
    Proxy customNamed2("Script Manager", true);
    JSP_CHECK(customNamed2.getPeerId() == "peers[\"Script Manager\"]");
}

/*
 * TODO: PREVENT THE FOLLOWING NON-INTENTIONALLY-ALLOWED OPERATIONS
 * 
 * WHY IS IT NON-TRIVIAL TO IMPLEMENT?
 * - TestingJS::testReadOnlyProperty2
 * - TestingJS::testPermanentProperty1
 * - Proxy::init / Proxy::uninit
 * - Proxy::addInstance / Proxy::removeInstance
 */
void TestingProxy::testPeers3()
{
    executeScript("peers.Proxy[0].alien = new Barker('ALIEN2')");
    
    /*
     * SHOULD NOT BE ALLOWED
     */
    executeScript("delete peers.Proxy[0]");
    
    /*
     * EVEN IF peers.Proxy[0] IS NOT ACCESSIBLE ANYMORE FROM JS:
     * - IT IS STILL ROOTED (VIA Proxy::peer):
     *   - AS LONG AS THIS C++ Proxy IS ALIVE
     */
    JSP::forceGC();
    JSP_CHECK(Barker::isHealthy("ALIEN2"));

    /*
     * SHOULD NOT BE ALLOWED
     */
    executeScript("peers.alien = new Barker('ALIEN3')");

    /*
     * SHOULD NOT BE ALLOWED
     */
    executeScript("delete peers");
    
    /*
     * EVEN IF peers IS NOT ACCESSIBLE ANYMORE FROM JS:
     * - IT IS STILL ROOTED (VIA Proxy::Statics::peers):
     *   - UNTIL Proxy::uninit() IS CALLED
     */
    JSP::forceGC();
    JSP_CHECK(Barker::isHealthy("ALIEN3"));
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

/*
 * FINDINGS:
 *
 * - THE HANDLER SHOULD ONLY DEAL WITH PROXYING
 * - BUT RIGHT NOW IT IS ALSO ASSOCIATED WITH A JS-PEER
 */
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
