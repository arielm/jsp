/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "TestingProxy.h"

#include "jsp/Proxy.h"

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
        Proxy vanilla;
        JSP_CHECK(vanilla.getPeerAccessor() == "peers.Proxy[0]");
        
        Proxy singleton("ScriptManager", true);
        JSP_CHECK(singleton.getPeerAccessor() == "peers.ScriptManager");
        
        /*
         * ALLOWED
         */
        executeScript("peers.Proxy[0].alien = new Barker('ALIEN1')");
        
        /*
         * INTENTIONALLY NOT ALLOWED
         */
        executeScript("peers.Proxy[0] = 255; peers.ScriptManager = 456"); // TODO: JSP_CHECK "CAPTURED" OUTPUT

        JSP_CHECK(toSource(get<OBJECT>(globalHandle(), "peers")) == "({Proxy:[{alien:(new Barker(\"ALIEN1\"))}], ScriptManager:{}})");
    }
    
    /*
     * AT THIS STAGE, THE JS-PEERS ASSOCIATED WITH vanilla AND singleton ARE:
     *
     * - NOT ACCESSIBLE FROM JS ANYMORE
     * - FINALIZED
     */
    JSP_CHECK(toSource(get<OBJECT>(globalHandle(), "peers")) == "({})");
    
    forceGC();
    JSP_CHECK(Barker::isFinalized("ALIEN1")); // PROOF THAT peers.Proxy[0] (ASSOCIATED WITH vanilla) IS TRULY GONE
}

void TestingProxy::testPeers2()
{
    Proxy proxy;
    
    executeScript(proxy.getPeerAccessor() + ".callMeBack = function() { print('PROXY IS CALLING BACK'); }");
    
    call(proxy.peer, "callMeBack"); // TODO: JSP_CHECK "CAPTURED" OUTPUT
    
    // ---
    
    /*
     * XXX: NOT SURE IF NAMES WHICH ARE NOT JS-IDENTIFIERS SHOULD BE ALLOWED...
     */
    
    Proxy customNamed1("Contains spaces");
    JSP_CHECK(customNamed1.getPeerAccessor() == "peers[\"Contains spaces\"][0]");
    
    Proxy customNamed2("Script Manager", true);
    JSP_CHECK(customNamed2.getPeerAccessor() == "peers[\"Script Manager\"]");
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
    Proxy proxy;
    
    executeScript(proxy.getPeerAccessor() + ".alien = new Barker('ALIEN2')");
    
    /*
     * SHOULD NOT BE ALLOWED
     */
    executeScript("delete " + proxy.getPeerAccessor());
    
    /*
     * EVEN IF peers.Proxy[0] IS NOT ACCESSIBLE ANYMORE FROM JS:
     * - IT IS STILL ROOTED (VIA Proxy::peer), AS LONG AS proxy IS ALIVE
     */
    forceGC();
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
     * - IT IS STILL ROOTED (VIA Proxy::Statics::peers), UNTIL Proxy::uninit() IS CALLED
     */
    forceGC();
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
    Proxy proxy;
    
    executeScript("var target = " + proxy.getPeerAccessor());

    // ---
    
    proxy.registerNativeCall("staticMethod1", BIND_STATIC1(staticMethod1));
    proxy.registerNativeCall("instanceMethod1", BIND_INSTANCE1(&TestingProxy::instanceMethod1, this));
    
    proxy.registerNativeCall("lambda1", [=](const CallArgs &args)->bool
    {
        if (args.hasDefined(0) && args[0].isNumber())
        {
            args.rval().set(NumberValue(args[0].toNumber() * instanceValue1));
            return true;
        }

        return false;
    });
    
    executeScript("print(target.staticMethod1(77), target.instanceMethod1(11), target.lambda1(33))"); // TODO: JSP_CHECK "CAPTURED" OUTPUT
    
    // ---

    proxy.unregisterNativeCall("staticMethod1");
    executeScript("try { print(target.staticMethod1(33)); } catch(e) { print(e);}"); // TODO: JSP_CHECK "CAPTURED" OUTPUT
}
