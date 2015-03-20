/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

/*
 * FOLLOW-UP:
 *
 * - SHOULD EVALUATION AND OBJECT-CREATION BE PART OF THE Proto INTERFACE?
 *   - E.G. exec(), newArray(), ETC.
 *   - OR SHOULD SUCH FUNCTIONALITY BE PROVIDED BY THE "INHERENT JS CONTEXT"? (I.E. VIA THE jsp NAMESPACE)
 */

#pragma once

#include "Context.h"
#include "WrappedObject.h"
#include "WrappedValue.h"
#include "CloneBuffer.h"
#include "Barker.h"

#include "chronotext/InputSource.h"

namespace jsp
{
    class Proto
    {
    public:
        virtual ~Proto() {}
        
        /*
         * TODO INSTEAD:
         *
         * bool exec<Maybe>(const std::string &source, const ReadOnlyCompileOptions &options);
         * - UPON EXECUTION-ERROR: REPORTS EXCEPTION TO JS AND RETURNS FALSE
         *
         * bool exec<CanThrow>(const std::string &source, const ReadOnlyCompileOptions &options);
         * - UPON EXECUTION-ERROR: THROWS C++ EXCEPTION WITH "JS ERROR" EMBEDDED
         */
        virtual bool exec(const std::string &source, const ReadOnlyCompileOptions &options) = 0;
        
        /*
         * TODO INSTEAD:
         *
         * bool exec<Maybe>(const std::string &source); // UPON EXECUTION-ERROR: REPORTS EXCEPTION TO JS AND RETURNS FALSE
         * bool exec<CanThrow>(const std::string &source); // UPON EXECUTION-ERROR: THROWS C++ EXCEPTION WITH "JS ERROR" EMBEDDED
         */
        void executeScript(const std::string &source, const std::string &file = "", int line = 1);
        
        /*
         * TODO INSTEAD:
         *
         * bool exec<Maybe>(chr::InputSource<std::string>::Ref textSource);
         * - UPON INPUT-SOURCE ERROR: RETURNS FALSE
         * - UPON EXECUTION-ERROR: REPORTS EXCEPTION TO JS AND RETURNS FALSE
         *
         * bool exec<CanThrow>(chr::InputSource<std::string>::Ref textSource);
         * - UPON INPUT-SOURCE ERROR: THROWS INPUT-SOURCE EXCEPTION
         * - UPON EXECUTION-ERROR: THROWS C++ EXCEPTION WITH "JS ERROR" EMBEDDED
         */
        void executeScript(chr::InputSource::Ref inputSource);
        
        /*
         * TODO INSTEAD:
         *
         * bool eval<Maybe>(const std::string &source, const ReadOnlyCompileOptions &options, MutableHandleValue result);
         * - UPON EXECUTION-ERROR: REPORTS EXCEPTION TO JS AND RETURNS FALSE
         *
         * bool eval<CanThrow>(const std::string &source, const ReadOnlyCompileOptions &options, MutableHandleValue result);
         * - UPON EXECUTION-ERROR: THROWS C++ EXCEPTION WITH "JS ERROR" EMBEDDED
         */
        virtual bool eval(const std::string &source, const ReadOnlyCompileOptions &options, MutableHandleValue result) = 0;
        
        /*
         * TODO INSTEAD:
         *
         * WrappedValue eval<Maybe>(const std::string &source);
         * - UPON EXECUTION-ERROR: REPORTS EXCEPTION TO JS AND RETURNS "UNDEFINED" WrappedValue
         *
         * WrappedValue eval<CanThrow>(const std::string &source);
         * - UPON EXECUTION-ERROR: THROWS C++ EXCEPTION WITH "JS ERROR" EMBEDDED
         */
        JSObject* evaluateObject(const std::string &source, const std::string &file = "", int line = 1);
        
        /*
         * TODO INSTEAD:
         *
         * WrappedValue eval<Maybe>(chr::InputSource<std::string>::Ref textSource);
         * - UPON INPUT-SOURCE ERROR: RETURNS "UNDEFINED" VALUE
         * - UPON EXECUTION-ERROR: REPORTS EXCEPTION TO JS AND RETURNS "UNDEFINED" WrappedValue
         *
         * WrappedValue eval<CanThrow>(chr::InputSource<std::string>::Ref textSource);
         * - UPON INPUT-SOURCE ERROR: THROWS INPUT-SOURCE EXCEPTION
         * - UPON EXECUTION-ERROR: THROWS C++ EXCEPTION WITH "JS ERROR" EMBEDDED
         */
        JSObject* evaluateObject(chr::InputSource::Ref inputSource);
        
        // ---
        
        /*
         * TODO:
         *
         * - DECIDE IF EXECUTION-ERRORS AND RETURN-VALUES SHOULD BE HANDLED AS IN THE "FORTHCOMING PLANS" FOR evaluateObject()
         * - CHECK POSSIBLE "OVERLAP" WITH SPIDERMONKEY'S Proxy::call() AND/OR Proxy::nativeCall()
         */
        
        virtual Value callFunction(HandleObject object, const char *name, const HandleValueArray& args = HandleValueArray::empty()) = 0;
        virtual Value callFunction(HandleObject object, HandleValue function, const HandleValueArray& args = HandleValueArray::empty()) = 0;
        virtual Value callFunction(HandleObject object, HandleFunction function, const HandleValueArray& args = HandleValueArray::empty()) = 0;
        
        // ---
        
        virtual bool invokeCallback(std::function<bool(CallArgs args)> &fn, CallArgs args) = 0;

        // ---
        
        virtual JSObject* newObject() = 0;
        
        virtual bool hasProperty(HandleObject object, const char *name) = 0;
        virtual bool hasOwnProperty(HandleObject object, const char *name) = 0;
        
        virtual bool getProperty(HandleObject object, const char *name, MutableHandleValue result) = 0;
        virtual bool setProperty(HandleObject object, const char *name, HandleValue value) = 0;

        virtual bool deleteProperty(HandleObject object, const char *name) = 0;

        /*
         * TODO:
         
         * HOW ABOUT ADOPTING PART OF THE SPIDERMONKEY'S Proxy PROTOCOL?
         * - https://github.com/mozilla/gecko-dev/blob/esr31/js/src/jsproxy.h#L175-224
         * - NOTE: SOME SIMILARITIES WITH /js/ipc/JavascriptParent.h (NOW DEPRECATED)
         *
         * THE NEW Reflect OBJECT DEFINED IN ECMA-6 SEEMS TO BE AN EVER BETTER CANDIDATE:
         * - https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Reflect
         */
        
        virtual bool getOwnPropertyDescriptor(HandleObject object, HandleId id, MutableHandle<JSPropertyDescriptor> desc) = 0;

        // ---
        
        template<typename T>
        T get(HandleObject targetObject, const char *propertyName, typename TypeTraits<T>::defaultType defaultValue = TypeTraits<T>::defaultValue());
        
        template<typename T>
        inline bool set(HandleObject targetObject, const char *propertyName, T value)
        {
            RootedValue rooted(cx, toValue<T>(value));
            return setProperty(targetObject, propertyName, rooted);
        }
        
        // ---
        
        /*
         * TODO:
         *
         * 1) C++11 ITERATORS
         *
         * 2) BULK READ/WRITE OPERATIONS (I.E. BYPASSING "GENERIC" ELEMENT-LOOKUP)
         *
         * 3) INTEGRATION WITH JAVASCRIPT'S TYPED-ARRAYS
         */
        
        virtual JSObject* newArray(size_t length = 0) = 0;
        virtual JSObject* newArray(const HandleValueArray& contents) = 0;

        virtual uint32_t getLength(HandleObject array) = 0;
        
        virtual bool getElement(HandleObject array, uint32_t index, MutableHandleValue result) = 0;
        virtual bool setElement(HandleObject array, uint32_t index, HandleValue value) = 0;
        
        virtual bool deleteElement(HandleObject array, uint32_t index) = 0;

        // ---
        
        template<typename T>
        T get(HandleObject targetArray, uint32_t elementIndex);
        
        template<typename T>
        inline bool set(HandleObject targetArray, uint32_t elementIndex, T value)
        {
            RootedValue rooted(cx, toValue<T>(value));
            return setElement(targetArray, elementIndex, rooted);
        }
        
        template<typename T>
        bool getElements(HandleObject array, std::vector<T> &values)
        {
            auto size = getLength(array);
            
            values.clear();
            values.reserve(size);
            
            for (auto i = 0; i < size; i++)
            {
                values.emplace_back(get<T>(array, i));
            }
            
            return size;
        }
        
        template<typename T>
        bool setElements(HandleObject array, const std::vector<T> &values)
        {
            int index = 0;
            
            for (const auto &value : values)
            {
                if (!set(array, index++, value))
                {
                    return false;
                }
            }
            
            return true;
        }
    };
}
