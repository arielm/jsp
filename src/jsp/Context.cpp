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

#include "chronotext/Log.h"
#include "chronotext/incubator/utils/FileCapture.h"

using namespace std;
using namespace chr;

#pragma mark ---------------------------------------- jsp NAMESPACE ----------------------------------------

namespace jsp
{
    namespace intern
    {
        struct Stringifier;

        map<void*, TracerCallbackFnType> tracerCallbacks;
        map<void*, GCCallbackFnType> gcCallbacks;
        
        void tracerCallback(JSTracer *trc, void *data);
        void gcCallback(JSRuntime *rt, JSGCStatus status, void *data);
        
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
            JS_AddExtraGCRootsTracer(rt, intern::tracerCallback, nullptr);
            JS_SetGCCallback(rt, intern::gcCallback, nullptr);
            
            // ---
            
            intern::postInitialized = true;
        }
        
        return intern::postInitialized;
    }
    
    void preShutdown()
    {
        if (intern::postInitialized && rt && cx)
        {
            JS_RemoveExtraGCRootsTracer(rt, intern::tracerCallback, nullptr);
            intern::tracerCallbacks.clear();
            
            JS_SetGCCallback(rt, nullptr, nullptr);
            intern::gcCallbacks.clear();
            
            // ---
            
            intern::postInitialized = false;
        }
    }
    
#pragma mark ---------------------------------------- CENTRALIZED EXTRA-ROOT-TRACING ----------------------------------------
    
    void addTracerCallback(void *instance, const TracerCallbackFnType &fn)
    {
        JS_ASSERT(intern::postInitialized);
        intern::tracerCallbacks.emplace(instance, fn);
    }
    
    void removeTracerCallback(void *instance)
    {
        JS_ASSERT(intern::postInitialized);
        intern::tracerCallbacks.erase(instance);
    }
    
    void intern::tracerCallback(JSTracer *trc, void *data)
    {
        for (auto &element : tracerCallbacks)
        {
            element.second(trc);
        }
    }
    
#pragma mark ---------------------------------------- CENTRALIZED GC-CALLBACKS ----------------------------------------

    void addGCCallback(void *instance, const GCCallbackFnType &fn)
    {
        JS_ASSERT(intern::postInitialized);
        intern::gcCallbacks.emplace(instance, fn);
    }
    
    void removeGCCallback(void *instance)
    {
        JS_ASSERT(intern::postInitialized);
        intern::gcCallbacks.erase(instance);
    }
    
    void intern::gcCallback(JSRuntime *rt, JSGCStatus status, void *data)
    {
        for (auto &element : gcCallbacks)
        {
            element.second(rt, status);
        }
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
    JSObject* toObject(Rooted<WrappedObject> *rooted)
    {
        return rooted ? rooted->get().unsafeGet() : nullptr;
    }
    
    template <>
    JSObject* toObject(Heap<WrappedObject> *heap)
    {
        return heap ? const_cast<JSObject*>(heap->get().address()) : nullptr;
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
    
    template <>
    JSObject* toObject(Rooted<WrappedValue> *rooted)
    {
        if (rooted && rooted->get().isObject())
        {
            return rooted->get().toObjectOrNull();
        }
        
        return nullptr;
    }
    
    template <>
    JSObject* toObject(Heap<WrappedValue> *heap)
    {
        if (heap && heap->get().isObject())
        {
            return heap->get().toObjectOrNull();
        }
        
        return nullptr;
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
    
    const string toSource(JSObject *object)
    {
        RootedValue value(cx, ObjectOrNullValue(object));
        return toSource(value);
    }
    
    const string toSource(HandleValue value)
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

using namespace jsp;

char JSP::traceBuffer[TRACE_BUFFER_SIZE];
map<string, uint32_t> JSP::htmlColors;

#pragma mark ---------------------------------------- DEBUG / DUMP ----------------------------------------

#if defined(JS_DEBUG)

void JSP::dumpString(JSString *str)
{
    js_DumpString(str);
}

void JSP::dumpAtom(JSAtom *atom)
{
    js_DumpAtom(atom);
}

void JSP::dumpObject(JSObject *obj)
{
    js_DumpObject(obj);
}

#else

void JSP::dumpString(JSString *str) {}
void JSP::dumpAtom(JSAtom *atom) {}
void JSP::dumpObject(JSObject *obj) {}

#endif

#if defined(DEBUG) && defined(JS_DEBUG) && defined(JSP_USE_PRIVATE_APIS)

string JSP::write(const Value &value)
{
    FileCapture capture(stderr);
    js_DumpValue(value);
    return capture.flushAsString(true);
}

string JSP::write(jsid id)
{
    FileCapture capture(stderr);
    js_DumpId(id);
    return capture.flushAsString(true);
}

/*
 * REFERENCES:
 * https://github.com/mozilla/gecko-dev/blob/esr31/js/src/jsfriendapi.cpp#L737-745
 * https://github.com/mozilla/gecko-dev/blob/esr31/js/src/gc/Marking.cpp#L193-200
 *
 * IN PRACTICE:
 * MarkDescriptor(void *thing) IN jsfriendapi.cpp SEEMS TO RETURN ONLY 'B' OR 'W'
 *
 * SEE ALSO: isHealthy(T*)
 */
char JSP::writeMarkDescriptor(void *thing)
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

char JSP::writeGCDescriptor(const Value &value)
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

template <>
string JSP::writeTraceThingInfo(JSObject *object, bool details)
{
    if (isHealthy(object))
    {
        JS_GetTraceThingInfo(traceBuffer, TRACE_BUFFER_SIZE, nullptr, object, JSTRACE_OBJECT, details);
        return traceBuffer;
    }
    
    return "";
}

template <>
string JSP::writeTraceThingInfo(JSFunction *function, bool details)
{
    if (isHealthy(function))
    {
        JS_GetTraceThingInfo(traceBuffer, TRACE_BUFFER_SIZE, nullptr, function, JSTRACE_OBJECT, details);
        return traceBuffer;
    }
    
    return "";
}

template <>
string JSP::writeTraceThingInfo(JSString *s, bool details)
{
    if (isHealthy(s))
    {
        JS_GetTraceThingInfo(traceBuffer, TRACE_BUFFER_SIZE, nullptr, s, JSTRACE_STRING, details);
        return traceBuffer;
    }
    
    return "";
}

string JSP::writeTraceThingInfo(const Value &value, bool details)
{
    if (isHealthy(value))
    {
        if (value.isMarkable())
        {
            JS_GetTraceThingInfo(traceBuffer, TRACE_BUFFER_SIZE, nullptr, value.toGCThing(), value.gcKind(), details);
            return traceBuffer;
        }
    }
    
    return "";
}

string JSP::writeDetailed(const Value &value)
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

string JSP::write(const Value &value) { return ""; }
string JSP::write(jsid id) { return ""; }

char JSP::writeMarkDescriptor(void *thing) { return '?'; }

char JSP::writeGCDescriptor(const Value &value) { return '?'; }
string JSP::writeDetailed(const Value &value) { return ""; }

#endif

#pragma mark ---------------------------------------- DEBUG / INFO ----------------------------------------

#if defined(DEBUG) && defined(JS_DEBUG) && defined(JSP_USE_PRIVATE_APIS)

bool JSP::isInsideNursery(void *thing)
{
    if (thing)
    {
        auto cell = static_cast<js::gc::Cell*>(thing);
        return !cell->isTenured();
    }
    
    return false;
}

template <>
bool JSP::isAboutToBeFinalized(JSObject **thing)
{
    return js::gc::IsObjectAboutToBeFinalized(thing);
}

template <>
bool JSP::isAboutToBeFinalized(JSFunction **thing)
{
    return js::gc::IsObjectAboutToBeFinalized(thing);
}

template <>
bool JSP::isAboutToBeFinalized(JSString **thing)
{
    return js::gc::IsStringAboutToBeFinalized(thing);
}

bool JSP::isPoisoned(const Value &value)
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

bool JSP::isHealthy(const Value &value)
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

#else

bool JSP::isInsideNursery(void *thing)
{
    return false;
}

bool JSP::isPoisoned(const Value &value)
{
    return false;
}

bool JSP::isHealthy(const Value &value)
{
    return false;
}

#endif

#pragma mark ---------------------------------------- GC AND ROOTING ----------------------------------------

void JSP::forceGC()
{
    LOGD << "JSP::forceGC() | BEGIN" << endl; // LOG: VERBOSE
    
    JS_SetGCParameter(rt, JSGC_MODE, JSGC_MODE_GLOBAL);
    JS_GC(rt);
    
    LOGD << "JSP::forceGC() | END" << endl; // LOG: VERBOSE
}

void JSP::setGCZeal(uint8_t zeal, uint32_t frequency)
{
#if defined(JS_GC_ZEAL)
    JS_SetGCZeal(cx, zeal, frequency);
#endif
}

#pragma mark ---------------------------------------- HTML COLORS ----------------------------------------

bool JSP::defineHTMLColors()
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

bool JSP::lookupHTMLColor(const string &colorName, uint32_t *result)
{
    if (defineHTMLColors())
    {
        auto color = htmlColors.find(colorName);
        
        if (color != htmlColors.end())
        {
            *result = color->second;
            return true;
        }
    }
    
    return false;
}

uint32_t JSP::toHTMLColor(const string &colorName, uint32_t defaultValue)
{
    uint32_t result;
    return lookupHTMLColor(colorName, &result) ? result : defaultValue;
}

uint32_t JSP::toHTMLColor(HandleValue value, uint32_t defaultValue)
{
    if (value.isString())
    {
        string tmp = toString(value);
        
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
        return convertSafely(value, defaultValue);
    }
    
    return defaultValue;
}
