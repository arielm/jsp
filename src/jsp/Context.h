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
#include <cassert>

#include "unicode/unistr.h" // TODO: TRY REMOVING THIS DEPENDENCY (CURRENTLY USED FOR UTF8 <-> UTF16 CONVERSION)

namespace jsp
{
    using namespace JS;

    class WrappedObject;
    class WrappedValue;

    // ---
    
    extern JSRuntime *rt;
    extern JSContext *cx;
    extern Heap<JSObject*> global;
    
    JSRuntime* runtime();
    JSContext* context();
    HandleObject globalHandle();
    
    bool postInit();
    void preShutdown();
    
    // ---
    
    void addTracer(void *tracer, std::function<void(JSTracer*)> fn);
    void removeTracer(void *tracer);
    
    // ---
    
    inline std::string toString(const jschar *s)
    {
        std::string tmp;
        return UnicodeString(reinterpret_cast<const UChar*>(s)).toUTF8String(tmp);
    }
    
    inline std::string toString(HandleString s)
    {
        JSAutoByteString tmp;
        return tmp.encodeUtf8(cx, s) ? tmp.ptr() : "";
    }
    
    inline JSString* toJSString(const char *s)
    {
        return JS_NewUCStringCopyZ(cx, reinterpret_cast<const jschar*>(UnicodeString::fromUTF8(s).getBuffer()));
    }
    
    // ---
    
    bool toObjectMaybe(HandleValue &&value, JSObject **result);
    bool toFloat32Maybe(HandleValue &&value, float *result);
    bool toFloat64Maybe(HandleValue &&value, double *result);
    bool toInt32Maybe(HandleValue &&value, int32_t *result);
    bool toUInt32Maybe(HandleValue &&value, uint32_t *result);

    inline bool toBoolean(HandleValue &&value) // INFAILIBLE, POSSIBLY SLOW
    {
        return ToBoolean(std::forward<HandleValue>(value));
    }
    
    inline std::string toString(HandleValue &&value) // INFAILIBLE, POSSIBLY SLOW
    {
        RootedString rooted(cx, ToString(cx, std::forward<HandleValue>(value)));
        return toString(rooted);
    }
    
    // ---
    
    JSObject* toObjectSafely(HandleValue &&value, JSObject *defaultValue = nullptr);
    float toFloat32Safely(HandleValue &&value, float defaultValue = 0);
    double toFloat64Safely(HandleValue &&value, double defaultValue = 0);
    int32_t toInt32Safely(HandleValue &&value, int32_t defaultValue = 0);
    uint32_t toUInt32Safely(HandleValue &&value, uint32_t defaultValue = 0);
    
    // ---

    inline bool compare(const Value &lhs, const Value &rhs)
    {
        if (lhs.isString() && rhs.isString())
        {
            int32_t result;
            
            if (JS_CompareStrings(cx, lhs.toString(), rhs.toString(), &result))
            {
                return (result == 0);
            }
            
            return false;
        }
        
        return lhs == rhs;
    }
    
    inline bool compare(const Value &value, const JSObject *object)
    {
        if (value.isObject())
        {
            return object == value.toObjectOrNull();
        }
        
        return false;
    }
    
    inline bool compare(const Value &value, float f)
    {
        if (value.isDouble())
        {
            return f == float(value.toDouble());
        }
        
        return false;
    }
    
    inline bool compare(const Value &value, double d)
    {
        if (value.isDouble())
        {
            return d == value.toDouble();
        }
        
        return false;
    }
    
    inline bool compare(const Value &value, int32_t i)
    {
        if (value.isInt32())
        {
            return i == value.toInt32();
        }
        
        return false;
    }
    
    inline bool compare(const Value &value, uint32_t ui)
    {
        if (value.isInt32())
        {
            return ui == uint32_t(value.toInt32());
        }
        
        return false;
    }
    
    inline bool compare(HandleValue value, bool b) // INFAILIBLE, POSSIBLY SLOW
    {
        return ToBoolean(value) == b;
    }
    
    inline bool compare(HandleValue value, const char *s) // INFAILIBLE, POSSIBLY SLOW
    {
        RootedString s1(cx, ToString(cx, value));
        RootedString s2(cx, toJSString(s));
        
        int32_t result;
        
        if (JS_CompareStrings(cx, s1, s2, &result))
        {
            return (result == 0);
        }
        
        return false;
    }
    
    // ---
    
    /*
     * TODO: USE toObject TEMPLATE FOR toSource(), stringify(), isFunction(), isHealthy(), ETC.
     */
    
    template <typename T>
    JSObject* toObject(T); // CAN RETURN A NULL POINTER

    bool isFunction(const Value &value);
    bool isArray(JSObject *object);

    // ---
    
    std::string toSource(JSObject *object);
    std::string toSource(HandleValue value);
    
    std::string stringify(JSObject *object, int indent = 2);
    std::string stringify(MutableHandleValue value, int indent = 2);
    
    // ---

    /*
     * TEMPLATE SPECIALIZATION IS NECESSARY, OTHERWISE toValue(bool) IS ALWAYS "PICKED"
     *
     * IN ADDITION: toValue(const std::string &s) IS NEVER "PICKED", HENCE toValue(const char *s)
     */
    
    template<typename T>
    inline Value toValue(T);
    
    template <>
    inline Value toValue(JSObject *object)
    {
        return ObjectOrNullValue(object);
    }
    
    template <>
    inline Value toValue(float f)
    {
        return NumberValue(f);
    }
    
    template <>
    inline Value toValue(double d)
    {
        return NumberValue(d);
    }
    
    template <>
    inline Value toValue(int32_t i)
    {
        return NumberValue(i);
    }

    template <>
    inline Value toValue(uint32_t ui)
    {
        return NumberValue(ui);
    }
    
    template <>
    inline Value toValue(bool b)
    {
        return BooleanValue(b);
    }
    
    template <>
    inline Value toValue(const char *s)
    {
        return StringValue(toJSString(s));
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
        operator JS::Handle<JSObject*> () const;
        operator JS::Handle<WrappedObject> () const;
        
        operator JS::MutableHandle<JSObject*> ();
        operator JS::MutableHandle<WrappedObject> ();
        
        operator JS::Handle<JS::Value> () const;
        operator JS::MutableHandle<JS::Value> ();
        
        explicit operator const bool () const;
    };
}

namespace JSP
{
    using namespace jsp;
    
    // ---
    
    void dumpString(JSString *str);
    void dumpAtom(JSAtom *atom);
    void dumpObject(JSObject *obj);

    std::string write(const Value &value);
    std::string write(jsid id);

    char writeMarkDescriptor(void *thing);

    template<typename T>
    char writeGCDescriptor(T *thing);
    
    char writeGCDescriptor(const Value &value);

    std::string writeTraceThingInfo(JSObject *object, bool details = true);
    std::string writeTraceThingInfo(const Value &value, bool details = true);

    std::string writeDetailed(JSObject *object);
    std::string writeDetailed(const Value &value);
    
    // ---
    
    /*
     * USING JSP::isPoisoned(T*) STANDALONE IS ACTUALLY NOT ENOUGH:
     *
     * 1) THE thing COULD BE NULL
     *
     * 2) THE "POISON PATTERN" CAN STILL BE DETECTED IN NEWLY ALLOCATED "CELLS"
     *    - HENCE THE COMPLICATED STUFF TAKING PLACE IN isHealthy()
     *    - ALL THE COMPLEX SITUATIONS SEEM TO BE HANDLED
     *      - MOST COMPLICTED CASE: JSString INSIDE JS::Value
     *      - TODO: FOLLOW-UP
     *
     * HENCE THE NEED FOR JSP::isHeathy(T*)
     */
    
    template<typename T>
    bool isHealthy(T *thing);
    
    bool isHealthy(const Value &value);
    
    bool isInsideNursery(void *thing);

    /*
     * MOSTLY FOR INTERNAL USAGE (HERE AGAIN: RESULTS ARE NOT AS-OBVIOUS-AS STATED IN THE NAMING)
     */
    template<typename T>
    bool isAboutToBeFinalized(T **thing);
    
#if defined(DEBUG) && defined(JS_DEBUG) && defined(JSP_USE_PRIVATE_APIS)
    
    /*
     * BORROWED FROM: https://github.com/mozilla/gecko-dev/blob/esr31/js/src/gc/Marking.cpp#L101-130
     *
     * THE ONLY VALID WAY OF CHECKING FOR POISONING, AT LEAST UNDER "GENERATIONAL GC"
     * IT SHOULD PROBABLY BE INTEGRATED IN THE OFFICIAL SPIDERMONKEY API SOON
     *
     *
     * BACKGROUND:
     *
     * THE JS::IsPoisonedPtr() IMPLEMENTATION (DEFINED IN /js/public/Utility.h)
     * IS NOT WORKING AS INTENDED, AT LEAST UNDER "GENERATIONAL GC":
     *
     * IT IS EITHER RETURNING false (UNLESS THE "JSGC_ROOT_ANALYSIS" MACRO IS DEFINED)
     *
     * OTHERWISE: IT RELIES ON THE "JS_FREE_PATTERN" MACRO WHICH IS NOT DECLARED (ANYMORE)
     * - IT WAS DECLARED AS 0xDA UNTIL ~SPIDERMONKEY 24
     * - IT WAS PROBABLY REMOVED (STARTING FROM ~SPIDERMONKEY 28) IN PREVISION OF THE "REVISED POISONING" POLICY
     */
    
    template<typename T>
    inline bool isPoisoned(T *thing)
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

    inline bool isPoisoned(const Value &value)
    {
        if (value.isString())
        {
            return isPoisoned(value.toString());
        }
        
        if (value.isObject())
        {
            return isPoisoned(&value.toObject());
        }
        
        return false;
    }
    
    /*
     * XXX: NOT CLEAR WHY TEMPLATE-MATCHING DOESN'T WORK FOR JSFunction (HENCE THE NEED FOR THE FOLLOWING 2...)
     */

    inline char writeGCDescriptor(JSFunction *function)
    {
        return writeGCDescriptor(static_cast<JSObject*>(function));
    }

    inline bool isHealthy(JSFunction *function)
    {
        return isHealthy(static_cast<JSObject*>(function));
    }

#else
    
    template<typename T>
    inline bool isPoisoned(T *thing)
    {
        return false;
    }
    
    inline bool isPoisoned(const Value &value)
    {
        return false;
    }
    
#endif

    // ---
    
    void forceGC();
    void setGCZeal(uint8_t zeal, uint32_t frequency = 100);
    
    // ---
    
    /*
     * TODO: PROBABLY OUT-OF-SCOPE (CONSIDER MOVING THIS TO SOME MORE "SPECIALIZED" CLASS...)
     */

    uint32_t toHTMLColor(const std::string &c, uint32_t defaultValue = 0x000000);
    uint32_t toHTMLColor(HandleValue &&value, uint32_t defaultValue = 0x000000); // INFAILIBLE
};
