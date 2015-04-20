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
    
#pragma mark ---------------------------------------- STRING HELPERS ----------------------------------------

    toChars::toChars(const jschar *chars, size_t len)
    {
        if (chars && len)
        {
            data = TwoByteCharsToNewUTF8CharsZ(cx, TwoByteChars(chars, len)).c_str();
        }
    }
    
    toChars::toChars(JSString *str)
    {
        if (str)
        {
            JSLinearString *linear = str->ensureLinear(cx); // ASSERTION: NO NEED TO PROTECT BECAUSE THE FOLLOWING CAN'T TRIGGER GC
            
            if (linear)
            {
                data = TwoByteCharsToNewUTF8CharsZ(cx, TwoByteChars(linear->chars(), linear->length())).c_str();
            }
        }
    }

    toChars::toChars(HandleValue value)
    :
    toChars(ToString(cx, value))
    {}
    
    toChars::~toChars()
    {
        js_free(data);
    }
    
    // ---
    
    string toString(const jschar *chars, size_t len)
    {
        string result;
        
        if (chars && len)
        {
            result = TwoByteCharsToNewUTF8CharsZ(cx, TwoByteChars(chars, len)).c_str(); // A BETTER ALTERNATIVE WOULD BE TO DECODE "IN PLACE" (NOT FEASIBLE WITH THE CURRENT API)
        }
        
        return result; // RVO-COMPLIANT
    }
    
    string toString(JSString *str)
    {
        string result;

        if (str)
        {
            JSLinearString *linear = str->ensureLinear(cx); // ASSERTION: NO NEED TO PROTECT BECAUSE THE FOLLOWING CAN'T TRIGGER GC
            
            if (linear)
            {
                result = TwoByteCharsToNewUTF8CharsZ(cx, TwoByteChars(linear->chars(), linear->length())).c_str(); // A BETTER ALTERNATIVE WOULD BE TO DECODE "IN PLACE" (NOT FEASIBLE WITH THE CURRENT API)
            }
        }
        
        return result; // RVO-COMPLIANT
    }

    // ---
    
    /*
     * ONE CAN SAFELY ASSUME THAT RESULT IS "FLAT" (I.E. LINEAR AND NULL-TERMINATED)
     */
    JSFlatString* toJSString(const char *c)
    {
        if (c)
        {
            size_t len;
            jschar *chars = LossyUTF8CharsToNewTwoByteCharsZ(cx, UTF8Chars(c, strlen(c)), &len).get();
            
            if (chars)
            {
                JSFlatString *result = js_NewString<js::CanGC>(cx, chars, len); // USED BY JS_NewUCString() BEHIND THE SCENES
                
                if (result)
                {
                    return result; // (TRANSFERRED) MEMORY IS NOW UNDER THE RULES OF GC...
                }
                
                js_free(chars); // OTHERWISE: WE'RE IN CHARGE
            }
        }
        
        return cx->emptyString(); // ATOMS ARE "FLAT"
    }
    
    bool stringEquals(JSString *str1, const char *c2)
    {
        if (str1 && c2)
        {
            JSLinearString *linear1 = str1->ensureLinear(cx); // ASSERTION: NO NEED TO PROTECT BECAUSE THE FOLLOWING CAN'T TRIGGER GC
            
            if (linear1)
            {
                size_t len2;
                jschar *chars2 = LossyUTF8CharsToNewTwoByteCharsZ(cx, UTF8Chars(c2, strlen(c2)), &len2).get();
                
                if (chars2)
                {
                    size_t len1 = linear1->length();
                    bool result = (len1 == len2) && (js::CompareChars(linear1->chars(), len1, chars2, len2) == 0);
                    
                    js_free(chars2);
                    return result;
                }
            }
        }
        
        return false;
    }
    
    bool stringEqualsASCII(JSString *str1, const char *c2)
    {
        if (str1 && c2)
        {
            size_t len1 = str1->length();
            size_t len2 = strlen(c2);

            if (len1 == len2)
            {
                if (!len1)
                {
                    return true; // SPECIAL CASE: BOTH STRINGS ARE EMPTY
                }
                
#if defined(DEBUG)
                for (auto i = 0; i != len2; ++i)
                {
                    JS_ASSERT(unsigned(c2[i]) <= 127);
                }
#endif
                
                JSLinearString *linear1 = str1->ensureLinear(cx);
                
                if (linear1)
                {
                    const jschar *chars1 = linear1->chars();
                    
                    for (auto i = 0; i != len1; ++i)
                    {
                        if (unsigned(chars1[i]) != unsigned(c2[i]))
                        {
                            return false;
                        }
                    }
                    
                    return true;
                }
            }
        }
        
        return false;
    }
    
#pragma mark ---------------------------------------- TYPE-CHECK ----------------------------------------
    
    bool isFunction(JSObject *object)
    {
        if (object)
        {
            return JS_ObjectIsFunction(cx, object);
        }
        
        return false;
    }
    
    bool isFunction(const Value &value)
    {
        if (value.isObject())
        {
            return isFunction(value.toObjectOrNull());
        }
        
        return false;
    }

    // ---
    
    bool isArray(JSObject *object)
    {
        if (object)
        {
            RootedObject rooted(cx, object);
            return JS_IsArrayObject(cx, rooted);
        }
        
        return false;
    }
    
    bool isArray(const Value &value)
    {
        if (value.isObject())
        {
            return isArray(value.toObjectOrNull());
        }
        
        return false;
    }
    
#pragma mark ---------------------------------------- TO-SOURCE ----------------------------------------
    
    string toSource(JSObject *object)
    {
        RootedValue value(cx, ObjectOrNullValue(object));
        return toSource(value); // RVO-COMPLIANT
    }

    string toSource(HandleValue value)
    {
        JSString *source = JS_ValueToSource(cx, value);
        
        if (source)
        {
            return toString(source); // RVO-COMPLIANT
        }
        
        return ""; // I.E. FAILURE
    }
    
#pragma mark ---------------------------------------- JSON ----------------------------------------
    
    /*
     * TODO:
     *
     * 1) CONSIDER IMPLEMENTING JSString* stringify() WITH jschar ACCUMULATION BUFFER
     * 2) HANDLE CUSTOM "REPLACER" (IN ORDER TO COPE WITH "CYCLIC VALUES")
     */
    
    struct intern::Stringifier
    {
        static bool callback(const jschar *buf, uint32_t len, void *data)
        {
            auto buffer = reinterpret_cast<string*>(data);
            buffer->append(toChars(buf, len));
            
            return true;
        }
    };
    
    string stringify(JSObject *object, int indent)
    {
        RootedValue value(cx, ObjectOrNullValue(object));
        return stringify(&value, indent); // RVO-COMPLIANT
    }
    
    string stringify(MutableHandleValue value, int indent)
    {
        string buffer;
        
        RootedValue indentValue(cx, (indent > 0) ? Int32Value(indent) : UndefinedHandleValue);
        
        if (JS_Stringify(cx, value, NullPtr(), indentValue, &intern::Stringifier::callback, &buffer))
        {
            return buffer; // RVO-COMPLIANT
        }
        
        return ""; // I.E. FAILURE, ON-PAR WITH JSON.stringify()
    }
    
    // ---
    
    JSObject* parse(const string &s)
    {
        if (!s.empty())
        {
            size_t len;
            jschar *chars = LossyUTF8CharsToNewTwoByteCharsZ(cx, UTF8Chars(s.data(), s.size()), &len).get();
            
            if (chars)
            {
                JSObject *result = parse(chars, len);
                js_free(chars);
                
                return result;
            }
        }
        
        return nullptr;
    }
    
    JSObject* parse(HandleValue value)
    {
        RootedString str(cx, ToString(cx, value));
        return parse(str);
    }
    
    JSObject* parse(HandleString str)
    {
        if (str)
        {
            js::RootedLinearString linear(cx, str->ensureLinear(cx)); // PROTECTING FROM GC
            
            if (linear)
            {
                return parse(linear->chars(), linear->length()); // CAN TRIGGER GC
            }
        }
        
        return nullptr;
    }
    
    JSObject* parse(const jschar *chars, size_t len)
    {
        if (chars && len)
        {
            RootedValue result(cx);
            
            if (JS_ParseJSON(cx, chars, len, &result))
            {
                if (result.isObject())
                {
                    return result.toObjectOrNull();
                }
            }
        }
        
        return nullptr;
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

bool JSP::isInsideNursery(const Value &value)
{
    if (value.isMarkable())
    {
        void *thing = value.toGCThing();
        
        if (value.isString())
        {
            return isInsideNursery((JSString*)thing); // XXX: MOST LIKELY ALWAYS FALSE
        }
        
        if (value.isObject())
        {
            return isInsideNursery((JSObject*)thing);
        }
    }
    
    return false;
}

#else

bool JSP::isPoisoned(const Value &value)
{
    return false;
}

bool JSP::isHealthy(const Value &value)
{
    return false;
}

bool JSP::isInsideNursery(const Value &value)
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

/*
 * REFERENCE FOR FUTURE WORK: https://github.com/mrdoob/three.js/blob/master/src/math/Color.js
 */

bool JSP::defineHTMLColors()
{
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

const uint32_t JSP::toHTMLColor(const string &colorName, const uint32_t defaultValue)
{
    uint32_t result;
    return lookupHTMLColor(colorName, &result) ? result : defaultValue;
}

const uint32_t JSP::toHTMLColor(HandleValue value, const uint32_t defaultValue)
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
        return convertSafely<uint32_t>(value, defaultValue);
    }
    
    return defaultValue;
}
