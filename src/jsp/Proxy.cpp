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
    Proxy::Statics *Proxy::statics = nullptr;

    bool Proxy::init()
    {
        if (!statics)
        {
            statics = new Statics;
            
            statics->peers = JS_NewObject(cx, nullptr, NullPtr(), NullPtr());
            JS_DefineProperty(cx, globalHandle(), "peers", statics->peers, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT); // XXX
        }
        
        return statics;
    }
    
    void Proxy::uninit()
    {
        if (statics)
        {
            statics->lastInstanceId = -1;
            statics->instances.clear();
            
            statics->peers = nullptr;
            JS_DeleteProperty(cx, globalHandle(), "peers"); // XXX
            
            delete statics;
            statics = nullptr;
        }
    }
    
    int32_t Proxy::addInstance(Proxy *instance, const PeerProperties &peerProperties)
    {
        if (!peerProperties.name.empty())
        {
            const char *name = peerProperties.name.data();
            
            RootedValue property(cx);
            JS_GetProperty(cx, statics->peers, name, &property);
            
            bool propertyIsDefined = !property.isUndefined();
            bool propertyIsArray = JS_IsArrayObject(cx, property);
            
            bool singletonEnabled = !propertyIsDefined;
            bool multipleInstancesEnabled = (propertyIsDefined && propertyIsArray) || !propertyIsDefined;
            
            if ((peerProperties.isSingleton && singletonEnabled) || (!peerProperties.isSingleton && multipleInstancesEnabled))
            {
                instance->peer = JS_NewObject(cx, nullptr, NullPtr(), NullPtr());
                
                if (peerProperties.isSingleton)
                {
                    JS_DefineProperty(cx, statics->peers, name, instance->peer, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT); // XXX
                }
                else
                {
                    RootedObject peerArray(cx);
                    uint32_t peerCount = 0;
                    
                    if (propertyIsDefined)
                    {
                        peerArray = property.toObjectOrNull();
                        JS_GetArrayLength(cx, peerArray, &peerCount);
                    }
                    else
                    {
                        peerArray = JS_NewArrayObject(cx, 0);
                        JS_DefineProperty(cx, statics->peers, name, peerArray, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT); // XXX
                    }
                    
                    instance->peer = JS_NewObject(cx, nullptr, NullPtr(), NullPtr());
                    JS_DefineElement(cx, peerArray, peerCount, ObjectOrNullValue(instance->peer.get()), nullptr, nullptr, JSPROP_READONLY | JSPROP_PERMANENT); // XXX
                }
                
                statics->instances.emplace(++statics->lastInstanceId, instance);
                
                return statics->lastInstanceId;
            }
        }
        
        return -1;
    }
    
    bool Proxy::removeInstance(int32_t instanceId)
    {
        /*
         * TODO: DELETE PEER
         */
        
        statics->instances.erase(instanceId);
        return true;
    }
    
    Proxy* Proxy::getInstance(int32_t instanceId)
    {
        auto found = statics->instances.find(instanceId);
        
        if (found != statics->instances.end())
        {
            return found->second;
        }
        
        return nullptr;
    }

    // ---
    
    Proxy::Proxy(Proto *target, const PeerProperties &peerProperties)
    {
        this->target = target;
        assert(this->target);

        instanceId = addInstance(this, peerProperties);
        assert(instanceId > -1);
    }
    
    Proxy::Proxy(Proto *target)
    {
        this->target = target ? target : defaultTarget();
        assert(this->target);

        instanceId = addInstance(this, defaultPeerProperties());
        assert(instanceId > -1);
    }
    
    Proxy::Proxy(const string &peerName, bool isSingleton)
    {
        target = defaultTarget();
        assert(this->target);

        instanceId = addInstance(this, PeerProperties(peerName, isSingleton));
        assert(instanceId > -1);
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
    
    int32_t Proxy::registerNativeCall(const string &name, const NativeCallFnType &fn)
    {
        auto nativeCallId = getNativeCallId(name);
        
        if (nativeCallId == -1)
        {
            auto function = DefineFunctionWithReserved(cx, peer.get(), name.data(), forwardNativeCall, 0, JSPROP_ENUMERATE | JSPROP_READONLY); // XXX
            
            if (function)
            {
                nativeCallId = addNativeCall(name, fn);
                
                SetFunctionNativeReserved(function, 0, NumberValue(instanceId));
                SetFunctionNativeReserved(function, 1, NumberValue(nativeCallId));
            }
        }
        
        return nativeCallId;
    }
    
    bool Proxy::unregisterNativeCall(const string &name)
    {
        auto nativeCallId = getNativeCallId(name);
        
        if (nativeCallId != -1)
        {
            bool success = false;
            
            if (JS_DeleteProperty2(cx, peer, name.data(), &success) && success)
            {
                removeNativeCall(nativeCallId);
                return true;
            }
        }
        
        return false;
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
