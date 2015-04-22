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
 * 1) HANDLE "COMPLEX EXCEPTION SITUATIONS", E.G.
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
        virtual bool exec(const std::string &source, const ReadOnlyCompileOptions &options);
        
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
        virtual bool eval(const std::string &source, const ReadOnlyCompileOptions &options, MutableHandleValue result);
        
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
         * 2) THERE SHOULD BE ONLY ONE VIRTUAL METHOD (THE ONE TAKING A HandleValue)
         *    - THE OTHER ONES SHOULD BE SHORTHAND VERSIONS
         */
        
        virtual Value call(HandleObject object, const char *functionName, const HandleValueArray& args = HandleValueArray::empty());
        virtual Value call(HandleObject object, HandleValue functionValue, const HandleValueArray& args = HandleValueArray::empty());
        virtual Value call(HandleObject object, HandleFunction function, const HandleValueArray& args = HandleValueArray::empty());
        
        // ---
        
        virtual bool apply(const NativeCall &nativeCall, CallArgs args);

        // ---
        
        /*
         * TODO:
         *
         * 1) bool clear(HandleObject object)
         *
         * 2) HOW ABOUT ADOPTING PART OF SPIDERMONKEY'S Proxy PROTOCOL?
         *    - https://github.com/mozilla/gecko-dev/blob/esr31/js/src/jsproxy.h#L175-224
         *    - SOME SIMILARITIES WITH /js/ipc/JavascriptParent.h (NOW DEPRECATED...)
         *    - THE NEW Reflect OBJECT DEFINED IN ECMA-6 SEEMS TO BE AN EVEN BETTER CANDIDATE:
         *      - https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Reflect
         */
        
        virtual JSObject* newPlainObject();
        virtual JSObject* newObject(const std::string &className, const HandleValueArray& args = HandleValueArray::empty());
        
        virtual bool hasProperty(HandleObject object, const char *name);
        virtual bool hasOwnProperty(HandleObject object, const char *name);
        virtual bool getOwnPropertyDescriptor(HandleObject object, HandleId id, MutableHandle<JSPropertyDescriptor> desc);
        
        virtual bool getProperty(HandleObject object, const char *name, MutableHandleValue result);
        virtual bool setProperty(HandleObject object, const char *name, HandleValue value);

        virtual bool defineProperty(HandleObject object, const char *name, HandleValue value, unsigned attrs = 0);
        virtual bool deleteProperty(HandleObject object, const char *name);

        //
        
        template<typename T>
        T get(HandleObject targetObject, const char *propertyName, const typename TypeTraits<T>::defaultType defaultValue = TypeTraits<T>::defaultValue());
        
        template<typename T>
        bool set(HandleObject targetObject, const char *propertyName, T &&value);
        
        bool defineProperty(HandleObject object, const char *name, HandleObject value, unsigned attrs = 0);
        
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
         * 1) template<typename T> bool append(HandleObject targetArray, T &&value)
         * 2) bool appendElements(HandleObject array, const HandleValueArray &values)
         * 3) bool appendElements(HandleObject array, const AutoValueVector &objects)
         * 4) INTEGRATION WITH JS TYPED-ARRAYS
         */
        
        virtual JSObject* newArray(size_t length = 0);
        virtual JSObject* newArray(const HandleValueArray& contents);
        
        virtual bool hasElement(HandleObject array, int index);
        virtual uint32_t getElementCount(HandleObject array);

        virtual uint32_t getLength(HandleObject array);
        virtual bool setLength(HandleObject array, size_t length);
        
        virtual bool getElement(HandleObject array, int index, MutableHandleValue result);
        virtual bool setElement(HandleObject array, int index, HandleValue value);
        
        virtual bool defineElement(HandleObject array, int index, HandleValue value, unsigned attrs = 0);
        virtual bool deleteElement(HandleObject array, int index);

        //
        
        template<typename T>
        T get(HandleObject targetArray, int elementIndex, const typename TypeTraits<T>::defaultType defaultValue = TypeTraits<T>::defaultValue());

        template<typename T>
        bool set(HandleObject targetArray, int elementIndex, T &&value);
        
        template<typename T>
        bool getElements(HandleObject array, std::vector<T> &elements, const typename TypeTraits<T>::defaultType defaultValue = TypeTraits<T>::defaultValue());
        
        template<typename T>
        bool setElements(HandleObject array, const std::vector<T> &elements);
        
        bool defineElement(HandleObject array, int index, HandleObject value, unsigned attrs = 0);
    };
    
    namespace proto
    {
        JSObject* newPlainObject();
        JSObject* newObject(const std::string &className, const HandleValueArray& args = HandleValueArray::empty());
        
        bool hasProperty(HandleObject object, const char *name);
        bool hasOwnProperty(HandleObject object, const char *name);
        bool getOwnPropertyDescriptor(HandleObject object, HandleId id, MutableHandle<JSPropertyDescriptor> desc);
        
        bool getProperty(HandleObject object, const char *name, MutableHandleValue result);
        bool setProperty(HandleObject object, const char *name, HandleValue value);
        
        bool defineProperty(HandleObject object, const char *name, HandleValue value, unsigned attrs = 0);
        bool deleteProperty(HandleObject object, const char *name);
        
        inline bool defineProperty(HandleObject object, const char *name, HandleObject value, unsigned attrs = 0)
        {
            RootedValue rooted(cx, ObjectOrNullValue(value));
            return defineProperty(object, name, rooted, attrs);
        }
        
        // ---
        
        JSObject* newArray(size_t length = 0);
        JSObject* newArray(const HandleValueArray& contents);
        
        bool hasElement(HandleObject array, int index);
        uint32_t getElementCount(HandleObject array);
        
        uint32_t getLength(HandleObject array);
        bool setLength(HandleObject array, size_t length);
        
        bool getElement(HandleObject array, int index, MutableHandleValue result);
        bool setElement(HandleObject array, int index, HandleValue value);
        
        bool defineElement(HandleObject array, int index, HandleValue value, unsigned attrs = 0);
        bool deleteElement(HandleObject array, int index);
        
        inline bool defineElement(HandleObject array, int index, HandleObject value, unsigned attrs = 0)
        {
            RootedValue rooted(cx, ObjectOrNullValue(value));
            return defineElement(array, index, rooted, attrs);
        }
    }
    
    // ---
    
    inline bool Proto::apply(const NativeCall &nativeCall, CallArgs args)
    {
        return nativeCall.fn(args);
    }
    
    // ---
    
    inline JSObject* Proto::newPlainObject() { return proto::newPlainObject(); }
    inline JSObject* Proto::newObject(const std::string &className, const HandleValueArray& args) { return proto::newObject(className, args); }
    
    inline bool Proto::hasProperty(HandleObject object, const char *name) { return proto::hasProperty(object, name); }
    inline bool Proto::hasOwnProperty(HandleObject object, const char *name) { return proto::hasOwnProperty(object, name); }
    inline bool Proto::getOwnPropertyDescriptor(HandleObject object, HandleId id, MutableHandle<JSPropertyDescriptor> desc) { return proto::getOwnPropertyDescriptor(object, id, desc); }
    
    inline bool Proto::getProperty(HandleObject object, const char *name, MutableHandleValue result) { return proto::getProperty(object, name, result); }
    inline bool Proto::setProperty(HandleObject object, const char *name, HandleValue value) { return proto::setProperty(object, name, value); }
    
    inline bool Proto::defineProperty(HandleObject object, const char *name, HandleValue value, unsigned attrs) { return proto::defineProperty(object, name, value, attrs); }
    inline bool Proto::deleteProperty(HandleObject object, const char *name) { return proto::deleteProperty(object, name); }
    
    //
    
    template<typename T>
    inline T Proto::get(HandleObject targetObject, const char *propertyName, const typename TypeTraits<T>::defaultType defaultValue)
    {
        T result;
        RootedValue value(cx);
        
        if (!getProperty(targetObject, propertyName, &value) || !convertMaybe(value, result))
        {
            result = defaultValue;
        }
        
        return result; // RVO-COMPLIANT
    }
    
    template<typename T>
    inline bool Proto::set(HandleObject targetObject, const char *propertyName, T &&value)
    {
        RootedValue rooted(cx, toValue<T>(std::forward<T>(value)));
        return setProperty(targetObject, propertyName, rooted);
    }
    
    inline bool Proto::defineProperty(HandleObject object, const char *name, HandleObject value, unsigned attrs)
    {
        return proto::defineProperty(object, name, value, attrs);
    }
    
    // ---
    
    inline JSObject* Proto::newArray(size_t length) { return proto::newArray(length); }
    inline JSObject* Proto::newArray(const HandleValueArray& contents) { return proto::newArray(contents); }

    inline bool Proto::hasElement(HandleObject array, int index) { return proto::hasElement(array, index); }
    inline uint32_t Proto::getElementCount(HandleObject array) { return proto::getElementCount(array); }
    
    inline uint32_t Proto::getLength(HandleObject array) { return proto::getLength(array); }
    inline bool Proto::setLength(HandleObject array, size_t length) { return proto::setLength(array, length); };
    
    inline bool Proto::getElement(HandleObject array, int index, MutableHandleValue result) { return proto::getElement(array, index, result); }
    inline bool Proto::setElement(HandleObject array, int index, HandleValue value) { return proto::setElement(array, index, value); }
    
    inline bool Proto::defineElement(HandleObject array, int index, HandleValue value, unsigned attrs) { return proto::defineElement(array, index, value, attrs); }
    inline bool Proto::deleteElement(HandleObject array, int index) { return proto::deleteElement(array, index); }
    
    //
    
    template<typename T>
    inline T Proto::get(HandleObject targetArray, int elementIndex, const typename TypeTraits<T>::defaultType defaultValue)
    {
        T result;
        RootedValue value(cx);
        
        if (!getElement(targetArray, elementIndex, &value) || !convertMaybe(value, result))
        {
            result = defaultValue;
        }
        
        return result; // RVO-COMPLIANT
    }
    
    template<typename T>
    inline bool Proto::set(HandleObject targetArray, int elementIndex, T &&value)
    {
        RootedValue rooted(cx, toValue<T>(std::forward<T>(value)));
        return setElement(targetArray, elementIndex, rooted);
    }
    
    /*
     * THE OVER-COMPLEXITY OF THE FOLLOWING 2 IS A CONSEQUENCE OF THE PARTIAL SUPPORT OF std::vector<bool> IN C++11
     */
    
    template<typename T>
    bool Proto::getElements(HandleObject array, std::vector<T> &elements, const typename TypeTraits<T>::defaultType defaultValue)
    {
        auto size = proto::getLength(array);
        
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
    bool Proto::setElements(HandleObject array, const std::vector<T> &elements)
    {
        if ((elements.size() > 0) && proto::setLength(array, 0))
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
    
    inline bool Proto::defineElement(HandleObject array, int index, HandleObject value, unsigned attrs)
    {
        return proto::defineElement(array, index, value, attrs);
    }
}
