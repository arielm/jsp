/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#pragma once

#include "jsp/Proto.h"

#include <map>

#define TARGET(FN, ...) target->FN(__VA_ARGS__)
#define HANDLE(FN, ...) handler->FN(__VA_ARGS__)
#define FORWARD(FN, ...) handler ? HANDLE(FN, __VA_ARGS__) : TARGET(FN, __VA_ARGS__)

#define REGISTER_CALLBACK(OBJECT, CLASS, METHOD) Proxy::registerCallback(OBJECT, #METHOD, std::bind(&CLASS::METHOD, this, std::placeholders::_1), this)

namespace jsp
{
    struct Callback
    {
        std::string name;
        std::function<bool(CallArgs args)> fn;
        
        Callback(const std::string &name, const std::function<bool(CallArgs args)> &fn)
        :
        name(name),
        fn(fn)
        {}
    };
    
    class Proxy : public Proto
    {
    public:
        Proxy(Proto *target);
        Proxy(Proxy *target);

        bool setTarget(Proto *target);
        bool setTarget(Proxy *target);
        
        bool setHandler(Proto *handler);
        bool setHandler(Proxy *handler);

        ~Proxy();

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
        
        inline Value call(HandleObject object, const char *name, const HandleValueArray& args = HandleValueArray::empty()) final
        {
            return FORWARD(call, object, name, args);
        }
        
        inline Value call(HandleObject object, HandleValue function, const HandleValueArray& args = HandleValueArray::empty()) final
        {
            return FORWARD(call, object, function, args);
        }
        
        inline Value call(HandleObject object, HandleFunction function, const HandleValueArray& args = HandleValueArray::empty()) final
        {
            return FORWARD(call, object, function, args);
        }
        
        // ---
        
        inline bool applyCallback(std::function<bool(CallArgs args)> &fn, CallArgs args) final
        {
            return FORWARD(applyCallback, fn, args);
        }
        
        // ---
        
        inline JSObject* newObject() final
        {
            return FORWARD(newObject);
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
        
        inline uint32_t getLength(HandleObject array) final
        {
            return FORWARD(getLength, array);
        }
        
        inline bool getElement(HandleObject array, uint32_t index, MutableHandleValue result) final
        {
            return FORWARD(getElement, array, index, result);
        }
        
        inline bool setElement(HandleObject array, uint32_t index, HandleValue value) final
        {
            return FORWARD(setElement, array, index, value);
        }
        
        inline bool deleteElement(HandleObject array, uint32_t index) final
        {
            return FORWARD(deleteElement, array, index);
        }

    protected:
        Proto *target = nullptr;
        Proto *handler = nullptr;
        
        static void registerCallback(JS::HandleObject object, const std::string &name, const std::function<bool(CallArgs args)> &fn, Proxy *proxy);
        static bool dispatchCallback(JSContext *cx, unsigned argc, Value *vp);
        
    private:
        /*
         * TODO:
         *
         * 1) ADD unregisterCallback(JS::HandleObject object, const std::string &name):
         *    - SHOULD DELETE THE PROPERTY ASSOCIATED WITH name IN object
         *    - SHOULD REMOVE THE Callback ASSOCIATED WITH name IN INSTANCE'S callbacks
         * 2) CALLBACK MANAGEMENT SHOULD TAKE PLACE AT THE "JS COMPARTMENT LEVEL"
         */
        
        int32_t proxyId = -1;
        int32_t lastCallbackId = -1;
        std::map<int32_t, Callback> callbacks;
        
        int32_t registerProxy();
        void unregisterProxy();
        
        Callback* getCallback(int32_t callbackId);
        int32_t getCallbackId(const std::string &name);
        int32_t registerCallback(const std::string &name, const std::function<bool(CallArgs args)> &fn);
    };
}

#undef TARGET
#undef HANDLE
#undef FORWARD
