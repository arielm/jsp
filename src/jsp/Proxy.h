/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#pragma once

#include "jsp/Proto.h"
#include "jsp/WrappedObject.h"

#define TARGET(FN, ...) target->FN(__VA_ARGS__)
#define HANDLE(FN, ...) handler->FN(__VA_ARGS__)
#define FORWARD(FN, ...) handler ? HANDLE(FN, __VA_ARGS__) : TARGET(FN, __VA_ARGS__)

namespace jsp
{
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
        
        Heap<WrappedObject> peer;
        
        Proxy(Proto *target, const PeerProperties &peerProperties);
        Proxy(Proto *target = nullptr);
        Proxy(const std::string &peerName, bool isSingleton = false);
        
        ~Proxy();

        bool setTarget(Proto *target);
        bool setHandler(Proto *handler);

        bool setTarget(Proxy *target);
        bool setHandler(Proxy *handler);
        
        virtual Proto* defaultTarget() const;
        virtual const PeerProperties defaultPeerProperties() const;
        
        // ---
        
        inline bool exec(const std::string &source, const ReadOnlyCompileOptions &options) final
        {
            return FORWARD(exec, source, options);
        }
        
        inline bool eval(const std::string &source, const ReadOnlyCompileOptions &options, MutableHandleValue result) final
        {
            return FORWARD(eval, source, options, result);
        }
        
        // ---
        
        inline Value call(HandleObject object, const char *functionName, const HandleValueArray& args = HandleValueArray::empty()) final
        {
            return FORWARD(call, object, functionName, args);
        }
        
        inline Value call(HandleObject object, HandleValue functionValue, const HandleValueArray& args = HandleValueArray::empty()) final
        {
            return FORWARD(call, object, functionValue, args);
        }
        
        inline Value call(HandleObject object, HandleFunction function, const HandleValueArray& args = HandleValueArray::empty()) final
        {
            return FORWARD(call, object, function, args);
        }
        
        // ---
        
        inline bool apply(const NativeCall &nativeCall, CallArgs args) final
        {
            return FORWARD(apply, nativeCall, args);
        }
        
        // ---
        
        inline JSObject* newPlainObject() final
        {
            return FORWARD(newPlainObject);
        }
        
        inline JSObject* newObject(const std::string &className, const HandleValueArray& args = HandleValueArray::empty()) final
        {
            return FORWARD(newObject, className, args);
        }

        inline bool hasProperty(HandleObject object, const char *name) final
        {
            return FORWARD(hasProperty, object, name);
        }
        
        inline bool hasOwnProperty(HandleObject object, const char *name) final
        {
            return FORWARD(hasOwnProperty, object, name);
        }
        
        inline bool getProperty(HandleObject object, const char *name, MutableHandleValue result) final
        {
            return FORWARD(getProperty, object, name, result);
        }
        
        inline bool setProperty(HandleObject object, const char *name, HandleValue value) final
        {
            return FORWARD(setProperty, object, name, value);
        }
        
        inline bool deleteProperty(HandleObject object, const char *name) final
        {
            return FORWARD(deleteProperty, object, name);
        }
        
        inline bool getOwnPropertyDescriptor(HandleObject object, HandleId id, MutableHandle<JSPropertyDescriptor> desc) final
        {
            return FORWARD(getOwnPropertyDescriptor, object, id, desc);
        }
        
        // ---
        
        inline JSObject* newArray(size_t length = 0) final
        {
            return FORWARD(newArray, length);
        }
        
        inline JSObject* newArray(const HandleValueArray& contents) final
        {
            return FORWARD(newArray, contents);
        }
        
        inline size_t getLength(HandleObject array) final
        {
            return FORWARD(getLength, array);
        }
        
        inline bool setLength(HandleObject array, size_t length) final
        {
            return FORWARD(setLength, array, length);
        }
        
        inline bool getElement(HandleObject array, int index, MutableHandleValue result) final
        {
            return FORWARD(getElement, array, index, result);
        }
        
        inline bool setElement(HandleObject array, int index, HandleValue value) final
        {
            return FORWARD(setElement, array, index, value);
        }
        
        inline bool deleteElement(HandleObject array, int index) final
        {
            return FORWARD(deleteElement, array, index);
        }

    protected:
        Proto *target = nullptr;
        Proto *handler = nullptr;
        
        PeerProperties peerProperties;
        uint32_t elementIndex = 0;

        /*
         * TODO:
         *
         * 1) EACH Proxy SHOULD BE ASSOCIATED WITH A JS-PEER:
         *    - CREATED DURING Proxy CONSTRUCTION:
         *      - ASSOCIATED WITH A "NAME" AND SOME "INDEX"
         *        - NAME COULD BE AUTOMATICALLY GENERATED (E.G. VIA "CLASS-NAME DEMANGLING")
         *          - BETTER ALTERNATIVE: A VIRTUAL string Proxy.peerName() METHOD
         *            - BECAUSE CLASS-NAME DEMANGLING IS NOT STANDARD
         *            - MORE FLEXIBLE ANYWAY, E.G. NOT BEING BOUND SOLELY TO C++ CLASS NAMES
         *        - INDEX SHOULD BE AUTOMATICALLY INCREMENTED BASED ON INSTANCE-COUNT
         *        - E.G. SomeProxy proxy(); SomeProxy anotherProxy();
         *          - proxy WOULD BE NAMED SomeProxy, WITH AN INDEX OF 0
         *          - anotherProxy WOULD BE NAMED SomeProxy, WITH AN INDEX OF 1
         *    - GLOBALLY-ACCESSIBLE FROM THE JS-SIDE:
         *      - E.G. Peers.SomeProxy[0] (OR peers.SomeProxy[0]?)
         *      - THE GLOBAL Peers (OR peers?) OBJECT SHOULD BE MANAGED AT THE JS-COMPARTMENT LEVEL
         * 2) registerNativeCall(HandleObject object, ...) AND unregisterNativeCall(HandleObject object, ...)
         *    SHOULD NOT OPERATE ON SOME EXTERNAL JS-OBJECT BUT ON THE PROXY'S JS-PEER, E.G.
         *    - proxy.registerNativeCall("method1", BIND_STATIC1(staticMethod1);
         *      proxy.unregisterNativeCall("method1");
         * 3) THEN IT SHOULD BE POSSIBLE TO "CONNECT/DISCONNECT":
         *    - FROM THE C++ SIDE, E.G.
         *      - proxy.bindNativeCall(globalHandle, "method1");
         *        proxy.unbindNativeCall(globalHandle, "method1");
         *    - OR FROM THE JS-SIDE, E.G.
         *      - var foo = {}; foo.method1 = peers.SomeProxy[0].method1; foo.method1(123);
         *        - peers.SomeProxy[0].method1 SHOULD BE A READ-ONLY PROPERTY
         */

        int32_t registerNativeCall(const std::string &name, const NativeCallFnType &fn);
        bool unregisterNativeCall(const std::string &name);
        
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
        
        int32_t instanceId = -1;
        int32_t lastNativeCallId = -1;
        std::map<int32_t, NativeCall> nativeCalls;
        
        const NativeCall* getNativeCall(int32_t nativeCallId) const;
        int32_t getNativeCallId(const std::string &name) const;
        int32_t addNativeCall(const std::string &name, const NativeCallFnType &fn);
        void removeNativeCall(int32_t nativeCallId);
    };
}

#undef TARGET
#undef HANDLE
#undef FORWARD
