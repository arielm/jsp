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
         * TODO:
         *
         * 1) C++11 ITERATORS?
         * 2) INTEGRATION WITH JS TYPED-ARRAYS?
         */

        /*
         * PURPOSELY USING int INSTEAD OF uint32_t FOR ARRAY-INDEX PARAMETERS:
         *
         * 1) BECAUSE OF THE AMBIGUITY WITH const char* WHEN INDEX IS 0
         * 2) BECAUSE "CAN'T GET ELEMENT AT INDEX -1" IS MORE EXPLICIT THAN "CAN'T GET ELEMENT AT INDEX 4294967295"
         */

        static JSObject* newArray(size_t length = 0);
        static JSObject* newArray(const HandleValueArray& contents);
        
        static bool hasElement(HandleObject array, int index);
        static uint32_t getElementCount(HandleObject array);
        
        static uint32_t getLength(HandleObject array);
        static bool setLength(HandleObject array, size_t length);
        
        static bool getElement(HandleObject array, int index, MutableHandleValue result);
        static bool setElement(HandleObject array, int index, HandleValue value);
        static bool appendElement(HandleObject array, HandleValue value);
        
        static bool defineElement(HandleObject array, int index, HandleValue value, unsigned attrs = 0);
        static bool deleteElement(HandleObject array, int index);
        
        static uint32_t getElements(HandleObject sourceArray, AutoValueVector &elements);
        static uint32_t appendElements(HandleObject targetArray, const HandleValueArray &elements);

        //
        
        template<typename T>
        static T get(HandleObject targetArray, int elementIndex, const typename TypeTraits<T>::defaultType defaultValue = TypeTraits<T>::defaultValue());
        
        template<typename T>
        static bool set(HandleObject targetArray, int elementIndex, T &&value);
        
        template<typename T>
        static bool append(HandleObject targetArray, T &&value);

        template<typename T>
        static bool define(HandleObject targetArray, int elementIndex, T &&value, unsigned attrs = 0);

        template<typename T>
        static std::vector<T> getElements(HandleObject sourceArray, const typename TypeTraits<T>::defaultType defaultValue = TypeTraits<T>::defaultValue());
        
        template<typename T>
        static uint32_t appendElements(HandleObject targetArray, const std::vector<T> &elements);
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
    inline bool Proto::append(HandleObject targetArray, T &&value)
    {
        RootedValue rooted(cx, toValue<T>(std::forward<T>(value)));
        return appendElement(targetArray, rooted);
    }
    
    template<typename T>
    inline bool Proto::define(HandleObject targetArray, int elementIndex, T &&value, unsigned attrs)
    {
        RootedValue rooted(cx, toValue<T>(std::forward<T>(value)));
        return defineElement(targetArray, elementIndex, rooted, attrs);
    }
    
    /*
     * REGARDING THE USE OF vector<T>::iterator IN Proto::getElements() AND Proto::setElements():
     * - IT IS NECESSARY BECAUSE std::vector<bool> IS NOT FULLY SUPPORTED IN C++11 (E.G. emplace_back() IS MISSING FOR THAT TYPE...)
     *
     * ANOTHER ADVANTAGE OF USING A VECTOR ITERATOR, SPECIFIC TO Proto::getElements():
     * - RESIZING THE VECTOR UP-FRONT AND ASSIGNING VALUES "IN PLACE" REDUCES DATA-COPYING (MOSTLY RELEVANT FOR STRINGS...)
     */
    
    template<typename T>
    std::vector<T> Proto::getElements(HandleObject sourceArray, const typename TypeTraits<T>::defaultType defaultValue)
    {
        auto size = getLength(sourceArray);
        int index = 0;

        std::vector<T> elements;
        elements.resize(size);
        
        RootedValue value(cx);
        
        for (typename std::vector<T>::iterator it = elements.begin(); it != elements.end(); ++it)
        {
            if (getElement(sourceArray, index++, &value))
            {
                if (Convert<T>::maybe(value, it))
                {
                    continue;
                }
            }
            
            *it = defaultValue;
        }
        
        return elements; // RVO-COMPLIANT
    }
    
    template<typename T>
    uint32_t Proto::appendElements(HandleObject targetArray, const std::vector<T> &elements)
    {
        uint32_t appendCount = 0;
        
        if (isArray(targetArray))
        {
            int index = getLength(targetArray);
            
            RootedValue value(cx);
            
            for (typename std::vector<T>::const_iterator it = elements.begin(); it != elements.end(); ++it)
            {
                value = toValue<T>(*it);
                
                if (setElement(targetArray, index++, value))
                {
                    appendCount++;
                }
            }
        }
        
        return appendCount;
    }
}
