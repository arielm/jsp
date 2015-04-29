/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "jsp/Barker.h"
#include "jsp/Proto.h"

#if defined(JSP_USE_PRIVATE_APIS)
#include "vm/StringBuffer.h"
#endif

#include "chronotext/utils/Utils.h"

using namespace std;
using namespace chr;

namespace jsp
{
    Barker::Statics *Barker::statics = nullptr;
    int32_t Barker::lastInstanceId = -1;

    int32_t Barker::addInstance(JSObject *instance, const string &name)
    {
        for (auto &element : statics->instances)
        {
            if (instance == element.second)
            {
                JS_ASSERT(false); // THIS SHOULD NEVER HAPPEN, UNLESS BARKER-FINALIZATION IS BROKEN
            }
        }
        
        int32_t barkerId = ++lastInstanceId;
        
        // ---
        
        string finalName;
        
        if (name.empty())
        {
            finalName = "ANONYMOUS-" + ci::toString(barkerId);
        }
        else if (getId(name) >= 0)
        {
            finalName = name + " [" + ci::toString(barkerId) + "]"; // NAMES MUST BE UNIQUE (NECESSARY FOR BARKER-FORENSICS)
        }
        else
        {
            finalName = name;
        }
        
        // ---
        
        statics->instances[barkerId] = instance;
        statics->names[barkerId] = finalName;
        
        RootedObject rootedInstance(cx, instance);
        Proto::define(rootedInstance, "id", barkerId, JSPROP_READONLY | JSPROP_PERMANENT);
        Proto::define(rootedInstance, "name", finalName, JSPROP_READONLY | JSPROP_PERMANENT);
        
        /*
         * RELYING SOLELY ON THE id PROPERTY (PREVIOUSLY DEFINED) IS NOT WORKING:
         * - IT WOULD CRASH WHEN THE PROPERTY IS ACCESSED DURING TRACING VIA Barker::getId(JSObject*)
         *
         * RELYING ON THE INSTANCE'S "PRIVATE" WOULD WORK, BUT:
         * - IT'S MORE SUITED TO STORE ptrdiff_t VALUES
         */
        JS_SetReservedSlot(instance, 0, Int32Value(barkerId));
        
        // ---
        
        LOGD << "Barker CONSTRUCTED: " << JSP::writeDetailed(instance) << " | " << finalName << endl; // LOG: VERBOSE
        
        return barkerId;
    }
    
    /*
     * RETURNS AN EMPTY-STRING IF THERE IS NO SUCH A LIVING BARKER
     * OTHERWISE: MAY RETURN THE NAME OF A NON-HEALTHY BARKER
     */
    string Barker::getName(int32_t barkerId)
    {
        auto element = statics->names.find(barkerId);
        
        if (element != statics->names.end())
        {
            return element->second;
        }
        
        return ""; // I.E. SUCH A BARKER NEVER EXISTED
    }
    
    /*
     * RETURNS -1 IF THERE IS NO SUCH A LIVING BARKER
     * OTHERWISE: MAY RETURN THE ID OF A NON-HEALTHY BARKER
     */
    int32_t Barker::getId(const string &name)
    {
        for (auto &element : statics->names)
        {
            if (element.second == name)
            {
                return element.first;
            }
        }
        
        return -1; // I.E. SUCH A BARKER NEVER EXISTED
    }
    
    /*
     * IF THE RETURNED bool IS FALSE: SUCH A BARKER NEVER EXISTED
     * OTHERWISE: THE RETURNED INSTANCE MAY NOT NECESSARILY BE HEALTHY
     */
    pair<bool, JSObject*> Barker::find(const string &name)
    {
        auto barkerId = getId(name);
        
        if (barkerId >= 0)
        {
            auto instance = statics->instances.at(barkerId); // NULL AFTER FINALIZATION
            return make_pair(true, instance);
        }
        
        return make_pair(false, nullptr); // I.E. SUCH A BARKER NEVER EXISTED
    }
    
    // ---
    
    /*
     * IN ORDER TO PROPERLY "WITNESS" GC-MOVING:
     * BARKERS MUST BE CREATED IN THE NURSERY (JUST LIKE ANY OTHER "PLAIN" JS-OBJECT)
     *
     * ASSIGNING A FINALIZER WOULD FORCE BARKERS TO ALWAYS BE TENURED:
     * https://github.com/mozilla/gecko-dev/blob/esr31/js/public/RootingAPI.h#L318-319
     */
    
    const JSClass Barker::clazz =
    {
        "Barker",
        JSCLASS_HAS_RESERVED_SLOTS(1),
        JS_PropertyStub,
        JS_DeletePropertyStub,
        JS_PropertyStub,
        JS_StrictPropertyStub,
        JS_EnumerateStub,
        JS_ResolveStub,
        JS_ConvertStub,
        nullptr,/*finalize*/
        nullptr,
        nullptr,
        nullptr,
        trace
    };
    
    const JSFunctionSpec Barker::functions[] =
    {
        JS_FS("toSource", function_toSource, 0, 0),
        JS_FS("bark", function_bark, 0, 0),
        JS_FS_END
    };
    
    const JSFunctionSpec Barker::static_functions[] =
    {
        JS_FS("getInstance", static_function_getInstance, 1, 0),
        JS_FS("isFinalized", static_function_isFinalized, 1, 0),
        JS_FS("isHealthy", static_function_isHealthy, 1, 0),
        JS_FS("bark", static_function_bark, 1, 0),
        JS_FS_END
    };
    
    // ---
    
    void Barker::gcCallback(JSRuntime *rt, JSGCStatus status)
    {
        switch (status)
        {
            case JSGC_BEGIN:
                break;
                
            case JSGC_END:
            {
                auto freeOp = rt->defaultFreeOp(); // NOT USED IN PRACTICE...
                
                for (auto &element : statics->instances)
                {
                    if (element.second)
                    {
                        if (!JSP::isHealthy(element.second))
                        {
                            finalize(freeOp, element.second);
                        }
                    }
                }
                
                break;
            }
        }
    }
    
    void Barker::finalize(JSFreeOp *fop, JSObject *obj)
    {
        auto barkerId = -1;
        
        if (obj)
        {
            for (auto &element : statics->instances)
            {
                if (obj == element.second)
                {
                    barkerId = element.first;
                    break;
                }
            }
        }
        
        if (barkerId >= 0)
        {
            /*
             * NECESSARY FOR BARKER-FORENSICS:
             *
             * - NULLING (I.E. NOT ERASING) ENTRY IN statics->instances
             * - PRESERVING ENTRY IN statics->names
             */
            statics->instances[barkerId] = nullptr;
            
            LOGD << "Barker FINALIZED: " << JSP::writeDetailed(obj) << " | " << getName(barkerId) << endl; // LOG: VERBOSE
        }
    }
    
    void Barker::trace(JSTracer *trc, JSObject *obj)
    {
        if (obj != statics->prototype)
        {
            auto barkerId = getId(obj);
            
            if (barkerId >= 0)
            {
                /*
                 * NECESSARY IN CASE OF MOVED-POINTER
                 */
                statics->instances[barkerId] = obj;
                
                LOGD << "Barker TRACED: " << JSP::writeDetailed(obj) << " | " << getName(barkerId) << endl; // LOG: VERBOSE
            }
        }
    }
    
    // ---
    
    int32_t Barker::nextId()
    {
        return lastInstanceId + 1;
    }
    
    int32_t Barker::getId(JSObject *instance)
    {
        if (JSP::isHealthy(instance))
        {
            if (JS_GetClass(instance) == &Barker::clazz)
            {
                auto barkerId = JS_GetReservedSlot(instance, 0).toInt32();
                
                if (statics->instances.count(barkerId))
                {
                    return barkerId;
                }
            }
        }
        
        return -1; // I.E. THERE IS NO SUCH A LIVING BARKER
    }
    
    string Barker::getName(JSObject *instance)
    {
        return getName(getId(instance));
    }
    
    JSObject* Barker::getInstance(const string &name)
    {
        bool found;
        JSObject *instance;
        tie(found, instance) = find(name);
        
        if (found)
        {
            if (JSP::isHealthy(instance))
            {
                return instance;
            }
        }
        
        return nullptr; // I.E. THERE IS NO SUCH A LIVING BARKER
    }
    
    bool Barker::isFinalized(const string &name)
    {
        bool found;
        JSObject *instance;
        tie(found, instance) = find(name);
        
        if (found)
        {
            return !JSP::isHealthy(instance);
        }
        
        return false;
    }
    
    bool Barker::isHealthy(const string &name)
    {
        bool found;
        JSObject *instance;
        tie(found, instance) = find(name);
        
        if (found)
        {
            return JSP::isHealthy(instance);
        }
        
        return false;
    }
    
    // ---
    
    bool Barker::bark(JSObject *object)
    {
        return maybeBark(object);
    }
    
    bool Barker::bark(const Value &value)
    {
        if (value.isObject())
        {
            return maybeBark(value.toObjectOrNull());
        }
        
        return false;
    }
    
    bool Barker::bark(const string &name)
    {
        bool found;
        JSObject *instance;
        tie(found, instance) = find(name);
        
        if (found)
        {
            return maybeBark(instance);
        }
        
        return false;
    }
    
    bool Barker::maybeBark(JSObject *instance)
    {
        auto barkerId = getId(instance);
        
        if (barkerId >= 0)
        {
            LOGD << "Barker BARKED: " << JSP::writeDetailed(instance) << " | " << getName(barkerId) << endl; // LOG: VERBOSE
            return true;
        }
        
        LOGD << "ONLY HEALTHY BARKERS CAN BARK" << endl; // LOG: VERBOSE
        return false;
    }
    
    // ---
    
    bool Barker::init()
    {
        if (!statics)
        {
            statics = new Statics;
            
            /*
             * ASSERTIONS REGARDING PROTOTYPE:
             *
             * - IT IS ROOTED (VIA CONSTRUCTOR / GLOBAL)
             * - IT IS TENURED (I.E. GC-POINTER WON'T BE "MOVED")
             */
            statics->prototype = JS_InitClass(cx, globalHandle(), NullPtr(), &clazz, construct, 0, nullptr, functions, nullptr, static_functions);
            
            /*
             * THE USAGE OF Barker::clazz IS ARBITRARY (I.E. ANY "UNIQUE" POINTER WILL DO THE JOB)
             */
            JSP::addGCCallback((void*)&clazz, BIND_STATIC2(Barker::gcCallback));
        }
        
        return bool(statics);
    }
    
    void Barker::uninit()
    {
        if (statics)
        {
            /*
             * PURPOSELY NOT RESETTING lastInstanceId IN ORDER TO PREVENT INSTANCES
             * POTENTIALLY ALIVE AT THIS STAGE TO "INTERFER" IN THE FUTURE (TODO: TEST)
             */
            
            JSP::removeGCCallback((void*)&clazz);
            
            statics->names.clear();
            statics->instances.clear();
            
            statics->prototype = nullptr;
            Proto::deleteProperty(globalHandle(), "Barker"); // WILL DELETE PROTOTYPE AND CONSTRUCTOR
            
            delete statics;
            statics = nullptr;
        }
    }
    
    // ---
    
    const Barker& Barker::create(const string &name)
    {
        static Barker delegate;

        delegate.object = JS_NewObject(cx, &clazz, NullPtr(), NullPtr());
        addInstance(delegate.object, name);
        
        return delegate;
    }
    
    bool Barker::construct(JSContext *cx, unsigned argc, Value *vp)
    {
        auto args = CallArgsFromVp(argc, vp);
        string name = (args.hasDefined(0) && args[0].isString()) ? JSP::toString(args[0]) : "";
        
        args.rval().set(create(name));
        return true;
    }
    
    // ---
    
    bool Barker::function_toSource(JSContext *cx, unsigned argc, Value *vp)
    {
        auto args = CallArgsFromVp(argc, vp);
        auto instance = args.thisv().toObjectOrNull();
        
        string quotedName = '"' + getName(instance) + '"';
        
        js::StringBuffer sb(cx);
        
        if (sb.append("(new Barker(") && sb.appendInflated(quotedName.data(), quotedName.size()) && sb.append("))"))
        {
            JSString *str = sb.finishString();
            
            if (str)
            {
                args.rval().setString(str);
                return true;
            }
        }
        
        return false;
    }
    
    bool Barker::function_bark(JSContext *cx, unsigned argc, Value *vp)
    {
        auto args = CallArgsFromVp(argc, vp);
        auto instance = args.thisv().toObjectOrNull();
        
        args.rval().setBoolean(maybeBark(instance));
        return true;
    }
    
    // ---
    
    bool Barker::static_function_getInstance(JSContext *cx, unsigned argc, Value *vp)
    {
        auto args = CallArgsFromVp(argc, vp);
        
        if (args.hasDefined(0) && args[0].isString())
        {
            bool found;
            JSObject *instance;
            tie(found, instance) = find(JSP::toString(args[0]));
            
            if (found)
            {
                args.rval().setObjectOrNull(instance);
                return true;
            }
        }
        
        return false;
    }
    
    bool Barker::static_function_isFinalized(JSContext *cx, unsigned argc, Value *vp)
    {
        auto args = CallArgsFromVp(argc, vp);
        
        if (args.hasDefined(0) && args[0].isString())
        {
            args.rval().setBoolean(isFinalized(JSP::toString(args[0])));
            return true;
        }
        
        return false;
    }
    
    bool Barker::static_function_isHealthy(JSContext *cx, unsigned argc, Value *vp)
    {
        auto args = CallArgsFromVp(argc, vp);
        
        if (args.hasDefined(0) && args[0].isString())
        {
            args.rval().setBoolean(isHealthy(JSP::toString(args[0])));
            return true;
        }
        
        return false;
    }
    
    bool Barker::static_function_bark(JSContext *cx, unsigned argc, Value *vp)
    {
        auto args = CallArgsFromVp(argc, vp);
        
        if (args.hasDefined(0) && args[0].isString())
        {
            args.rval().setBoolean(bark(JSP::toString(args[0])));
            return true;
        }
        
        return false;
    }
}
