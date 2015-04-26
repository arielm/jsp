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
    Proxy::Statics *Proxy::statics = nullptr;

    bool Proxy::init()
    {
        if (!statics)
        {
            statics = new Statics;
            
            statics->peers = proto::newPlainObject();
            proto::defineProperty(globalHandle(), "peers", statics->peers, JSPROP_ENUMERATE | JSPROP_READONLY); // XXX: CAN'T BE MADE "PERMANENT"
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
            proto::deleteProperty(globalHandle(), "peers");
            
            delete statics;
            statics = nullptr;
        }
    }
    
    int32_t Proxy::addInstance(Proxy *instance)
    {
        if (!instance->peerProperties.name.empty())
        {
            const char *name = instance->peerProperties.name.data();
            
            RootedValue peerValue(cx);
            proto::getProperty(statics->peers, name, &peerValue);
            
            bool peerIsDefined = !peerValue.isUndefined();
            bool peerIsArray = isArray(peerValue);
            
            bool singletonEnabled = !peerIsDefined;
            bool multipleInstancesEnabled = (peerIsDefined && peerIsArray) || !peerIsDefined;
            
            if ((instance->peerProperties.isSingleton && singletonEnabled) || (!instance->peerProperties.isSingleton && multipleInstancesEnabled))
            {
                if (instance->peerProperties.isSingleton)
                {
                    instance->peer = instance->createPeer();
                    proto::defineProperty(statics->peers, name, instance->peer, JSPROP_ENUMERATE | JSPROP_READONLY); // XXX: CAN'T BE MADE "PERMANENT"
                }
                else
                {
                    RootedObject peerArray(cx);
                    instance->peerElementIndex = 0;
                    
                    if (peerIsDefined)
                    {
                        peerArray = peerValue.toObjectOrNull();
                        instance->peerElementIndex = proto::getLength(peerArray);
                    }
                    else
                    {
                        peerArray = proto::newArray();
                        proto::defineProperty(statics->peers, name, peerArray, JSPROP_ENUMERATE | JSPROP_READONLY); // XXX: CAN'T BE MADE "PERMANENT"
                    }
                    
                    instance->peer = instance->createPeer();
                    proto::defineElement(peerArray, instance->peerElementIndex, instance->peer, JSPROP_READONLY); // XXX: CAN'T BE MADE "PERMANENT"
                }
                
                statics->instances.emplace(++statics->lastInstanceId, instance);
                return statics->lastInstanceId;
            }
        }
        
        return -1;
    }
    
    bool Proxy::removeInstance(int32_t instanceId)
    {
        Proxy *instance = getInstance(instanceId);
        
        if (instance)
        {
            const char *name = instance->peerProperties.name.data();
            
            RootedValue peerValue(cx);
            proto::getProperty(statics->peers, name, &peerValue);
         
            if (peerValue.isObject())
            {
                if (instance->peerProperties.isSingleton)
                {
                    proto::deleteProperty(statics->peers, name);
                }
                else
                {
                    RootedObject peerArray(cx, peerValue.toObjectOrNull());
                    proto::deleteElement(peerArray, instance->peerElementIndex);
                    
                    if (proto::getElementCount(peerArray) == 0)
                    {
                        proto::deleteProperty(statics->peers, name);
                    }
                }
            }
            
            // ---
            
            statics->instances.erase(instanceId);
            return true;
        }
        
        return false;
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
    
    const PeerProperties Proxy::defaultPeerProperties() const
    {
        return PeerProperties("Proxy", false);
    }
    
    JSObject* Proxy::createPeer()
    {
        RootedObject object(cx, proto::newPlainObject());
        
        RootedValue tmp(cx, toValue(peerProperties.name));
        JS_DefineProperty(cx, object, "name", tmp, JSPROP_READONLY | JSPROP_PERMANENT);
        
        tmp = toValue(peerProperties.isSingleton);
        JS_DefineProperty(cx, object, "isSingleton", tmp, JSPROP_READONLY | JSPROP_PERMANENT);
        
        if (!peerProperties.isSingleton)
        {
            tmp = toValue(peerElementIndex);
            JS_DefineProperty(cx, object, "index", tmp, JSPROP_READONLY | JSPROP_PERMANENT);
        }
        
        return object;
    }
    
    std::string Proxy::getPeerId()
    {
        string result = "peers";
        
        if (isIdentifier(peerProperties.name))
        {
            result += '.' + peerProperties.name;
        }
        else
        {
            result += "[\"" + peerProperties.name + "\"]";
        }
        
        if (!peerProperties.isSingleton)
        {
            result += '[' + ci::toString(peerElementIndex) + ']';
        }
        
        return result;
    }

    // ---

    Proxy::Proxy()
    :
    peerProperties(defaultPeerProperties())
    {
        instanceId = addInstance(this);
        assert(instanceId > -1);
    }
    
    Proxy::Proxy(const string &peerName, bool isSingleton)
    :
    peerProperties(PeerProperties(peerName, isSingleton))
    {
        instanceId = addInstance(this);
        assert(instanceId > -1);
    }

    Proxy::~Proxy()
    {
        removeInstance(instanceId);
    }
    
    Proxy* Proxy::getHandler() const
    {
        return handler;
    }
    
    void Proxy::setHandler(Proxy *handler)
    {
        if (handler != this)
        {
            this->handler = handler;
        }
    }
    
    // ---
    
    int32_t Proxy::registerNativeCall(const string &name, const NativeCallFnType &fn)
    {
        auto nativeCallId = getNativeCallId(name);
        
        if (nativeCallId == -1)
        {
            auto function = DefineFunctionWithReserved(cx, peer.get(), name.data(), forwardNativeCall, 0, JSPROP_ENUMERATE | JSPROP_READONLY); // XXX: CAN'T BE MADE "PERMANENT"
            
            if (function)
            {
                nativeCalls.emplace(++lastNativeCallId, NativeCall(name, fn));
                nativeCallId = lastNativeCallId;
                
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
            proto::deleteProperty(peer, name.data());
            nativeCalls.erase(nativeCallId);
            
            return true;
        }
        
        return false;
    }
    
    bool Proxy::apply(const NativeCall &nativeCall, const CallArgs &args)
    {
        return nativeCall.fn(args);
    }

    // ---
    
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
}
