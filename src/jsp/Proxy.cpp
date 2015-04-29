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
    int32_t Proxy::lastInstanceId = -1;

    bool Proxy::init()
    {
        if (!statics)
        {
            statics = new Statics;
            
            statics->peers = newPlainObject();
            define(globalHandle(), "peers", statics->peers, JSPROP_ENUMERATE | JSPROP_READONLY); // XXX: CAN'T BE MADE "PERMANENT"
        }
        
        return bool(statics);
    }
    
    void Proxy::uninit()
    {
        if (statics)
        {
            /*
             * PURPOSELY NOT RESETTING lastInstanceId IN ORDER TO PREVENT INSTANCES
             * POTENTIALLY ALIVE AT THIS STAGE TO "INTERFER" IN THE FUTURE (TODO: TEST)
             */
            
            statics->instances.clear();
            
            statics->peers = nullptr;
            deleteProperty(globalHandle(), "peers");
            
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
            getProperty(statics->peers, name, &peerValue);
            
            bool peerIsDefined = !peerValue.isUndefined();
            bool peerIsArray = isArray(peerValue);
            
            bool singletonEnabled = !peerIsDefined;
            bool multipleInstancesEnabled = (peerIsDefined && peerIsArray) || !peerIsDefined;
            
            if ((instance->peerProperties.isSingleton && singletonEnabled) || (!instance->peerProperties.isSingleton && multipleInstancesEnabled))
            {
                if (instance->peerProperties.isSingleton)
                {
                    instance->peer = instance->createPeer();
                    define(statics->peers, name, instance->peer, JSPROP_ENUMERATE | JSPROP_READONLY); // XXX: CAN'T BE MADE "PERMANENT"
                }
                else
                {
                    RootedObject peerArray(cx);
                    instance->peerElementIndex = 0;
                    
                    if (peerIsDefined)
                    {
                        peerArray = peerValue.toObjectOrNull();
                        instance->peerElementIndex = getLength(peerArray);
                    }
                    else
                    {
                        peerArray = newArray();
                        define(statics->peers, name, peerArray, JSPROP_ENUMERATE | JSPROP_READONLY); // XXX: CAN'T BE MADE "PERMANENT"
                    }
                    
                    instance->peer = instance->createPeer();
                    define(peerArray, instance->peerElementIndex, instance->peer, JSPROP_READONLY); // XXX: CAN'T BE MADE "PERMANENT"
                }
                
                int32_t instanceId = ++lastInstanceId;
                statics->instances.emplace(instanceId, instance);
                
                return instanceId;
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
            
            RootedObject peer(cx, get<OBJECT>(statics->peers, name));
         
            if (peer)
            {
                if (instance->peerProperties.isSingleton)
                {
                    deleteProperty(statics->peers, name);
                }
                else
                {
                    deleteElement(peer, instance->peerElementIndex);
                    
                    if (getElementCount(peer) == 0)
                    {
                        deleteProperty(statics->peers, name);
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

    Proxy::Proxy()
    :
    peerProperties(defaultPeerProperties())
    {
        instanceId = addInstance(this);
        assert(instanceId > -1); // TODO INSTEAD: ALLOW PEER-LESS PROXIES
    }
    
    Proxy::Proxy(const string &peerName, bool isSingleton)
    :
    peerProperties(PeerProperties(peerName, isSingleton))
    {
        instanceId = addInstance(this);
        assert(instanceId > -1); // TODO INSTEAD: ALLOW PEER-LESS PROXIES
    }

    Proxy::~Proxy()
    {
        removeInstance(instanceId);
    }
    
    // ---
    
    const PeerProperties Proxy::defaultPeerProperties() const
    {
        return PeerProperties("Proxy", false);
    }
    
    JSObject* Proxy::createPeer()
    {
        RootedObject object(cx, newPlainObject());
        
        define(object, "name", peerProperties.name, JSPROP_READONLY | JSPROP_PERMANENT);
        define(object, "isSingleton", peerProperties.isSingleton, JSPROP_READONLY | JSPROP_PERMANENT);
        
        if (!peerProperties.isSingleton)
        {
            define(object, "index", peerElementIndex, JSPROP_READONLY | JSPROP_PERMANENT);
        }
        
        return object;
    }
    
    std::string Proxy::getPeerAccessor()
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
            deleteProperty(peer, name.data());
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
