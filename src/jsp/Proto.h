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
 * 1) HANDLE "COMPLEX EXCEPTION SITUATIONS" IN exec(), eval() AND call()
 *    - E.G. CALLING exec() FROM C++ -> WHICH IN TURN CALLS C++ CODE FROM JS NATIVE-CALLBACK -> WHICH IN TURN THROWS C++ EXCEPTION
 */

#pragma once

#include "jsp/Context.h"

#include "chronotext/InputSource.h"

namespace jsp
{
    class Proto : public JSP
    {
    public:
        /*
         * TODO INSTEAD:
         *
         * bool exec<Maybe>(const std::string &source, const ReadOnlyCompileOptions &options);
         * - UPON EXECUTION-ERROR: REPORTS EXCEPTION TO JS (IF RELEVANT) AND RETURNS FALSE
         *
         * bool exec<CanThrow>(const std::string &source, const ReadOnlyCompileOptions &options);
         * - UPON EXECUTION-ERROR: THROWS C++ EXCEPTION (WITH JS-ERROR EMBEDDED IF RELEVANT)
         */
        static bool exec(const std::string &source, const ReadOnlyCompileOptions &options);
        
        /*
         * TODO INSTEAD:
         *
         * bool exec<Maybe>(const std::string &source); // UPON EXECUTION-ERROR: REPORTS EXCEPTION TO JS (IF RELEVANT) AND RETURNS FALSE
         * bool exec<CanThrow>(const std::string &source); // UPON EXECUTION-ERROR: THROWS C++ EXCEPTION (WITH JS-ERROR EMBEDDED IF RELEVANT)
         */
        static void executeScript(const std::string &source, const std::string &file = "", int line = 1);
        
        /*
         * TODO INSTEAD:
         *
         * bool exec<Maybe>(chr::InputSource::Ref inputSource);
         * - UPON INPUT-SOURCE ERROR: RETURNS FALSE
         * - UPON EXECUTION-ERROR: REPORTS EXCEPTION TO JS (IF RELEVANT) AND RETURNS FALSE
         *
         * bool exec<CanThrow>(chr::InputSource::Ref inputSource);
         * - UPON INPUT-SOURCE ERROR: THROWS INPUT-SOURCE EXCEPTION
         * - UPON EXECUTION-ERROR: THROWS C++ EXCEPTION (WITH JS-ERROR EMBEDDED IF RELEVANT)
         */
        static void executeScript(chr::InputSource::Ref inputSource);
        
        /*
         * TODO INSTEAD:
         *
         * bool eval<Maybe>(const std::string &source, const ReadOnlyCompileOptions &options, MutableHandleValue result);
         * - UPON EXECUTION-ERROR: REPORTS EXCEPTION TO JS (IF RELEVANT) AND RETURNS FALSE
         *
         * bool eval<CanThrow>(const std::string &source, const ReadOnlyCompileOptions &options, MutableHandleValue result);
         * - UPON EXECUTION-ERROR: THROWS C++ EXCEPTION (WITH JS-ERROR EMBEDDED IF RELEVANT)
         */
        static bool eval(const std::string &source, const ReadOnlyCompileOptions &options, MutableHandleValue result);
        
        /*
         * TODO INSTEAD:
         *
         * WrappedValue eval<Maybe>(const std::string &source);
         * - UPON EXECUTION-ERROR: REPORTS EXCEPTION TO JS (IF RELEVANT) AND RETURNS "UNDEFINED" WrappedValue
         *
         * WrappedValue eval<CanThrow>(const std::string &source);
         * - UPON EXECUTION-ERROR: THROWS C++ EXCEPTION (WITH JS-ERROR EMBEDDED IF RELEVANT)
         */
        static JSObject* evaluateObject(const std::string &source, const std::string &file = "", int line = 1);
        
        /*
         * TODO INSTEAD:
         *
         * WrappedValue eval<Maybe>(chr::InputSource::Ref inputSource);
         * - UPON INPUT-SOURCE ERROR: RETURNS "UNDEFINED" VALUE
         * - UPON EXECUTION-ERROR: REPORTS EXCEPTION TO JS (IF RELEVANT) AND RETURNS "UNDEFINED" WrappedValue
         *
         * WrappedValue eval<CanThrow>(chr::InputSource::Ref inputSource);
         * - UPON INPUT-SOURCE ERROR: THROWS INPUT-SOURCE EXCEPTION
         * - UPON EXECUTION-ERROR: THROWS C++ EXCEPTION (WITH JS-ERROR EMBEDDED IF RELEVANT)
         */
        static JSObject* evaluateObject(chr::InputSource::Ref inputSource);
        
        // ---
        
        /*
         * TODO:
         *
         * CONSIDER HANDLING EXECUTION-ERRORS AND RETURN-VALUES SIMILARELY AS WHAT'S PLANNED FOR evaluateObject()
         *
         * THERE SHOULD BE ONLY ONE VIRTUAL METHOD (THE ONE TAKING A HandleValue)
         * - THE OTHER ONES SHOULD BE SHORTHAND VERSIONS
         */

        static Value call(HandleObject object, const char *functionName, const HandleValueArray& args = HandleValueArray::empty());
        static Value call(HandleObject object, HandleValue functionValue, const HandleValueArray& args = HandleValueArray::empty());
        static Value call(HandleObject object, HandleFunction function, const HandleValueArray& args = HandleValueArray::empty());
        
        // ---
        
        /*
         * TODO:
         *
         * 1) bool clear(HandleObject object)
         */

        static JSObject* newPlainObject();
        static JSObject* newObject(const std::string &className, const HandleValueArray& args = HandleValueArray::empty());
        
        static bool hasProperty(HandleObject object, const char *name);
        static bool hasOwnProperty(HandleObject object, const char *name);
        
        static bool getProperty(HandleObject object, const char *name, MutableHandleValue result);
        static bool setProperty(HandleObject object, const char *name, HandleValue value);
        
        static bool defineProperty(HandleObject object, const char *name, HandleValue value, unsigned attrs = 0);
        static bool deleteProperty(HandleObject object, const char *name);
        
        //
        
        template<typename T>
        static T get(HandleObject targetObject, const char *propertyName, const typename TypeTraits<T>::defaultType defaultValue = TypeTraits<T>::defaultValue());
        
        template<typename T>
        static bool set(HandleObject targetObject, const char *propertyName, T &&value);
        
        template<typename T>
        static bool define(HandleObject targetObject, const char *propertyName, T &&value, unsigned attrs = 0);
        
        // ---
        
        /*
         * PURPOSELY NOT USING uint32_t FOR ARRAY INDICES:
         *
         * 1) BECAUSE OF THE AMBIGUITY WITH const char* WHEN INDEX IS 0
         * 2) BECAUSE "CAN'T GET ELEMENT AT INDEX -1" IS MORE EXPLICIT THAN "CAN'T GET ELEMENT AT INDEX 4294967295"
         */
        
        /*
         * TODO:
         *
         * 1) template<typename T> bool append(HandleObject targetArray, T &&value)
         * 2) bool appendElements(HandleObject array, const HandleValueArray &values)
         * 3) bool appendElements(HandleObject array, const AutoValueVector &objects)
         * 4) INTEGRATION WITH JS TYPED-ARRAYS
         */
        
        static JSObject* newArray(size_t length = 0);
        static JSObject* newArray(const HandleValueArray& contents);
        
        static bool hasElement(HandleObject array, int index);
        static uint32_t getElementCount(HandleObject array);
        
        static uint32_t getLength(HandleObject array);
        static bool setLength(HandleObject array, size_t length);
        
        static bool getElement(HandleObject array, int index, MutableHandleValue result);
        static bool setElement(HandleObject array, int index, HandleValue value);
        
        static bool defineElement(HandleObject array, int index, HandleValue value, unsigned attrs = 0);
        static bool deleteElement(HandleObject array, int index);
        
        //
        
        template<typename T>
        static T get(HandleObject targetArray, int elementIndex, const typename TypeTraits<T>::defaultType defaultValue = TypeTraits<T>::defaultValue());
        
        template<typename T>
        static bool set(HandleObject targetArray, int elementIndex, T &&value);

        template<typename T>
        static bool define(HandleObject targetArray, int elementIndex, T &&value, unsigned attrs = 0);

        /*
         * TODO INSTEAD (ASSUMING IT IS RVO-COMPLIANT):
         * std::vector<T> getElements(HandleObject array, const typename TypeTraits<T>::defaultType defaultValue = ...)
         */
        template<typename T>
        static bool getElements(HandleObject array, std::vector<T> &elements, const typename TypeTraits<T>::defaultType defaultValue = TypeTraits<T>::defaultValue());
        
        /*
         * TODO INSTEAD:
         * bool appendElements(HandleObject array, const std::vector<T> &elements)
         */
        template<typename T>
        static bool setElements(HandleObject array, const std::vector<T> &elements);
    };
    
    // ---
    
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
    
    template<typename T>
    inline bool Proto::define(HandleObject targetObject, const char *propertyName, T &&value, unsigned attrs)
    {
        RootedValue rooted(cx, toValue<T>(std::forward<T>(value)));
        return defineProperty(targetObject, propertyName, rooted, attrs);
    }
    
    // ---
    
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
    
    template<typename T>
    inline bool Proto::define(HandleObject targetArray, int elementIndex, T &&value, unsigned attrs)
    {
        RootedValue rooted(cx, toValue<T>(std::forward<T>(value)));
        return defineElement(targetArray, elementIndex, rooted, attrs);
    }
    
    /*
     * THE OVER-COMPLEXITY OF THE FOLLOWING 2 IS A CONSEQUENCE OF THE PARTIAL SUPPORT OF std::vector<bool> IN C++11
     */
    
    template<typename T>
    bool Proto::getElements(HandleObject array, std::vector<T> &elements, const typename TypeTraits<T>::defaultType defaultValue)
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
    bool Proto::setElements(HandleObject array, const std::vector<T> &elements)
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
}
