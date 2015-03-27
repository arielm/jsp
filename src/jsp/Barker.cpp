/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "jsp/Barker.h"

#include "chronotext/utils/Utils.h"

using namespace std;
using namespace chr;

namespace jsp
{
#pragma mark ---------------------------------------- barker NAMESPACE ----------------------------------------
    
    namespace barker
    {
        bool initialized = false;
        ptrdiff_t constructCount = 0; // CLASS-INSTANCE IS NOT COUNTED
        
        map<ptrdiff_t, string> names;
        map<ptrdiff_t, JSObject*> instances;
        
        void setup(JSObject *instance, ptrdiff_t barkerId, const string &name = "");
        bool maybeCleanup(JSObject *instance);
        
        /*
         * RETURNS AN EMPTY-STRING IF THERE IS NO SUCH A LIVING BARKER
         * OTHERWISE: MAY RETURN THE NAME OF A NON-HEALTHY BARKER
         */
        string getName(ptrdiff_t barkerId);

        /*
         * RETURNS -1 IF THERE IS NO SUCH A LIVING BARKER
         * OTHERWISE: MAY RETURN THE ID OF A NON-HEALTHY BARKER
         */
        ptrdiff_t getId(const string &name);
        
        /*
         * IF THE RETURNED bool IS FALSE: SUCH A BARKER NEVER EXISTED
         * OTHERWISE: THE RETURNED INSTANCE MAY NOT NECESSARILY BE HEALTHY
         */
        pair<bool, JSObject*> getInstance(const string &name); //
    }
    
    void barker::setup(JSObject *instance, ptrdiff_t barkerId, const string &name)
    {
        assert(instance);
        assert(barkerId >= 0);
        
        RootedObject rootedInstance(cx, instance);
        
        if (barkerId > 0)
        {
            /*
             * IN CASE THE NEW INSTANCE IS ALLOCATED AT THE ADDRESS OF A
             * PREVIOUS BARKER NOT FINALIZED DURING Barker::forceGC()
             */
            maybeCleanup(instance);
        }
        
        instances[barkerId] = instance;
        JS_SetPrivate(instance, reinterpret_cast<void*>(barkerId));
        
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
        
        names[barkerId] = finalName;
        
        // ---
        
        RootedValue rootedId(cx, Int32Value(barkerId));
        JS_DefineProperty(cx, rootedInstance, "id", rootedId, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT);

        
        RootedValue rootedName(cx, StringValue(toJSString(finalName.data())));
        JS_DefineProperty(cx, rootedInstance, "name", rootedName, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT);
        
        // ---
        
        LOGD << "Barker CONSTRUCTED: " << JSP::writeDetailed(instance) << " | " << finalName << endl; // LOG: VERBOSE
    }
    
    bool barker::maybeCleanup(JSObject *instance)
    {
        for (auto &element : instances)
        {
            if (instance == element.second)
            {
                instances[element.first] = nullptr;
                return true;
            }
        }
        
        return false;
    }
    
    string barker::getName(ptrdiff_t barkerId)
    {
        auto element = names.find(barkerId);
        
        if (element != names.end())
        {
            return element->second;
        }
        
        return ""; // I.E. SUCH A BARKER NEVER EXISTED
    }
    
    ptrdiff_t barker::getId(const string &name)
    {
        for (auto &element : names)
        {
            if (element.second == name)
            {
                return element.first;
            }
        }
        
        return -1; // I.E. SUCH A BARKER NEVER EXISTED
    }
    
    pair<bool, JSObject*> barker::getInstance(const string &name)
    {
        auto barkerId = getId(name);
        
        if (barkerId >= 0)
        {
            auto instance = instances.at(barkerId); // XXX: NULL ONLY IF FINALIZED DURING Barker::forceGC()
            return make_pair(true, instance);
        }
        
        return make_pair(false, nullptr); // I.E. SUCH A BARKER NEVER EXISTED
    }
    
#pragma mark ---------------------------------------- Barker NAMESPACE ----------------------------------------
    
    /*
     * MORE ON WHY WE PERFORM OUR OWN "ASSISTED" FINALIZATION INSTEAD OF DEFINING A FINALIZER FOR THE CLASS:
     *
     * https://github.com/mozilla/gecko-dev/blob/esr31/js/public/RootingAPI.h#L319-320
     */
    
    const JSClass Barker::clazz =
    {
        "Barker",
        JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,
        JS_DeletePropertyStub,
        JS_PropertyStub,
        JS_StrictPropertyStub,
        JS_EnumerateStub,
        JS_ResolveStub,
        JS_ConvertStub,
        nullptr,/*finalize*/ // WE WANT BARKERS TO BE CREATED IN THE NURSERY, IN ORDER TO BE ABLE TO "WITNESS" GC-MOVING
        nullptr,
        nullptr,
        nullptr,
        trace
    };
    
    const JSFunctionSpec Barker::functions[] =
    {
        JS_FS("bark", function_bark, 0, 0),
        JS_FS_END
    };
    
    const JSFunctionSpec Barker::static_functions[] =
    {
        JS_FS("instances", static_function_instances, 1, 0),
        JS_FS("forceGC", static_function_forceGC, 0, 0),
        JS_FS_END
    };
    
    // ---
    
    ptrdiff_t Barker::getId(JSObject *instance)
    {
        if (JSP::isHealthy(instance))
        {
            if (JS_GetClass(instance) == &Barker::clazz)
            {
                auto barkerId = reinterpret_cast<ptrdiff_t>(JS_GetPrivate(instance));
                
                if (barker::instances.count(barkerId))
                {
                    return barkerId;
                }
            }
        }
        
        return -1; // I.E. THERE IS NO SUCH A LIVING BARKER
    }
    
    string Barker::getName(JSObject *instance)
    {
        return barker::getName(getId(instance));
    }
    
    // ---
    
    void Barker::forceGC()
    {
        /*
         * TODO:
         *
         * INSTEAD OF PASSING BY Barker::forceGC():
         *
         * - JS_SetGCCallback() COULD BE USED AT THE jsp MAINSPACE LEVEL,
         *   TOGETHER WITH A CHAIN OF REGISTERABLE GC-CALLBACKS
         *
         * - ADVANTAGE: POSSIBILITY TO DETECT ANY FINALIZED BARKER
         */

        LOGD << "Barker GC-BEGIN" << endl; // LOG: VERBOSE

        JS_SetGCCallback(rt, Barker::gcCallback, nullptr);
        JSP::forceGC();
        JS_SetGCCallback(rt, nullptr, nullptr);
        
        LOGD << "Barker GC-END" << endl; // LOG: VERBOSE
    }
    void Barker::gcCallback(JSRuntime *rt, JSGCStatus status, void *data)
    {
        switch (status)
        {
            case JSGC_BEGIN:
                break;
                
            case JSGC_END:
            {
                auto freeOp = rt->defaultFreeOp();
                
                for (auto &element : barker::instances)
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
    
    /*
     * IMPORTANT: CURRENTLY OCCURS ONLY DURING Barker::forceGC()
     */
    void Barker::finalize(JSFreeOp *fop, JSObject *obj)
    {
        auto barkerId = -1;
        
        if (obj)
        {
            for (auto &element : barker::instances)
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
             * - NULLING (I.E. NOT ERASING) ENTRY IN barker::instances
             * - PRESERVING ENTRY IN barker::names
             */
            barker::instances[barkerId] = nullptr;
            
            LOGD << "Barker FINALIZED: " << JSP::writeDetailed(obj) << " | " << barker::getName(barkerId) << endl; // LOG: VERBOSE
        }
    }
    
    void Barker::trace(JSTracer *trc, JSObject *obj)
    {
        auto barkerId = getId(obj);
        
        if (barkerId >= 0)
        {
            /*
             * NECESSARY IN CASE OF MOVED-POINTER
             */
            barker::instances[barkerId] = obj;
            
            if (barkerId > 0) // EXCLUDING THE CLASS-INSTANCE
            {
                LOGD << "Barker TRACED: " << JSP::writeDetailed(obj) << " | " << barker::getName(barkerId) << endl; // LOG: VERBOSE
            }
        }
    }
    
    // ---
    
    /*
     * TODO:
     *
     * CHECK IF THIS CAN HELP WITH THE FOLOWING 2: https://github.com/mozilla/gecko-dev/blob/esr31/js/src/jsobj.h#L1128-1138
     */
    
    template <>
    JSObject* Barker::as() const
    {
        return object;
    }
    
    template <>
    Value Barker::as() const
    {
        return ObjectOrNullValue(object);
    }
    
    // ---
    
    bool Barker::isFinalized(const char *name)
    {
        bool found;
        JSObject *instance;
        tie(found, instance) = barker::getInstance(name);
        
        if (found)
        {
            return !JSP::isHealthy(instance);
        }
        
        return false;
    }
    
    bool Barker::isHealthy(const char *name)
    {
        bool found;
        JSObject *instance;
        tie(found, instance) = barker::getInstance(name);
        
        if (found)
        {
            return JSP::isHealthy(instance);
        }
        
        return false;
    }
    
    
    bool Barker::bark(const char *name)
    {
        bool found;
        JSObject *instance;
        tie(found, instance) = barker::getInstance(name);
        
        if (found)
        {
            return maybeBark(instance);
        }
        
        return false;
    }
    
    // ---
    
    bool Barker::init()
    {
        if (!barker::initialized)
        {
            auto classInstance = JS_InitClass(cx, globalHandle(), NullPtr(), &clazz, construct, 0, nullptr, functions, nullptr, static_functions);
            barker::setup(classInstance, 0, "CLASS-INSTANCE");
            
            barker::initialized = true;
        }
        
        return barker::initialized;
    }
    
    const Barker& Barker::construct(const string &name)
    {
        static Barker delegate;

        delegate.object = JS_NewObject(cx, &clazz, NullPtr(), NullPtr());
        barker::setup(delegate.object, ++barker::constructCount, name);
        
        return delegate;
    }
    
    bool Barker::construct(JSContext *cx, unsigned argc, Value *vp)
    {
        auto args = CallArgsFromVp(argc, vp);
        string name;
        
        if (args.hasDefined(0) && args[0].isString())
        {
            name = toString(args[0]);
        }
        
        args.rval().set(construct(name));
        return true;
    }
    
    // ---
    
    bool Barker::function_bark(JSContext *cx, unsigned argc, Value *vp)
    {
        auto args = CallArgsFromVp(argc, vp);
        auto instance = args.thisv().toObjectOrNull();
        
        args.rval().setBoolean(maybeBark(instance));
        return true;
    }
    
    bool Barker::maybeBark(JSObject *instance)
    {
        auto barkerId = getId(instance);
        
        if (barkerId >= 0)
        {
            LOGD << "Barker BARKED: " << JSP::writeDetailed(instance) << " | " << barker::getName(barkerId) << endl;
            return true;
        }
        
        LOGD << "ONLY HEALTHY BARKERS CAN BARK" << endl;
        return false;
    }
    
    // ---
    
    bool Barker::static_function_instances(JSContext *cx, unsigned argc, Value *vp)
    {
        auto args = CallArgsFromVp(argc, vp);
        
        if (args.hasDefined(0) && args[0].isString())
        {
            bool found;
            JSObject *instance;
            tie(found, instance) = barker::getInstance(toString(args[0]));
            
            if (found)
            {
                args.rval().setObjectOrNull(instance);
                return true;
            }
        }
        
        args.rval().setUndefined();
        return true;
    }
    
    bool Barker::static_function_forceGC(JSContext *cx, unsigned argc, Value *vp)
    {
        forceGC();
        
        CallArgsFromVp(argc, vp).rval().setUndefined();
        return true;
    }
}
