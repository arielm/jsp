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
        int32_t lastProxyId = -1;
        
        Proxy* getRegisteredProxy(int32_t proxyId);
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
        unregisterProxy();
    }
    
    // ---
    
    int32_t Proxy::registerProxy()
    {
        if (proxyId == -1)
        {
            proxy::registeredProxies.emplace(++proxy::lastProxyId, this);
            proxyId = proxy::lastProxyId;
        }
        
        return proxyId;
    }
    
    void Proxy::unregisterProxy()
    {
        proxy::registeredProxies.erase(proxyId);
    }
    
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
    
    int32_t Proxy::registerCallback(const string &name, const function<bool(CallArgs args)> &fn)
    {
        callbacks.emplace(++lastCallbackId, Callback(name, fn));
        return lastCallbackId;
    }
    
    // ---
    
    void Proxy::registerCallback(HandleObject object, const string &name, const function<bool(CallArgs args)> &fn, Proxy *proxy)
    {
        auto proxyId = proxy->registerProxy();
        auto callbackId = proxy->getCallbackId(name);
        
        if (callbackId != -1)
        {
            throw EXCEPTION(Proxy, "UNABLE TO REGISTER CALLBACK | REASON: " + name + " IS ALREADY DEFINED AT Proxy LEVEL");
        }
        
        // ---
        
        auto function = DefineFunctionWithReserved(cx, object, name.data(), dispatchCallback, 0, 0);
        
        if (!function)
        {
            throw EXCEPTION(Proxy, "UNABLE TO REGISTER CALLBACK | REASON: " + name + " CAN'T BE DEFINED AT JSObject LEVEL");
        }
        
        callbackId = proxy->registerCallback(name, fn);
        
        SetFunctionNativeReserved(function, 0, NumberValue(proxyId));
        SetFunctionNativeReserved(function, 1, NumberValue(callbackId));
    }

    bool Proxy::dispatchCallback(JSContext *cx, unsigned argc, Value *vp)
    {
        auto args = CallArgsFromVp(argc, vp);
        auto function = &args.callee().as<JSFunction>();
        
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
