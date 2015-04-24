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

#define TARGET(FN, ...) Proto::FN(__VA_ARGS__)
#define HANDLE(FN, ...) handler->FN(__VA_ARGS__)
#define FORWARD(FN, ...) handler ? HANDLE(FN, __VA_ARGS__) : TARGET(FN, __VA_ARGS__)

namespace jsp
{
    typedef std::function<bool(CallArgs)> NativeCallFnType;
    
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
        
        Heap<WrappedObject> peer;

        Proxy();
        Proxy(const std::string &peerName, bool isSingleton = false);
        
        ~Proxy();

        virtual Proxy* getHandler() const;
        virtual void setHandler(Proxy *handler);

        // ---
        
        virtual const PeerProperties defaultPeerProperties() const;
        
        virtual int32_t registerNativeCall(const std::string &name, const NativeCallFnType &fn);
        virtual bool unregisterNativeCall(const std::string &name);
        virtual bool apply(const NativeCall &nativeCall, CallArgs args);
        
        // ---
        
        inline bool exec(const std::string &source, const ReadOnlyCompileOptions &options) override
        {
            return FORWARD(exec, source, options);
        }
        
        inline bool eval(const std::string &source, const ReadOnlyCompileOptions &options, MutableHandleValue result) override
        {
            return FORWARD(eval, source, options, result);
        }
        
        // ---
        
        inline Value call(HandleObject object, const char *functionName, const HandleValueArray& args = HandleValueArray::empty()) override
        {
            return FORWARD(call, object, functionName, args);
        }
        
        inline Value call(HandleObject object, HandleValue functionValue, const HandleValueArray& args = HandleValueArray::empty()) override
        {
            return FORWARD(call, object, functionValue, args);
        }
        
        inline Value call(HandleObject object, HandleFunction function, const HandleValueArray& args = HandleValueArray::empty()) override
        {
            return FORWARD(call, object, function, args);
        }

        // ---
        
        inline JSObject* newPlainObject() override
        {
            return FORWARD(newPlainObject);
        }
        
        inline JSObject* newObject(const std::string &className, const HandleValueArray& args = HandleValueArray::empty()) override
        {
            return FORWARD(newObject, className, args);
        }

        inline bool hasProperty(HandleObject object, const char *name) override
        {
            return FORWARD(hasProperty, object, name);
        }
        
        inline bool hasOwnProperty(HandleObject object, const char *name) override
        {
            return FORWARD(hasOwnProperty, object, name);
        }
        
        inline bool getOwnPropertyDescriptor(HandleObject object, HandleId id, MutableHandle<JSPropertyDescriptor> desc) override
        {
            return FORWARD(getOwnPropertyDescriptor, object, id, desc);
        }
        
        inline bool getProperty(HandleObject object, const char *name, MutableHandleValue result) override
        {
            return FORWARD(getProperty, object, name, result);
        }
        
        inline bool setProperty(HandleObject object, const char *name, HandleValue value) override
        {
            return FORWARD(setProperty, object, name, value);
        }
        
        inline bool defineProperty(HandleObject object, const char *name, HandleValue value, unsigned attrs = 0)
        {
            return FORWARD(defineProperty, object, name, value, attrs);
        }
        
        inline bool deleteProperty(HandleObject object, const char *name) override
        {
            return FORWARD(deleteProperty, object, name);
        }
        
        // ---
        
        inline JSObject* newArray(size_t length = 0) override
        {
            return FORWARD(newArray, length);
        }
        
        inline JSObject* newArray(const HandleValueArray& contents) override
        {
            return FORWARD(newArray, contents);
        }
        
        inline bool hasElement(HandleObject array, int index) override
        {
            return FORWARD(hasElement, array, index);
        }
        
        inline uint32_t getElementCount(HandleObject array) override
        {
            return FORWARD(getElementCount, array);
        }
        
        inline uint32_t getLength(HandleObject array) override
        {
            return FORWARD(getLength, array);
        }
        
        inline bool setLength(HandleObject array, size_t length) override
        {
            return FORWARD(setLength, array, length);
        }
        
        inline bool getElement(HandleObject array, int index, MutableHandleValue result) override
        {
            return FORWARD(getElement, array, index, result);
        }
        
        inline bool setElement(HandleObject array, int index, HandleValue value) override
        {
            return FORWARD(setElement, array, index, value);
        }
        
        inline bool defineElement(HandleObject array, int index, HandleValue value, unsigned attrs = 0) override
        {
            return FORWARD(defineElement, array, index, value, attrs);
        }
        
        inline bool deleteElement(HandleObject array, int index) override
        {
            return FORWARD(deleteElement, array, index);
        }

    protected:
        Proxy *handler = nullptr;
        
        PeerProperties peerProperties;
        uint32_t elementIndex = 0;

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
