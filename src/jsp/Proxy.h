/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

/*
 * TODO:
 *
 * 1) IT SHOULD BE POSSIBLE TO CREATE A (C++)PROXY WITHOUT ANY (JS)PEER, E.G.
 *    - WHEN PEER-NAME IS ALREADY TAKEN (I.E. FOR SINGLETONS)
 *    - WHEN PEER-NAME IS NOT A JS-IDENTIFIER, ETC.
 *    - OR WHEN THE PROXY INTENDED TO ACT SOLELY AS A "HANDLER"
 *      - QUESTION: HOW TO CONSTRUCT SUCH A PROXY?
 *
 * 2) PEERS COULD HAVE A "NAMESPACE" (IN ADDITION TO THEIR NAME), E.G.
 *    - v1.FileManager
 *
 * 3) PEERS COULD HAVE A STRING-BASED "SIGNATURE" (INSTEAD OF THE PeerProperties STRUCT), E.G.
 *    - Proxy[] (I.E. ARRAY-BASED)
 *    - FileManager (I.E. SINGLETON)
 *    - v1.FileDownloader[] (I.E. ARRAY-BASED, WITH NAMESPACE)
 *
 * 4) SHOULD IT BE POSSIBLE TO CREATE A (C++)PROXY FROM THE JS-SIDE?, E.G.
 *    var peer = new Peer("v1.FileDownloader", "http:://foo.com/bar.txt");
 *    peer.onReady = function(data) { print(data); };
 *    peer.start();
 *
 * 5) THERE COULD BE A STATIC Proxy::peersHandle() METHOD, I.E.
 *    - INSTEAD OF USING get<OBJECT>(globalHandle(), "peers")
 */

#pragma once

#include "jsp/Proto.h"
#include "jsp/WrappedObject.h"

namespace jsp
{
    typedef std::function<bool(const CallArgs&)> NativeCallFnType;
    
    struct NativeCall
    {
        std::string name;
        NativeCallFnType fn;
        
        NativeCall(const std::string &name, const NativeCallFnType &fn)
        :
        name(name),
        fn(fn)
        {}
    };
    
    struct PeerProperties
    {
        std::string name;
        bool isSingleton;
        
        PeerProperties(const std::string &name = "", bool isSingleton = false)
        :
        name(name),
        isSingleton(isSingleton)
        {}
    };
    
    class Proxy : public Proto
    {
    public:
        static bool init();
        static void uninit();
        
        // ---
        
        Proxy();
        Proxy(const std::string &peerName, bool isSingleton = false);
        
        virtual ~Proxy();

        // ---

        inline JS::HandleObject peerHandle() const
        {
            return JS::Handle<JSObject*>::fromMarkedLocation(reinterpret_cast<JSObject* const*>(peer.address())); // XXX
        }
        
        std::string getPeerId();

        virtual int32_t registerNativeCall(const std::string &name, const NativeCallFnType &fn);
        virtual bool unregisterNativeCall(const std::string &name);
        virtual bool apply(const NativeCall &nativeCall, const CallArgs &args);

    protected:
        PeerProperties peerProperties;
        int32_t peerElementIndex = -1;

        virtual const PeerProperties defaultPeerProperties() const;
        virtual JSObject* createPeer();

        static bool forwardNativeCall(JSContext *cx, unsigned argc, Value *vp);
        
    private:
        struct Statics
        {
            int32_t lastInstanceId = -1;
            std::map<int32_t, Proxy*> instances;
            
            Heap<WrappedObject> peers;
        };
        
        static Statics *statics;
        
        static int32_t addInstance(Proxy *instance);
        static bool removeInstance(int32_t instanceId);
        static Proxy* getInstance(int32_t instanceId);
        
        // ---
        
        Heap<WrappedObject> peer;
        
        int32_t instanceId = -1;
        int32_t lastNativeCallId = -1;
        std::map<int32_t, NativeCall> nativeCalls;
        
        const NativeCall* getNativeCall(int32_t nativeCallId) const;
        int32_t getNativeCallId(const std::string &name) const;
    };
}
