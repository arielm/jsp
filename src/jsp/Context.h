/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#pragma once

#include "jsp/Types.h"

#include "jsapi.h"
#include "jsfriendapi.h"
#include "js/GCAPI.h"
#include "js/TracingAPI.h"

#if defined(JSP_USE_PRIVATE_APIS)
#include "jsobjinlines.h"
#include "jscntxtinlines.h"
#include "jsgc.h"
#include "gc/Heap.h"
#include "gc/Marking.h"
#endif

#include <functional>
#include <map>
#include <sstream>
#include <vector>

#define BIND_STATIC1(CALLABLE) std::bind(CALLABLE, std::placeholders::_1)
#define BIND_STATIC2(CALLABLE) std::bind(CALLABLE, std::placeholders::_1, std::placeholders::_2)
#define BIND_STATIC3(CALLABLE) std::bind(CALLABLE, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)

#define BIND_INSTANCE1(CALLABLE, INSTANCE) std::bind(CALLABLE, INSTANCE, std::placeholders::_1)
#define BIND_INSTANCE2(CALLABLE, INSTANCE) std::bind(CALLABLE, INSTANCE, std::placeholders::_1, std::placeholders::_2)
#define BIND_INSTANCE3(CALLABLE, INSTANCE) std::bind(CALLABLE, INSTANCE, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)

namespace jsp
{
    using namespace JS;

    class WrappedObject;
    class WrappedValue;

    // ---
    
    extern JSRuntime *rt;
    extern JSContext *cx;
    extern Heap<JSObject*> global;
    
    inline JS::HandleObject globalHandle()
    {
        return JS::HandleObject::fromMarkedLocation(global.address());
    }
}

namespace js
{
    using jsp::WrappedObject;
    using jsp::WrappedValue;
    
    template <>
    class HeapBase<WrappedObject>
    {
    public:
        explicit operator const bool () const;
        operator JS::Handle<JSObject*> () const;
        
        template <class U>
        JS::Handle<U*> as() const;
    };
    
    template <>
    class HeapBase<WrappedValue>
    {
    public:
        explicit operator const bool () const;
        operator JS::Handle<JS::Value> () const;
    };
}

class JSP
{
public:
    typedef std::function<void(JSTracer*)> TracerCallbackFnType;
    typedef std::function<void(JSRuntime*, JSGCStatus)> GCCallbackFnType;

    static bool init();
    static void uninit();
    
    // ---
    
    static void addTracerCallback(void *instance, const TracerCallbackFnType &fn);
    static void removeTracerCallback(void *instance);
    
    static void addGCCallback(void *instance, const GCCallbackFnType &fn);
    static void removeGCCallback(void *instance);
    
    // ---

    static void assignString(std::string &target, const jschar *chars, size_t len);
    static void assignString(std::string &target, JSString *str);
    static void assignString(std::string &target, JS::HandleValue value);

    static std::string& appendString(std::string &target, const jschar *chars, size_t len);
    static std::string& appendString(std::string &target, JSString *str);
    static std::string& appendString(std::string &target, JS::HandleValue value);

    static std::string toString(const jschar *chars, size_t len);
    static std::string toString(JSString *str);
    static std::string toString(JS::HandleValue value);

    static JSFlatString* toJSString(const char *c);
    static JSFlatString* toJSString(const std::string &s);

    static bool stringEquals(JSString *str1, const char *c2);
    static bool stringEquals(JSString *str1, const std::string &s2);

    static bool stringEqualsASCII(JSString *str1, const char *c2);
    static bool stringEqualsASCII(JSString *str1, const std::string &s2);

    // ---
    
    template<class T>
    static bool convertMaybe(JS::HandleValue, T&);
    
    template<class T>
    struct Convert
    {
        inline static bool maybe(const JS::HandleValue &v, typename std::vector<T>::iterator &result)
        {
            return convertMaybe(v, *result);
        }
    };
    
    template<class T>
    inline static T convertSafely(JS::HandleValue value, const typename jsp::TypeTraits<T>::defaultType defaultValue = jsp::TypeTraits<T>::defaultValue())
    {
        T result;
        
        if (!convertMaybe(value, result))
        {
            result = defaultValue;
        }
        
        return result; // RVO-COMPLIANT
    }
    
    // ---
    
    template<typename T> static JS::Value toValue(JSObject *object);
    template<typename T> static JS::Value toValue(std::nullptr_t);
    template<typename T> static JS::Value toValue(float f);
    template<typename T> static JS::Value toValue(double d);
    template<typename T> static JS::Value toValue(int32_t i);
    template<typename T> static JS::Value toValue(uint32_t ui);
    template<typename T> static JS::Value toValue(bool b);
    template<typename T> static JS::Value toValue(const std::string &s);
    template<typename T> static JS::Value toValue(const char *c);
    
    //
    
    template<typename T>
    static JS::Value toValue(T&&);
    
    template <size_t N>
    inline static JS::Value toValue(const char (&c)[N])
    {
        return JS::StringValue(toJSString(c));
    }
    
    // ---
    
    static bool compare(const JS::Value &value, const JS::Value &other);
    static bool compare(const JS::Value &value, const std::nullptr_t);
    static bool compare(const JS::Value &value, const JSObject *other);
    static bool compare(const JS::Value &value, float other);
    static bool compare(const JS::Value &value, double other);
    static bool compare(const JS::Value &value, int32_t other);
    static bool compare(const JS::Value &value, uint32_t other);
    static bool compare(const JS::Value &value, bool other); // WARNING: THIS WILL BE PICKED BY DEFAULT WHEN NO METHOD IS DEFINED FOR A PARTICULAR TYPE
    static bool compare(const JS::Value &value, const std::string &other);
    static bool compare(const JS::Value &value, const char *other);
    
    // ---
    
    static void assignValue(JS::Value &target, std::nullptr_t);
    static void assignValue(JS::Value &target, JSObject *object);
    static void assignValue(JS::Value &target, float f);
    static void assignValue(JS::Value &target, double d);
    static void assignValue(JS::Value &target, int32_t i);
    static void assignValue(JS::Value &target, uint32_t ui);
    static void assignValue(JS::Value &target, bool b);
    static void assignValue(JS::Value &target, const std::string &s);
    static void assignValue(JS::Value &target, const char *c);
    static void assignValue(JS::Value &target, JSString *s);
    
    // ---
    
    static bool isFunction(JSObject *object);
    static bool isFunction(const JS::Value &value);
    
    static bool isArray(JSObject *object);
    static bool isArray(const JS::Value &value);
    
    static bool isIdentifier(JSString *str);
    static bool isIdentifier(const std::string &s);
    
    // ---
    
    static std::string toSource(JSObject *object);
    static std::string toSource(JS::HandleValue value);
    
    // ---
    
    static std::string stringify(JSObject *object, int indent = 2);
    static std::string stringify(JS::MutableHandleValue value, int indent = 2);
    
    static JSObject* parse(const std::string &s);
    static JSObject* parse(JS::HandleValue value);
    static JSObject* parse(JS::HandleString str);
    static JSObject* parse(const jschar *chars, size_t len);
    
    // ---
    
    static void dumpString(JSString *str);
    static void dumpAtom(JSAtom *atom);
    static void dumpObject(JSObject *obj);

    static std::string write(const JS::Value &value);
    static std::string write(jsid id);

    static char writeMarkDescriptor(void *thing);

    static char writeGCDescriptor(const JS::Value &value);
    static std::string writeTraceThingInfo(const JS::Value &value, bool details = true);
    static std::string writeDetailed(const JS::Value &value);
    
    // ---

    static bool isPoisoned(const JS::Value &value);
    static bool isHealthy(const JS::Value &value);
    static bool isInsideNursery(const JS::Value &value);
    
#if defined(DEBUG) && defined(JS_DEBUG) && defined(JSP_USE_PRIVATE_APIS)
    
    /*
     * BORROWED FROM: https://github.com/mozilla/gecko-dev/blob/esr31/js/src/gc/Marking.cpp#L101-130
     *
     * THIS IS THE ONLY VALID WAY OF CHECKING FOR POISONING, AT LEAST UNDER GENERATIONAL-GC
     * AS OF SPIDERMONKEY 31: NOT YET "INTEGRATED" IN THE OFFICIAL API
     *
     *
     * BACKGROUND:
     *
     * THE JS::IsPoisonedPtr() IMPLEMENTATION (DEFINED IN /js/public/Utility.h)
     * IS NOT WORKING AS INTENDED, AT LEAST UNDER GENERATIONAL-GC:
     *
     * IT IS EITHER RETURNING false (UNLESS THE "JSGC_ROOT_ANALYSIS" MACRO IS DEFINED)
     *
     * OTHERWISE: IT RELIES ON THE "JS_FREE_PATTERN" MACRO WHICH IS NOT DECLARED (ANYMORE)
     * - IT WAS DECLARED AS 0xDA UNTIL ~SPIDERMONKEY 24
     * - IT WAS PROBABLY REMOVED (STARTING FROM ~SPIDERMONKEY 28) IN PREVISION OF SOME "REVISED POISONING" POLICY
     *
     *
     * MOSTLY FOR INTERNAL USAGE (JSP::isHealthy(T*) IS RECOMMENDED INSTEAD)
     */
    template<typename T>
    static inline bool isPoisoned(T *thing)
    {
        static_assert(sizeof(T) >= sizeof(js::gc::FreeSpan) + sizeof(uint32_t),
                      "Ensure it is well defined to look past any free span that "
                      "may be embedded in the thing's header when freed.");
        const uint8_t poisonBytes[] = {
            JS_FRESH_NURSERY_PATTERN,
            JS_SWEPT_NURSERY_PATTERN,
            JS_ALLOCATED_NURSERY_PATTERN,
            JS_FRESH_TENURED_PATTERN,
            JS_SWEPT_TENURED_PATTERN,
            JS_ALLOCATED_TENURED_PATTERN,
            JS_SWEPT_CODE_PATTERN,
            JS_SWEPT_FRAME_PATTERN
        };
        const int numPoisonBytes = sizeof(poisonBytes) / sizeof(poisonBytes[0]);
        uint32_t *p = reinterpret_cast<uint32_t *>(reinterpret_cast<js::gc::FreeSpan *>(thing) + 1);
        // Note: all free patterns are odd to make the common, not-poisoned case a single test.
        if ((*p & 1) == 0)
            return false;
        for (int i = 0; i < numPoisonBytes; ++i) {
            const uint8_t pb = poisonBytes[i];
            const uint32_t pw = pb | (pb << 8) | (pb << 16) | (pb << 24);
            if (*p == pw)
                return true;
        }
        return false;
    }
    
    /*
     * USING JSP::isPoisoned(T*) STANDALONE IS ACTUALLY NOT ENOUGH:
     *
     * 1) THE thing COULD BE NULL
     *
     * 2) THE "POISON PATTERN" CAN STILL BE DETECTED IN NEWLY ALLOCATED "CELLS"
     *    - HENCE THE COMPLICATED STUFF TAKING PLACE IN isHealthy(T*)
     *    - ALL THE COMPLEX SITUATIONS SEEM TO BE HANDLED
     *      - MOST COMPLEX CASE: JSString INSIDE JS::Value
     *      - TODO: FOLLOW-UP
     */
    template<typename T>
    static bool isHealthy(T *thing)
    {
        if (thing)
        {
            /*
             * REFERENCES:
             * https://github.com/mozilla/gecko-dev/blob/esr31/js/src/jsfriendapi.cpp#L737-745
             * https://github.com/mozilla/gecko-dev/blob/esr31/js/src/gc/Marking.cpp#L193-200
             *
             * IN PRACTICE:
             * thing->zone()->runtimeFromMainThread()->isHeapBusy() SEEMS TO NEVER RETURN TRUE
             *
             * SEE ALSO: writeMarkDescriptor(void*)
             */
            
            auto cell = static_cast<js::gc::Cell*>(thing);
            
            if (cell->isTenured() && cell->arenaHeader()->allocated())
            {
                if (!InFreeList(cell->arenaHeader(), thing))
                {
                    if (!cell->isMarked(js::gc::GRAY))
                    {
                        return true;
                    }
                }
            }
            
            return !isPoisoned(thing);
        }
        
        return false;
    }
    
    template<typename T>
    static bool isInsideNursery(T *thing)
    {
        if (thing)
        {
            auto cell = static_cast<js::gc::Cell*>(thing);
            return !cell->isTenured();
        }
        
        return false;
    }
    
    template<typename T>
    static std::string writeDetailed(T *thing)
    {
        if (thing)
        {
            std::stringstream ss;
            ss << thing;
            
            const auto traceThingInfo = writeTraceThingInfo(thing, true);
            
            if (!traceThingInfo.empty())
            {
                ss << " {" << traceThingInfo << "}";
            }
            
            auto gcDescriptor = writeGCDescriptor(thing);
            
            if (gcDescriptor != '?')
            {
                ss << " [" << gcDescriptor << "]";
            }
            
            return ss.str();
        }
        
        return "";
    }

    template<typename T>
    static char writeGCDescriptor(T *thing)
    {
        if (thing)
        {
            if (isInsideNursery(thing))
            {
                if (isPoisoned(thing))
                {
                    return 'P';
                }
                
                return 'n';
            }
            
            // ---
            
            auto markDescriptor = writeMarkDescriptor(thing);
            
            if ((markDescriptor != 'B') && (markDescriptor != 'W'))
            {
                if (isPoisoned(thing))
                {
                    return 'P';
                }
            }
            
            return markDescriptor;
        }
        
        return '?';
    }

    /*
     * MOSTLY FOR INTERNAL USAGE
     */
    template<typename T>
    static std::string writeTraceThingInfo(T *thing, bool details = true);

#else
    
    template<typename T>
    static inline bool isPoisoned(T *thing)
    {
        return false;
    }
    
    template<typename T>
    static bool isHealthy(T *thing)
    {
        return false;
    }
    
    template<typename T>
    static bool isInsideNursery(T *thing)
    {
        return false;
    }
    
    template<typename T>
    static std::string writeDetailed(T *thing)
    {
        return "";
    }
    
    template<typename T>
    static char writeGCDescriptor(T *thing)
    {
        return '?';
    }
    
    template<typename T>
    static std::string writeTraceThingInfo(T *thing, bool details = true)
    {
        return "";
    }
    
#endif

    // ---
    
    static void forceGC();
    static void setGCZeal(uint8_t zeal, uint32_t frequency = 100);
    
private:
    struct Stringifier
    {
        static bool callback(const jschar *buf, uint32_t len, void *data);
    };

    static bool initialized;

    static std::map<void*, TracerCallbackFnType> tracerCallbacks;
    static std::map<void*, GCCallbackFnType> gcCallbacks;

    static void tracerCallback(JSTracer *trc, void *data);
    static void gcCallback(JSRuntime *rt, JSGCStatus status, void *data);
    
    // ---
    
    static constexpr size_t TRACE_BUFFER_SIZE = 256;
    static char traceBuffer[TRACE_BUFFER_SIZE];
};

// ---

inline void JSP::assignString(std::string &target, JS::HandleValue value)
{
    return assignString(target, ToString(jsp::cx, value));
}

inline std::string& JSP::appendString(std::string &target, JS::HandleValue value)
{
    return appendString(target, ToString(jsp::cx, value));
}

inline std::string JSP::toString(JS::HandleValue value)
{
    return toString(ToString(jsp::cx, value));
}

inline JSFlatString* JSP::toJSString(const std::string &s)
{
    return toJSString(s.data());
}

inline bool JSP::stringEquals(JSString *str1, const std::string &s2)
{
    return stringEquals(str1, s2.data());
}

inline bool JSP::stringEqualsASCII(JSString *str1, const std::string &s2)
{
    return stringEqualsASCII(str1, s2.data());
}

// ---

template <>
inline bool JSP::convertMaybe(JS::HandleValue value, JSObject *&result)
{
    if (value.isObjectOrNull())
    {
        result = value.toObjectOrNull();
        return true;
    }
    
    return false;
}

template <>
inline bool JSP::convertMaybe(JS::HandleValue value, float &result)
{
    if (value.isDouble())
    {
        result = float(value.toDouble());
        return true;
    }
    
    return false;
}

template <>
inline bool JSP::convertMaybe(JS::HandleValue value, double &result)
{
    if (value.isDouble())
    {
        result = value.toDouble();
        return true;
    }
    
    return false;
}

template <>
inline bool JSP::convertMaybe(JS::HandleValue value, int32_t &result)
{
    if (value.isInt32())
    {
        result = value.toInt32();
        return true;
    }
    
    return false;
}

template <>
inline bool JSP::convertMaybe(JS::HandleValue value, uint32_t &result)
{
    if (value.isInt32())
    {
        result = uint32_t(value.toInt32());
        return true;
    }
    
    if (value.isDouble())
    {
        result = uint32_t(value.toDouble());
        return true;
    }
    
    return false;
}

template <>
inline bool JSP::convertMaybe(JS::HandleValue value, bool &result)
{
    if (!value.isUndefined())
    {
        result = ToBoolean(value); // INFAILIBLE, ACCORDING TO JAVASCRIPT RULES
        return true;
    }
    
    return false;
}

template <>
inline bool JSP::convertMaybe(JS::HandleValue value, std::string &result)
{
    if (!value.isUndefined())
    {
        assignString(result, value); // INFAILIBLE, POSSIBLY SLOW
        return true;
    }
    
    return false;
}

//

template <>
struct JSP::Convert<bool>
{
    inline static bool maybe(const JS::HandleValue &v, std::vector<bool>::iterator &result)
    {
        if (!v.isUndefined())
        {
            *result = ToBoolean(v); // INFAILIBLE, ACCORDING TO JAVASCRIPT RULES
            return true;
        }
        
        return false;
    }
};

// ---

template<typename T>
inline JS::Value JSP::toValue(JSObject *object)
{
    return JS::ObjectOrNullValue(object);
}

template<typename T>
inline JS::Value JSP::toValue(std::nullptr_t)
{
    return JS::NullValue();
}

template<typename T>
inline JS::Value JSP::toValue(float f)
{
    return JS::DoubleValue(f);
}

template<typename T>
inline JS::Value JSP::toValue(double d)
{
    return JS::DoubleValue(d);
}

template<typename T>
inline JS::Value JSP::toValue(int32_t i)
{
    return JS::Int32Value(i);
}

template<typename T>
inline JS::Value JSP::toValue(uint32_t ui)
{
    return JS::NumberValue(ui);
}

template<typename T>
inline JS::Value JSP::toValue(bool b)
{
    return JS::BooleanValue(b);
}

template <typename T>
inline JS::Value JSP::toValue(const std::string &s)
{
    return JS::StringValue(toJSString(s));
}

template<typename T>
inline JS::Value JSP::toValue(const char *c)
{
    return JS::StringValue(toJSString(c));
}

//

template <>
inline JS::Value JSP::toValue(std::nullptr_t&&)
{
    return JS::NullValue();
}

template <>
inline JS::Value JSP::toValue(JSObject &object)
{
    return JS::ObjectValue(object);
}

template <>
inline JS::Value JSP::toValue(JS::RootedObject &object)
{
    return JS::ObjectOrNullValue(object);
}

template <>
inline JS::Value JSP::toValue(JS::HandleObject &object)
{
    return JS::ObjectOrNullValue(object);
}

template <>
inline JS::Value JSP::toValue(JS::HandleObject &&object)
{
    return JS::ObjectOrNullValue(object);
}

template <>
inline JS::Value JSP::toValue(JSObject *&&object)
{
    return JS::ObjectOrNullValue(object);
}

template <>
inline JS::Value JSP::toValue(float &f)
{
    return JS::DoubleValue(f);
}

template <>
inline JS::Value JSP::toValue(float &&f)
{
    return JS::DoubleValue(f);
}

template <>
inline JS::Value JSP::toValue(double &d)
{
    return JS::DoubleValue(d);
}

template <>
inline JS::Value JSP::toValue(double &&d)
{
    return JS::DoubleValue(d);
}

template <>
inline JS::Value JSP::toValue(int32_t &i)
{
    return JS::Int32Value(i);
}

template <>
inline JS::Value JSP::toValue(int32_t &&i)
{
    return JS::Int32Value(i);
}

template <>
inline JS::Value JSP::toValue(uint32_t &ui)
{
    return JS::NumberValue(ui);
}

template <>
inline JS::Value JSP::toValue(uint32_t &&ui)
{
    return JS::NumberValue(ui);
}

template <>
inline JS::Value JSP::toValue(bool &b)
{
    return JS::BooleanValue(b);
}

template <>
inline JS::Value JSP::toValue(bool &&b)
{
    return JS::BooleanValue(b);
}

template <>
inline JS::Value JSP::toValue(std::string &s)
{
    return JS::StringValue(toJSString(s));
}

template <>
inline JS::Value JSP::toValue(const std::string &s)
{
    return JS::StringValue(toJSString(s));
}

template <>
inline JS::Value JSP::toValue(const char *&c)
{
    return JS::StringValue(toJSString(c));
}

// ---

inline bool JSP::compare(const JS::Value &value, const JS::Value &other)
{
    if (value.isString() && other.isString())
    {
        bool result;
        
        if (js::EqualStrings(jsp::cx, value.toString(), other.toString(), &result)) // ASSERTION: CAN'T TRIGGER GC?
        {
            return result;
        }
        
        return false;
    }
    
    return value == other;
}

inline bool JSP::compare(const JS::Value &value, const std::nullptr_t)
{
    return value.isNull();
}

inline bool JSP::compare(const JS::Value &value, const JSObject *other)
{
    if (value.isObject())
    {
        return other == value.toObjectOrNull();
    }
    
    return false;
}

inline bool JSP::compare(const JS::Value &value, float other)
{
    if (value.isDouble())
    {
        return other == float(value.toDouble());
    }
    
    return false;
}

inline bool JSP::compare(const JS::Value &value, double other)
{
    if (value.isDouble())
    {
        return other == value.toDouble();
    }
    
    return false;
}

inline bool JSP::compare(const JS::Value &value, int32_t other)
{
    if (value.isInt32())
    {
        return other == value.toInt32();
    }
    
    return false;
}

inline bool JSP::compare(const JS::Value &value, uint32_t other)
{
    if (value.isInt32())
    {
        return other == uint32_t(value.toInt32());
    }
    
    if (value.isDouble())
    {
        return other == uint32_t(value.toDouble());
    }
    
    return false;
}

inline bool JSP::compare(const JS::Value &value, bool other)
{
    JS::RootedValue rooted(jsp::cx, value);
    return ToBoolean(rooted) == other; // INFAILIBLE, ACCORDING TO JAVASCRIPT RULES
}

inline bool JSP::compare(const JS::Value &value, const std::string &other)
{
    if (!value.isNullOrUndefined()) // ACCORDING TO JAVASCRIPT RULES
    {
        JS::RootedValue rooted(jsp::cx, value);
        return stringEquals(ToString(jsp::cx, rooted), other); // INFAILIBLE, POSSIBLY SLOW
    }
    
    return false;
}

inline bool JSP::compare(const JS::Value &value, const char *other)
{
    if (!value.isNullOrUndefined()) // ACCORDING TO JAVASCRIPT RULES
    {
        JS::RootedValue rooted(jsp::cx, value);
        return stringEquals(ToString(jsp::cx, rooted), other); // INFAILIBLE, POSSIBLY SLOW
    }
    
    return false;
}

// ---

inline void JSP::assignValue(JS::Value &target, std::nullptr_t)
{
    target.setNull();
}

inline void JSP::assignValue(JS::Value &target, JSObject *object)
{
    target.setObjectOrNull(object);
}

inline void JSP::assignValue(JS::Value &target, float f)
{
    target.setDouble(f);
}

inline void JSP::assignValue(JS::Value &target, double d)
{
    target.setDouble(d);
}

inline void JSP::assignValue(JS::Value &target, int32_t i)
{
    target.setInt32(i);
}

inline void JSP::assignValue(JS::Value &target, uint32_t ui)
{
    target.setNumber(ui);
}

inline void JSP::assignValue(JS::Value &target, bool b)
{
    target.setBoolean(b);
}

inline void JSP::assignValue(JS::Value &target, const std::string &s)
{
    target.setString(toJSString(s));
}

inline void JSP::assignValue(JS::Value &target, const char *c)
{
    target.setString(toJSString(c));
}

inline void JSP::assignValue(JS::Value &target, JSString *s)
{
    target.setString(s);
}
