/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "jsp/Proxy.h"
#include "jsp/BaseProto.h"

#include "chronotext/utils/Utils.h"

using namespace std;
using namespace chr;

namespace jsp
{
#pragma mark ---------------------------------------- proxy NAMESPACE ----------------------------------------
    
    namespace proxy
    {
        map<int32_t, Proxy*> instances;
        int32_t lastInstanceId = -1;
        
        int32_t instanceCreated(Proxy *instance);
        void instanceDestroyed(int32_t instanceId);
        Proxy* getInstance(int32_t instanceId);
    }
    
    int32_t proxy::instanceCreated(Proxy *instance)
    {
        proxy::instances.emplace(++proxy::lastInstanceId, instance);
        return proxy::lastInstanceId;
    }
    
    void proxy::instanceDestroyed(int32_t instanceId)
    {
        proxy::instances.erase(instanceId);
    }
    
    Proxy* proxy::getInstance(int32_t instanceId)
    {
        auto found = instances.find(instanceId);
        
        if (found != instances.end())
        {
            return found->second;
        }
        
        return nullptr;
    }
    
#pragma mark ---------------------------------------- Proxy NAMESPACE ----------------------------------------

    Proxy::Proxy(Proto *target)
    {
        if (!target)
        {
            this->target = BaseProto::instance();
        }
        else
        {
            this->target = target;
        }
        
        instanceId = proxy::instanceCreated(this);
    }
    
    Proxy::Proxy(Proxy *target)
    :
    Proxy(static_cast<Proto*>(target))
    {}

    Proxy::~Proxy()
    {
        proxy::instanceDestroyed(instanceId);
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
    
    bool Proxy::setHandler(Proto *handler)
    {
        if ((handler != this) && (handler != target))
        {
            this->handler = handler;
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
    
    bool Proxy::setHandler(Proxy *handler)
    {
        if (handler->target != this)
        {
            return setHandler(static_cast<Proto*>(handler));
        }
        
        return false;
    }
    
    string Proxy::peerName()
    {
        return "Proxy";
    }

    bool Proxy::isSingleton()
    {
        return false;
    }
    
    // ---
    
    NativeCall* Proxy::getNativeCall(int32_t nativeCallId)
    {
        auto found = nativeCalls.find(nativeCallId);
        
        if (found != nativeCalls.end())
        {
            return &found->second;
        }
        
        return nullptr;
    }
    
    int32_t Proxy::getNativeCallId(const string &name)
    {
        for (auto &element : nativeCalls)
        {
            if (name == element.second.name)
            {
                return element.first;
            }
        }
        
        return -1;
    }
    
    int32_t Proxy::addNativeCall(const string &name, const NativeCallFnType &fn)
    {
        nativeCalls.emplace(++lastNativeCallId, NativeCall(name, fn));
        return lastNativeCallId;
    }
    
    void Proxy::removeNativeCall(int32_t nativeCallId)
    {
        nativeCalls.erase(nativeCallId);
    }

    // ---
    
    bool Proxy::registerNativeCall(HandleObject object, const string &name, const NativeCallFnType &fn)
    {
        auto nativeCallId = getNativeCallId(name);
        
        if (nativeCallId == -1)
        {
            auto function = DefineFunctionWithReserved(cx, object, name.data(), forwardNativeCall, 0, 0);
            
            if (function)
            {
                nativeCallId = addNativeCall(name, fn);
                
                SetFunctionNativeReserved(function, 0, NumberValue(instanceId));
                SetFunctionNativeReserved(function, 1, NumberValue(nativeCallId));
                
                return true;
            }
        }
        
        return false;
    }
    
    void Proxy::unregisterNativeCall(HandleObject object, const string &name)
    {
        auto nativeCallId = getNativeCallId(name);
        
        if (nativeCallId != -1)
        {
            removeNativeCall(nativeCallId);
            
            /*
             * TODO: SOME CONDITIONS SHOULD BE MET IN ORDER TO DELETE
             *
             * 1) object HAS A name PROPERTY CONTAINING A JSFunction
             * 2) JSFunction HAS 2 NATIVE-RESERVED SLOTS
             * 3) SLOT 1 CONTAINS VALUE EQUAL TO instanceId
             * 4) SLOT 2 CONTAINS VALUE EQUAL TO nativeCallId
             */
            JS_DeleteProperty(cx, object, name.data());
        }
    }

    bool Proxy::forwardNativeCall(JSContext *cx, unsigned argc, Value *vp)
    {
        auto args = CallArgsFromVp(argc, vp);
        auto function = &args.callee().as<JSFunction>();
        
        auto proxyId = GetFunctionNativeReserved(function, 0).toInt32();
        auto proxy = proxy::getInstance(proxyId);
        
        if (proxy)
        {
            auto nativeCallId = GetFunctionNativeReserved(function, 1).toInt32();
            auto nativeCall = proxy->getNativeCall(nativeCallId);
            
            if (nativeCall)
            {
                return proxy->applyNativeCall(nativeCall->fn, args);
            }
        }
        
        /*
         * TODO: REPORT JS-ERROR
         */
        
        return false;
    }
}
