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
 * 1) SHOULD EVALUATION AND OBJECT-CREATION BE PART OF THE Proto INTERFACE?
 *    - E.G. exec(), newArray(), ETC.
 *    - OR SHOULD SUCH FUNCTIONALITY BE PROVIDED BY THE "INHERENT JS CONTEXT"? (I.E. VIA THE jsp NAMESPACE)
 *
 * 2) HANDLE COMPLEX EXCEPTION SITUATIONS", E.G.
 *    - CALLING exec() FROM C++ -> CALLING C++ CODE FROM JS -> C++ EXCEPTION
 */

#pragma once

#include "jsp/Context.h"

#include "chronotext/utils/Utils.h"

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

    class Proto
    {
    public:
        virtual ~Proto() {}
        
        /*
         * TODO INSTEAD:
         *
         * bool exec<Maybe>(const std::string &source, const ReadOnlyCompileOptions &options);
         * - UPON EXECUTION-ERROR: REPORTS EXCEPTION TO JS (IF RELEVANT) AND RETURNS FALSE
         *
         * bool exec<CanThrow>(const std::string &source, const ReadOnlyCompileOptions &options);
         * - UPON EXECUTION-ERROR: THROWS C++ EXCEPTION (WITH JS-ERROR EMBEDDED IF RELEVANT)
         */
        virtual bool exec(const std::string &source, const ReadOnlyCompileOptions &options) = 0;
        
        /*
         * TODO INSTEAD:
         *
         * bool exec<Maybe>(const std::string &source); // UPON EXECUTION-ERROR: REPORTS EXCEPTION TO JS (IF RELEVANT) AND RETURNS FALSE
         * bool exec<CanThrow>(const std::string &source); // UPON EXECUTION-ERROR: THROWS C++ EXCEPTION (WITH JS-ERROR EMBEDDED IF RELEVANT)
         */
        void executeScript(const std::string &source, const std::string &file = "", int line = 1);
        
        /*
         * TODO INSTEAD:
         *
         * bool exec<Maybe>(chr::InputSource<std::string>::Ref textSource);
         * - UPON INPUT-SOURCE ERROR: RETURNS FALSE
         * - UPON EXECUTION-ERROR: REPORTS EXCEPTION TO JS (IF RELEVANT) AND RETURNS FALSE
         *
         * bool exec<CanThrow>(chr::InputSource<std::string>::Ref textSource);
         * - UPON INPUT-SOURCE ERROR: THROWS INPUT-SOURCE EXCEPTION
         * - UPON EXECUTION-ERROR: THROWS C++ EXCEPTION (WITH JS-ERROR EMBEDDED IF RELEVANT)
         */
        void executeScript(chr::InputSource::Ref inputSource);
        
        /*
         * TODO INSTEAD:
         *
         * bool eval<Maybe>(const std::string &source, const ReadOnlyCompileOptions &options, MutableHandleValue result);
         * - UPON EXECUTION-ERROR: REPORTS EXCEPTION TO JS (IF RELEVANT) AND RETURNS FALSE
         *
         * bool eval<CanThrow>(const std::string &source, const ReadOnlyCompileOptions &options, MutableHandleValue result);
         * - UPON EXECUTION-ERROR: THROWS C++ EXCEPTION (WITH JS-ERROR EMBEDDED IF RELEVANT)
         */
        virtual bool eval(const std::string &source, const ReadOnlyCompileOptions &options, MutableHandleValue result) = 0;
        
        /*
         * TODO INSTEAD:
         *
         * WrappedValue eval<Maybe>(const std::string &source);
         * - UPON EXECUTION-ERROR: REPORTS EXCEPTION TO JS (IF RELEVANT) AND RETURNS "UNDEFINED" WrappedValue
         *
         * WrappedValue eval<CanThrow>(const std::string &source);
         * - UPON EXECUTION-ERROR: THROWS C++ EXCEPTION (WITH JS-ERROR EMBEDDED IF RELEVANT)
         */
        JSObject* evaluateObject(const std::string &source, const std::string &file = "", int line = 1);
        
        /*
         * TODO INSTEAD:
         *
         * WrappedValue eval<Maybe>(chr::InputSource<std::string>::Ref textSource);
         * - UPON INPUT-SOURCE ERROR: RETURNS "UNDEFINED" VALUE
         * - UPON EXECUTION-ERROR: REPORTS EXCEPTION TO JS (IF RELEVANT) AND RETURNS "UNDEFINED" WrappedValue
         *
         * WrappedValue eval<CanThrow>(chr::InputSource<std::string>::Ref textSource);
         * - UPON INPUT-SOURCE ERROR: THROWS INPUT-SOURCE EXCEPTION
         * - UPON EXECUTION-ERROR: THROWS C++ EXCEPTION (WITH JS-ERROR EMBEDDED IF RELEVANT)
         */
        JSObject* evaluateObject(chr::InputSource::Ref inputSource);
        
        // ---
        
        /*
         * TODO:
         *
         * 1) DECIDE IF EXECUTION-ERRORS AND RETURN-VALUES SHOULD BE HANDLED AS IN WHAT'S PLANNED FOR evaluateObject()
         *
         * 2) THERE SHOULD BE ONLY ONE VIRTUAL METHOD (THE ONE TAKE A HandleValue)
         *    - THE OTHER ONES SHOULD BE SHORTHAND VERSIONS
         */
        
        virtual Value call(HandleObject object, const char *functionName, const HandleValueArray& args = HandleValueArray::empty()) = 0;
        virtual Value call(HandleObject object, HandleValue functionValue, const HandleValueArray& args = HandleValueArray::empty()) = 0;
        virtual Value call(HandleObject object, HandleFunction function, const HandleValueArray& args = HandleValueArray::empty()) = 0;
        
        // ---
        
        virtual bool apply(const NativeCall &nativeCall, CallArgs args) = 0;

        // ---
        
        /*
         * TODO:
         *
         * 1) bool defineProperty(HandleObject object, const char *name, HandleValue value, unsigned attrs)
         * 2) bool clear(HandleObject object)
         */
        
        virtual JSObject* newPlainObject() = 0;
        virtual JSObject* newObject(const std::string &className, const HandleValueArray& args = HandleValueArray::empty()) = 0;
        
        virtual bool hasProperty(HandleObject object, const char *name) = 0;
        virtual bool hasOwnProperty(HandleObject object, const char *name) = 0;
        
        virtual bool getProperty(HandleObject object, const char *name, MutableHandleValue result) = 0;
        virtual bool setProperty(HandleObject object, const char *name, HandleValue value) = 0;

        virtual bool deleteProperty(HandleObject object, const char *name) = 0;

        /*
         * TODO:
         *
         * HOW ABOUT ADOPTING PART OF SPIDERMONKEY'S Proxy PROTOCOL?
         * - https://github.com/mozilla/gecko-dev/blob/esr31/js/src/jsproxy.h#L175-224
         * - NOTE: SOME SIMILARITIES WITH /js/ipc/JavascriptParent.h (NOW DEPRECATED)
         *
         * THE NEW Reflect OBJECT DEFINED IN ECMA-6 SEEMS TO BE AN EVER BETTER CANDIDATE:
         * - https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Reflect
         */
        
        virtual bool getOwnPropertyDescriptor(HandleObject object, HandleId id, MutableHandle<JSPropertyDescriptor> desc) = 0;

        // ---
        
        template<typename T>
        inline T get(HandleObject targetObject, const char *propertyName, const typename TypeTraits<T>::defaultType defaultValue = TypeTraits<T>::defaultValue())
        {
            RootedValue value(cx);
            
            if (getProperty(targetObject, propertyName, &value))
            {
                T result;
                
                if (convertMaybe(value, &result))
                {
                    return result;
                }
            }
            
            return defaultValue;
        }
        
        template<typename T>
        inline bool set(HandleObject targetObject, const char* propertyName, T &&value)
        {
            RootedValue rooted(cx, toValue<T>(std::forward<T>(value)));
            return setProperty(targetObject, propertyName, rooted);
        }
        
        // ---
        
        /*
         * PURPOSELY NOT USING uint32_t FOR ARRAY INDICES:
         *
         * 1) BECAUSE OF THE AMBIGUITY WITH const char* WHEN INDEX IS 0
         *
         * 2) BECAUSE IT'S MORE EXPLICIT, E.G.
         *    - "CAN'T GET ELEMENT AT INDEX -1" VS "CAN'T GET ELEMENT AT INDEX 4294967295"
         */
        
        /*
         * TODO:
         *
         * 1) bool hasElement(HandleObject array, int index)
         * 2) bool defineElement(HandleObject array, int index, HandleValue value, unsigned attrs)
         * 3) template<typename T> bool append(HandleObject targetArray, T value)
         * 4) bool append(const HandleValueArray& contents)
         *
         * 5) C++11 ITERATORS
         * 6) INTEGRATION WITH JAVASCRIPT'S TYPED-ARRAYS
         */
        
        virtual JSObject* newArray(size_t length = 0) = 0;
        virtual JSObject* newArray(const HandleValueArray& contents) = 0;

        virtual size_t getLength(HandleObject array) = 0;
        virtual bool setLength(HandleObject array, size_t length) = 0;
        
        virtual bool getElement(HandleObject array, int index, MutableHandleValue result) = 0;
        virtual bool setElement(HandleObject array, int index, HandleValue value) = 0;
        
        virtual bool deleteElement(HandleObject array, int index) = 0;

        // ---
        
        template<typename T>
        inline T get(HandleObject targetArray, int elementIndex, const typename TypeTraits<T>::defaultType defaultValue = TypeTraits<T>::defaultValue())
        {
            RootedValue value(cx);
            
            if (getElement(targetArray, elementIndex, &value))
            {
                T result;
                
                if (convertMaybe(value, &result))
                {
                    return result;
                }
            }
            
            return defaultValue;
        }
        
        template<typename T>
        inline bool set(HandleObject targetArray, int elementIndex, T &&value)
        {
            RootedValue rooted(cx, toValue<T>(std::forward<T>(value)));
            return setElement(targetArray, elementIndex, rooted);
        }
        
        /*
         * THE OVER-COMPLEXITY OF THE FOLLOWING 2 IS A CONSEQUENCE OF THE PARTIAL SUPPORT OF std::vector<bool> IN C++11
         */
        
        template<typename T>
        bool getElements(HandleObject array, std::vector<T> &elements, const typename TypeTraits<T>::defaultType defaultValue = TypeTraits<T>::defaultValue())
        {
            auto size = getLength(array);
            
            if (size > 0)
            {
                elements.clear();
                elements.resize(size, TypeTraits<T>::defaultValue());

                bool assignUnconverted = TypeTraits<T>::defaultValue() != defaultValue;
                int index = 0;
                int converted = 0;
                
                RootedValue value(cx);
                
                for (typename std::vector<T>::iterator it = elements.begin(); it != elements.end(); ++it)
                {
                    if (getElement(array, index++, &value))
                    {
                        if (Convert<T>::maybe(value, it))
                        {
                            converted++;
                            continue;
                        }
                    }
                    
                    if (assignUnconverted)
                    {
                        *it = defaultValue;
                    }
                }
                
                return (index == converted);
            }
            
            return false;
        }
        
        template<typename T>
        bool setElements(HandleObject array, const std::vector<T> &elements)
        {
            if ((elements.size() > 0) && setLength(array, 0))
            {
                int index = 0;
                int converted = 0;

                RootedValue value(cx);
                
                for (typename std::vector<T>::const_iterator it = elements.begin(); it != elements.end(); ++it)
                {
                    value = toValue<T>(*it);
                    
                    if (setElement(array, index++, value))
                    {
                        converted++;
                    }
                }
                
                return (index == converted);
            }
            
            return false;
        }
    };
}
