/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "jsp/Context.h"
#include "jsp/WrappedObject.h"
#include "jsp/WrappedValue.h"
#include "jsp/Barker.h"

#include "chronotext/incubator/utils/FileCapture.h"

#include <map>
#include <sstream>

using namespace std;
using namespace chr;

#pragma mark ---------------------------------------- jsp NAMESPACE ----------------------------------------

namespace jsp
{
    namespace intern
    {
        map<void*, function<void(JSTracer*)>> tracers;
        void traceCallback(JSTracer *trc, void *data);
        
        struct Stringifier;
        
        // ---
        
        bool postInitialized = false;
    }
    
    // ---
    
    JSRuntime *rt = nullptr;
    JSContext *cx = nullptr;
    JS::Heap<JSObject*> global;
    
    JSRuntime* runtime()
    {
        return rt;
    }
    
    JSContext* context()
    {
        return cx;
    }
    
    HandleObject globalHandle()
    {
        return HandleObject::fromMarkedLocation(global.address());
    }
    
    // ---
    
    bool postInit()
    {
        if (!intern::postInitialized && rt && cx)
        {
            JS_AddExtraGCRootsTracer(rt, intern::traceCallback, nullptr);
            
            Barker::init();
            
            // ---
            
            intern::postInitialized = true;
        }
        
        return intern::postInitialized;
    }
    
    void preShutdown()
    {
        if (intern::postInitialized && rt && cx)
        {
            Barker::shutdown();
            
            JS_RemoveExtraGCRootsTracer(rt, intern::traceCallback, nullptr);
            intern::tracers.clear();
            
            // ---
            
            intern::postInitialized = false;
        }
    }
    
#pragma mark ---------------------------------------- CENTRALIZED EXTRA-ROOT-TRACING ----------------------------------------
    
    void addTracer(void *tracer, function<void(JSTracer*)> fn)
    {
        assert(intern::postInitialized);
        intern::tracers[tracer] = fn;
    }
    
    void removeTracer(void *tracer)
    {
        assert(intern::postInitialized);
        intern::tracers.erase(tracer);
    }
    
    void intern::traceCallback(JSTracer *trc, void *data)
    {
        for (auto &element : tracers)
        {
            element.second(trc);
        }
    }
    
#pragma mark ---------------------------------------- VALUE TO TYPE (MAYBE) ----------------------------------------
    
    bool toObjectMaybe(HandleValue &&value, JSObject **result)
    {
        if (value.isObject())
        {
            *result = value.toObjectOrNull();
            return true;
        }
        
        return false;
    }
    
    bool toFloat32Maybe(HandleValue &&value, float *result)
    {
        double d;
        
        if (ToNumber(cx, forward<HandleValue>(value), &d))
        {
            *result = float(d);
            return true;
        }
        
        return false;
    }
    
    bool toFloat64Maybe(HandleValue &&value, double *result)
    {
        return ToNumber(cx, forward<HandleValue>(value), result);
    }
    
    bool toInt32Maybe(HandleValue &&value, int32_t *result)
    {
        return ToInt32(cx, forward<HandleValue>(value), result);
    }
    
    bool toUInt32Maybe(HandleValue &&value, uint32_t *result)
    {
        return ToUint32(cx, forward<HandleValue>(value), result);
    }
    
#pragma mark ---------------------------------------- VALUE TO TYPE (SAFELY) ----------------------------------------
    
    JSObject* toObjectSafely(HandleValue &&value, JSObject *defaultValue)
    {
        JSObject *result;
        return toObjectMaybe(forward<HandleValue>(value), &result) ? result : defaultValue;
    }
    
    float toFloat32Safely(HandleValue &&value, float defaultValue)
    {
        float result;
        return toFloat32Maybe(forward<HandleValue>(value), &result) ? result : defaultValue;
    }
    
    double toFloat64Safely(HandleValue &&value, double defaultValue)
    {
        double result;
        return toFloat64Maybe(forward<HandleValue>(value), &result) ? result : defaultValue;
    }
    
    int32_t toInt32Safely(HandleValue &&value, int32_t defaultValue)
    {
        int32_t result;
        return toInt32Maybe(forward<HandleValue>(value), &result) ? result : defaultValue;
    }
    
    uint32_t toUInt32Safely(HandleValue &&value, uint32_t defaultValue)
    {
        uint32_t result;
        return toUInt32Maybe(forward<HandleValue>(value), &result) ? result : defaultValue;
    }
    
#pragma mark ---------------------------------------- DOWNCASTING TO JS OBJECT ----------------------------------------
    
    template <>
    JSObject* toObject(JSObject *object)
    {
        return object;
    }
    
    template <>
    JSObject* toObject(const JSObject *object)
    {
        return const_cast<JSObject*>(object);
    }
    
    template <>
    JSObject* toObject(JSObject &object)
    {
        return &object;
    }
    
    template <>
    JSObject* toObject(const JSObject &object)
    {
        return const_cast<JSObject*>(&object);
    }
    
    template <>
    JSObject* toObject(HandleObject handle)
    {
        return handle.get();
    }
    
    template <>
    JSObject* toObject(MutableHandleObject handle)
    {
        return handle.get();
    }
    
    template <>
    JSObject* toObject(WrappedObject *wrapped)
    {
        return wrapped ? wrapped->unsafeGet() : nullptr;
    }
    
    template <>
    JSObject* toObject(const WrappedObject *wrapped)
    {
        return wrapped ? const_cast<JSObject*>(wrapped->address()) : nullptr;
    }
    
    template <>
    JSObject* toObject(WrappedObject &wrapped)
    {
        return wrapped.unsafeGet();
    }
    
    template <>
    JSObject* toObject(const WrappedObject &wrapped)
    {
        return const_cast<JSObject*>(wrapped.address());
    }
    
    template <>
    JSObject* toObject(Handle<WrappedObject> handle)
    {
        return handle.get();
    }
    
    template <>
    JSObject* toObject(MutableHandle<WrappedObject> handle)
    {
        return handle.get();
    }
    
    template <>
    JSObject* toObject(Value &value)
    {
        return value.isObject() ? value.toObjectOrNull() : nullptr;
    }
    
    template <>
    JSObject* toObject(const Value &value)
    {
        return value.isObject() ? value.toObjectOrNull() : nullptr;
    }
    
    template <>
    JSObject* toObject(WrappedValue &wrapped)
    {
        return wrapped.isObject() ? wrapped.toObjectOrNull() : nullptr;
    }
    
    template <>
    JSObject* toObject(const WrappedValue &wrapped)
    {
        return wrapped.isObject() ? wrapped.toObjectOrNull() : nullptr;
    }
    
    template <>
    JSObject* toObject(Handle<WrappedValue> handle)
    {
        return handle.get().isObject() ? handle.get().toObjectOrNull() : nullptr;
    }
    
    template <>
    JSObject* toObject(MutableHandle<WrappedValue> handle)
    {
        return handle.get().isObject() ? handle.get().toObjectOrNull() : nullptr;
    }
    
#pragma mark ---------------------------------------- MISC ----------------------------------------
    
    bool isFunction(const Value &value)
    {
        return value.isObject() && JS_ObjectIsFunction(cx, &value.toObject());
    }
    
    bool isArray(JSObject *object)
    {
        RootedObject tmp(cx, object);
        return JS_IsArrayObject(cx, tmp);
    }
    
#pragma mark ---------------------------------------- TO-SOURCE ----------------------------------------
    
    string toSource(JSObject *object)
    {
        RootedValue value(cx, ObjectOrNullValue(object));
        return toSource(value);
    }
    
    string toSource(HandleValue value)
    {
        RootedString source(cx);
        source.set(JS_ValueToSource(cx, value));
        
        JS_ReportPendingException(cx); // E.G. FOR REPORTING SOME "OUT OF MEMORY" ERROR
        JS_ClearPendingException(cx);
        
        return toString(source);
    }
    
#pragma mark ---------------------------------------- STRINGIFICATION ----------------------------------------
    
    /*
     * TODO:
     *
     * 1) STRINGIFICATION: HANDLE CUSTOM-REPLACER
     * 2) HANDLE JSON PARSING
     */
    
    struct intern::Stringifier
    {
        string buffer;
        
        static bool callback(const jschar *buf, uint32_t len, void *data)
        {
            auto self = reinterpret_cast<Stringifier*>(data);
            UnicodeString(reinterpret_cast<const UChar*>(buf), len).toUTF8String(self->buffer);
            return true;
        }
    };
    
    string stringify(JSObject *object, int indent)
    {
        RootedValue value(cx, ObjectOrNullValue(object));
        return stringify(&value, indent);
    }
    
    string stringify(MutableHandleValue value, int indent)
    {
        intern::Stringifier stringifier;
        
        RootedValue indentValue(cx, indent ? Int32Value(indent) : UndefinedHandleValue);
        
        if (!JS_Stringify(cx, value, NullPtr(), indentValue, &intern::Stringifier::callback, &stringifier))
        {
            JS_ReportPendingException(cx); // E.G. FOR REPORTING SOME "OUT OF MEMORY" ERROR
            JS_ClearPendingException(cx);
            
            return ""; // TODO: MIMICK JSON.stringify() BEHAVIOR (I.E. SHALL WE RETURN THE ACCUMULATED BUFFER UPON ERROR?)
        }
        
        return stringifier.buffer;
    }
}

#pragma mark ---------------------------------------- JSP NAMESPACE ----------------------------------------

namespace JSP
{
    namespace intern
    {
        constexpr size_t TRACE_BUFFER_SIZE = 256;
        char traceBuffer[TRACE_BUFFER_SIZE];

        // ---
        
        map<string, uint32_t> htmlColors;
        
        bool defineHTMLColors();
        bool lookupHTMLColor(const string &c, uint32_t *result);
    }
    
#pragma mark ---------------------------------------- DEBUG / DUMP ----------------------------------------
    
#if defined(JS_DEBUG)
    
    void dumpString(JSString *str)
    {
        js_DumpString(str);
    }
    
    void dumpAtom(JSAtom *atom)
    {
        js_DumpAtom(atom);
    }
    
    void dumpObject(JSObject *obj)
    {
        js_DumpObject(obj);
    }
    
#else
    
    void dumpString(JSString *str) {}
    void dumpAtom(JSAtom *atom) {}
    void dumpObject(JSObject *obj) {}
    
#endif
    
#if defined(DEBUG) && defined(JS_DEBUG) && defined(JSP_USE_PRIVATE_APIS)
    
    string write(const Value &value)
    {
        FileCapture capture(stderr);
        js_DumpValue(value);
        return capture.flushAsString(true);
    }
    
    string write(jsid id)
    {
        FileCapture capture(stderr);
        js_DumpId(id);
        return capture.flushAsString(true);
    }
    
    /*
     * REFERENCES:
     *
     * https://github.com/mozilla/gecko-dev/blob/esr31/js/src/jsfriendapi.cpp#L737-745
     * https://github.com/mozilla/gecko-dev/blob/esr31/js/src/gc/Marking.cpp#L193-200
     *
     * IN PRACTICE:
     * MarkDescriptor(void *thing) IN jsfriendapi.cpp SEEMS TO RETURN ONLY 'B' OR 'W'
     *
     * SEE ALSO: isHealthy(JSString *thing)
     */
    
    char writeMarkDescriptor(void *thing)
    {
        if (thing)
        {
            auto cell = static_cast<js::gc::Cell*>(thing);
            
            if (cell->isTenured() && cell->arenaHeader()->allocated())
            {
                if (!InFreeList(cell->arenaHeader(), thing))
                {
                    if (!cell->isMarked(js::gc::GRAY))
                    {
                        return cell->isMarked(js::gc::BLACK) ? 'B' : 'W';
                    }
                }
            }
        }
        
        return '?';
    }
    
    template <typename T>
    char writeGCDescriptor(T *thing)
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
            
            T *forwarded = thing;
            
            if (isAboutToBeFinalized(&forwarded))
            {
                return 'f';
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
    
    char writeGCDescriptor(const Value &value)
    {
        char gcDescriptor = '?';
        
        if (value.isMarkable())
        {
            if (value.isObject())
            {
                gcDescriptor = writeGCDescriptor(value.toObjectOrNull());
            }
            else if (value.isString())
            {
                gcDescriptor = writeGCDescriptor(value.toString());
            }
        }
        
        return gcDescriptor;
    }
    
    string writeTraceThingInfo(JSObject *object, bool details)
    {
        if (isHealthy(object))
        {
            JS_GetTraceThingInfo(intern::traceBuffer, intern::TRACE_BUFFER_SIZE, nullptr, object, JSTRACE_OBJECT, details);
            return intern::traceBuffer;
        }
        
        return "";
    }
    
    string writeTraceThingInfo(const Value &value, bool details)
    {
        if (isHealthy(value))
        {
            if (value.isMarkable())
            {
                JS_GetTraceThingInfo(intern::traceBuffer, intern::TRACE_BUFFER_SIZE, nullptr, value.toGCThing(), value.gcKind(), details);
                return intern::traceBuffer;
            }
        }
        
        return "";
    }
    
    string writeDetailed(JSObject *object)
    {
        if (object)
        {
            stringstream ss;
            ss << object;
            
            const auto traceThingInfo = writeTraceThingInfo(object, true);
            
            if (!traceThingInfo.empty())
            {
                ss << " {" << traceThingInfo << "}";
            }
            
            auto gcDescriptor = writeGCDescriptor(object);
            
            if (gcDescriptor != '?')
            {
                ss << " [" << gcDescriptor << "]";
            }
            
            return ss.str();
        }
        
        return "";
    }
    
    string writeDetailed(const Value &value)
    {
        if (value.isMarkable())
        {
            stringstream ss;
            ss << value.toGCThing();
            
            const auto traceThingInfo = writeTraceThingInfo(value, true);
            
            if (!traceThingInfo.empty())
            {
                ss << " {" << traceThingInfo << "}";
            }
            
            auto gcDescriptor = writeGCDescriptor(value);
            
            if (gcDescriptor != '?')
            {
                ss << " [" << gcDescriptor << "]";
            }
            
            return ss.str();
        }
        
        return write(value);
    }
    
#else
    
    string write(const Value &value) { return ""; }
    string write(jsid id) { return ""; }
    
    char writeMarkDescriptor(void *thing) { return '?'; }
    
    template <>
    char writeGCDescriptor(JSObject *thing)
    {
        return '?';
    }
    
    template <>
    char writeGCDescriptor(JSString *thing)
    {
        return '?';
    }
    
    char writeGCDescriptor(const Value &value) { return '?'; }
    
    string writeDetailed(JSObject *object) { return ""; }
    string writeDetailed(const Value &value) { return ""; }
    
#endif

#pragma mark ---------------------------------------- DEBUG / INFO ----------------------------------------
    
#if defined(DEBUG) && defined(JS_DEBUG) && defined(JSP_USE_PRIVATE_APIS)
    
    template <>
    bool isAboutToBeFinalized(JSObject **thing)
    {
        return js::gc::IsObjectAboutToBeFinalized(thing);
    }
    
    template <>
    bool isAboutToBeFinalized(JSString **thing)
    {
        return js::gc::IsStringAboutToBeFinalized(thing);
    }
    
    /*
     * REFERENCES:
     *
     * https://github.com/mozilla/gecko-dev/blob/esr31/js/src/jsfriendapi.cpp#L737-745
     * https://github.com/mozilla/gecko-dev/blob/esr31/js/src/gc/Marking.cpp#L193-200
     *
     * IN PRACTICE:
     * thing->zone()->runtimeFromMainThread()->isHeapBusy() SEEMS TO NEVER RETURN TRUE
     *
     * SEE ALSO: writeMarkDescriptor(void *thing)
     */
    
    template<typename T>
    bool isHealthy(T *thing)
    {
        if (thing)
        {
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
    
    bool isHealthy(const Value &value)
    {
        if (value.isMarkable())
        {
            void *thing = value.toGCThing();
            
            if (value.isString())
            {
                return isHealthy((JSString*)thing);
            }
            
            if (value.isObject())
            {
                return isHealthy((JSObject*)thing);
            }
        }
        
        return false;
    }
    
    bool isInsideNursery(void *thing)
    {
        if (thing)
        {
            auto cell = static_cast<js::gc::Cell*>(thing);
            return !cell->isTenured();
        }
        
        return false;
    }
    
#else
    
    template <>
    bool isAboutToBeFinalized(JSObject **thing)
    {
        return false;
    }
    
    template <>
    bool isAboutToBeFinalized(JSString **thing)
    {
        return false;
    }
    
    template <>
    bool isHealthy(JSObject *thing)
    {
        return false;
    }
    
    template <>
    bool isHealthy(JSString *thing)
    {
        return false;
    }
    
    bool isHealthy(const Value &value)
    {
        return false;
    }
    
    bool isInsideNursery(void *thing)
    {
        return false;
    }
    
#endif
    
#pragma mark ---------------------------------------- GC AND ROOTING ----------------------------------------
    
    void forceGC()
    {
        JS_SetGCParameter(rt, JSGC_MODE, JSGC_MODE_GLOBAL);
        JS_GC(rt);
    }
    
    void setGCZeal(uint8_t zeal, uint32_t frequency)
    {
#if defined(JS_GC_ZEAL)
        JS_SetGCZeal(cx, zeal, frequency);
#endif
    }
    
#pragma mark ---------------------------------------- HTML COLORS ----------------------------------------

    bool intern::defineHTMLColors()
    {
        /*
         * XXX: THERE SHOULD BE MORE COLORS
         */
        
        if (htmlColors.empty())
        {
            htmlColors["black"] = 0x000000;
            htmlColors["blue"] = 0x0000ff;
            htmlColors["gray"] = 0x808080;
            htmlColors["green"] = 0x00ff00;
            htmlColors["orange"] = 0xffa500;
            htmlColors["purple"] = 0x800080;
            htmlColors["red"] = 0xff0000;
            htmlColors["white"] = 0xffffff;
            htmlColors["yellow"] = 0xffff00;
        }
        
        return true;
    }
    
    bool intern::lookupHTMLColor(const string &c, uint32_t *result)
    {
        if (defineHTMLColors())
        {
            auto color = htmlColors.find(c);
            
            if (color != htmlColors.end())
            {
                *result = color->second;
                return true;
            }
        }
        
        return false;
    }
    
    uint32_t toHTMLColor(const string &c, uint32_t defaultValue)
    {
        uint32_t result;
        return intern::lookupHTMLColor(c, &result) ? result : defaultValue;
    }
    
    uint32_t toHTMLColor(HandleValue &&value, uint32_t defaultValue)
    {
        if (value.isString())
        {
            string tmp = toString(forward<HandleValue>(value));
            
            if (!tmp.empty())
            {
                if ((tmp[0] == '#') && (tmp.size() == 7))
                {
                    uint32_t result;
                    stringstream ss;
                    ss << hex << tmp.substr(1, string::npos);
                    ss >> result;
                    
                    return result;
                }
                
                return toHTMLColor(tmp, defaultValue);
            }
        }
        else
        {
            return toUInt32Safely(forward<HandleValue>(value), defaultValue);
        }
        
        return defaultValue;
    }
}
