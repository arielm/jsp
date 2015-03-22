/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "jsp/Proxy.h"

#include "chronotext/utils/Utils.h"

using namespace std;
using namespace chr;

namespace jsp
{
    namespace proxy
    {
        map<int32_t, Proxy*> registeredProxies;
        int32_t lastRegisteredProxyId = 0;
        
        Proxy* getRegisteredProxy(int32_t proxyId);
        int32_t getRegisteredProxyId(Proxy *proxy);
        int32_t nextRegisteredProxyId();
    }
    
    Proxy* proxy::getRegisteredProxy(int32_t proxyId)
    {
        auto found = registeredProxies.find(proxyId);
        
        if (found != registeredProxies.end())
        {
            return found->second;
        }
        
        return nullptr;
    }
    
    int32_t proxy::getRegisteredProxyId(Proxy *proxy)
    {
        for (auto &element : registeredProxies)
        {
            if (proxy == element.second)
            {
                return element.first;
            }
        }
        
        return -1;
    }
    
    int32_t proxy::nextRegisteredProxyId()
    {
        return lastRegisteredProxyId++;
    }
    
    // ---
    
    Proxy::Proxy(Proto *target)
    {
        if (!setTarget(target))
        {
            throw EXCEPTION(Proxy, "INVALID TARGET");
        }
    }
    
    Proxy::Proxy(Proxy *target)
    {
        if (!setTarget(target))
        {
            throw EXCEPTION(Proxy, "INVALID TARGET");
        }
    }
    
    bool Proxy::setTarget(Proto *target)
    {
        if (target && (target != this) && (target != handler))
        {
            this->target = target;
            return true;
        }
        
        return false;
    }
    
    bool Proxy::setTarget(Proxy *target)
    {
        if (target && (target->target != this))
        {
            return setTarget(static_cast<Proto*>(target));
        }
        
        return false;
    }
    
    bool Proxy::setHandler(Proto *handler)
    {
        if ((handler != this) && (handler != target))
        {
            this->handler = handler;
            return true;
        }
        
        return false;
    }
    
    bool Proxy::setHandler(Proxy *handler)
    {
        if (handler->target != this)
        {
            return setHandler(static_cast<Proto*>(handler));
        }
        
        return false;
    }
    
    Proxy::~Proxy()
    {
        // TODO: UNREGISTER ASSOCIATED NATIVE-CALLBACKS
    }
    
    // ---
    
    /*
     * TODO: CALLBACK-MAPPING SHOULD TAKE PLACE PER GLOBAL-OBJECT (I.E. PER COMPARTMENT)
     */
    
    Callback* Proxy::getCallback(int32_t callbackId)
    {
        auto found = callbacks.find(callbackId);
        
        if (found != callbacks.end())
        {
            return &found->second;
        }
        
        return nullptr;
    }
    
    int32_t Proxy::getCallbackId(const std::string &name)
    {
        for (auto &element : callbacks)
        {
            if (name == element.second.name)
            {
                return element.first;
            }
        }
        
        return -1;
    }
    
    int32_t Proxy::nextCallbackId()
    {
        return lastCallbackId++;
    }
    
    void Proxy::registerCallback(HandleObject object, const string &name, const function<bool(CallArgs args)> &fn, Proxy *proxy)
    {
        if (proxy)
        {
            auto proxyId = proxy::getRegisteredProxyId(proxy);
            auto callbackId = proxy->getCallbackId(name);
            
            if (proxyId == -1)
            {
                proxyId = proxy::nextRegisteredProxyId();
                proxy::registeredProxies.emplace(proxyId, proxy);
                
                if (callbackId == -1)
                {
                    callbackId = proxy->nextCallbackId();
                    proxy->callbacks.emplace(callbackId, Callback(name, fn));
                }
                else
                {
                    // TODO: HANDLE ALREADY-REGISTERED name
                }
            }
            else
            {
                // TODO: HANDLE ALREADY-REGISTERED proxy
            }
            
            /*
             * TODO: HANDLE ALREADY-DEFINED JSFunction IN object
             */
            
            RootedFunction function(cx, DefineFunctionWithReserved(cx, object, name.data(), dispatchCallback, 0, 0));
            SetFunctionNativeReserved(function, 0, NumberValue(proxyId));
            SetFunctionNativeReserved(function, 1, NumberValue(callbackId));
        }
    }
    
    bool Proxy::dispatchCallback(JSContext *cx, unsigned argc, Value *vp)
    {
        auto args = CallArgsFromVp(argc, vp);
        JSObject &callee = args.callee();
        JSFunction *function = &callee.as<JSFunction>();
        
        auto proxyId = GetFunctionNativeReserved(function, 0).toInt32();
        auto proxy = proxy::getRegisteredProxy(proxyId);
        
        if (proxy)
        {
            auto callbackId = GetFunctionNativeReserved(function, 1).toInt32();
            auto callback = proxy->getCallback(callbackId);
            
            if (callback)
            {
                return proxy->applyCallback(callback->fn, args);
            }
        }
        
        /*
         * TODO: REPORT JS-ERROR
         */
        
        return false;
    }
}
