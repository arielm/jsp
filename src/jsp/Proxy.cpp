/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "jsp/Proxy.h"
#include "jsp/BaseProto.h"
#include "jsp/WrappedObject.h"

#include "chronotext/utils/Utils.h"

using namespace std;
using namespace chr;

namespace jsp
{
    Proxy::StaticData *Proxy::data = nullptr;

    bool Proxy::init()
    {
        if (!data)
        {
            data = new StaticData;
            
            data->peers = JS_NewObject(cx, nullptr, NullPtr(), NullPtr());
            JS_DefineProperty(cx, globalHandle(), "peers", data->peers, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT);
        }
        
        return data;
    }
    
    void Proxy::uninit()
    {
        if (data)
        {
            data->lastInstanceId = -1;
            data->instances.clear();
            
            data->peers = nullptr;
            JS_DeleteProperty(cx, globalHandle(), "peers");
            
            delete data;
            data = nullptr;
        }
    }
    
    int32_t Proxy::addInstance(Proxy *instance, const PeerProperties &peerProperties)
    {
        data->instances.emplace(++data->lastInstanceId, instance);
        
        // ---
        
        const char *name = peerProperties.name.data();
        
        instance->peer = JS_NewObject(cx, nullptr, NullPtr(), NullPtr());
        
        if (peerProperties.isSingleton)
        {
            JS_DefineProperty(cx, data->peers, name, instance->peer, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT);
        }
        else
        {
            RootedObject peerArray(cx);
            uint32_t peerCount = 0;
            
            RootedValue tmp1(cx);
            JS_GetProperty(cx, data->peers, name, &tmp1);
            
            if (tmp1.isUndefined())
            {
                peerArray = JS_NewArrayObject(cx, 0);
                JS_DefineProperty(cx, data->peers, name, peerArray, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT);
            }
            else
            {
                peerArray = tmp1.toObjectOrNull();
                JS_GetArrayLength(cx, peerArray, &peerCount);
            }
            
            RootedValue tmp2(cx, instance->peer.get());
            JS_DefineElement(cx, peerArray, peerCount, tmp2, nullptr, nullptr, JSPROP_READONLY | JSPROP_PERMANENT);
        }
        
        return data->lastInstanceId;
    }
    
    void Proxy::removeInstance(int32_t instanceId)
    {
        data->instances.erase(instanceId);
    }
    
    Proxy* Proxy::getInstance(int32_t instanceId)
    {
        auto found = data->instances.find(instanceId);
        
        if (found != data->instances.end())
        {
            return found->second;
        }
        
        return nullptr;
    }

    // ---
    
    Proxy::Proxy(Proto *target, const PeerProperties &peerProperties)
    {
        this->target = target;
        instanceId = addInstance(this, peerProperties);
    }
    
    Proxy::Proxy(Proto *target)
    {
        this->target = target ? target : defaultTarget();
        instanceId = addInstance(this, defaultPeerProperties());
    }
    
    Proxy::Proxy(const string &peerName, bool isSingleton)
    {
        target = defaultTarget();
        instanceId = addInstance(this, PeerProperties(peerName, isSingleton));
    }

    Proxy::~Proxy()
    {
        removeInstance(instanceId);
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
    
    // ---
    
    Proto* Proxy::defaultTarget() const
    {
        return BaseProto::target();
    }
    
    const PeerProperties Proxy::defaultPeerProperties() const
    {
        return PeerProperties("Proxy", false);
    }

    // ---
    
    const NativeCall* Proxy::getNativeCall(int32_t nativeCallId) const
    {
        const auto found = nativeCalls.find(nativeCallId);
        
        if (found != nativeCalls.end())
        {
            return &found->second;
        }
        
        return nullptr;
    }
    
    int32_t Proxy::getNativeCallId(const string &name) const
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
        auto proxy = getInstance(proxyId);
        
        if (proxy)
        {
            auto nativeCallId = GetFunctionNativeReserved(function, 1).toInt32();
            auto nativeCall = proxy->getNativeCall(nativeCallId);
            
            if (nativeCall)
            {
                return proxy->apply(*nativeCall, args);
            }
        }
        
        /*
         * TODO: REPORT JS-ERROR
         */
        
        return false;
    }
}
