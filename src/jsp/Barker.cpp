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
        
        ptrdiff_t lastInstanceId = 0; // ID 0 IS RESERVED FOR THE CLASS-INSTANCE (WHICH IS NOT NECESSARILY CREATED)
        ptrdiff_t constructCount = 0; // CLASS-INSTANCE IS NOT COUNTED
        
        map<ptrdiff_t, string> names;
        map<ptrdiff_t, JSObject*> instances;
        
        void setup(JSObject *instance, ptrdiff_t barkerId, const string &name = "");
        string getName(ptrdiff_t barkerId);
    }
    
    void barker::setup(JSObject *instance, ptrdiff_t barkerId, const string &name)
    {
        if (barkerId > 0)
        {
            lastInstanceId = barkerId;
            
            /*
             * IN CASE THE NEW INSTANCE IS ALLOCATED AT THE ADDRESS OF A
             * PREVIOUS BARKER FINALIZED IN A "NON-ASSISTED" FASHION
             */
            
            for (auto &element : instances)
            {
                if (instance == element.second)
                {
                    instances[element.first] = nullptr;
                    break;
                }
            }
        }
        
        instances[barkerId] = instance;
        JS_SetPrivate(instance, reinterpret_cast<void*>(barkerId));
        
        /*
         * TODO: HANDLE DUPLICATE NAMES
         */
        
        if (name.empty())
        {
            names[barkerId] = "ANONYMOUS-" + ci::toString(barkerId);
        }
        else
        {
            names[barkerId] = name;
        }
        
        LOGD << "Barker CONSTRUCTED: " << JSP::writeDetailed(instance) << " | " << getName(barkerId) << endl; // LOG: VERBOSE
    }
    
    string barker::getName(ptrdiff_t barkerId)
    {
        auto element = names.find(barkerId);
        
        if (element != names.end())
        {
            return element->second;
        }
        
        return ""; // I.E. NO TRACES OF SUCH A BARKER
    }
    
#pragma mark ---------------------------------------- Barker NAMESPACE ----------------------------------------
    
    /*
     * MORE ON WHY WE PERFORM OUR OWN "ASSISTED FINALIZATION" INSTEAD OF DEFINING A FINALIZER FOR THE CLASS:
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
        nullptr,/*finalize*/ // WE WANT BARKERS TO BE "CREATED IN THE NURSERY", IN ORDER TO BE ABLE TO "WITNESS" GC-MOVING
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
    
    int Barker::finalizeCount = 0;
    int Barker::traceCount = 0;
    
    // ---
    
    ptrdiff_t Barker::getId(JSObject *instance)
    {
        if (instance)
        {
            /*
             * TODO: CHECK IF instance IS A Barker
             */
            
            if (JSP::isPoisoned(instance)) // FIXME: USE !JSP::isHealthy() INSTEAD
            {
                for (auto &element : barker::instances)
                {
                    if (instance == element.second)
                    {
                        return element.first; // I.E. NOT AFTER "ASSISTED FINALIZATION"
                    }
                }
            }
            else
            {
                auto barkerId = reinterpret_cast<ptrdiff_t>(JS_GetPrivate(instance));
                
                if (barker::instances.count(barkerId))
                {
                    return barkerId;
                }
            }
        }
        
        return -1; // I.E. NO TRACES OF SUCH A BARKER
    }
    
    string Barker::getName(JSObject *instance)
    {
        return barker::getName(getId(instance));
    }
    
    // ---
    
    void Barker::forceGC()
    {
        finalizeCount = 0;
        traceCount = 0;
        
        /*
         * JS_SetFinalizeCallback IS NECESSARY FOR OUR "ASSISTED FINALIZATION"
         *
         *
         * ALTERNATIVES:
         *
         * 1) INSTEAD OF PASSING BY Barker::forceGC(): A "CHAIN OF REGISTERABLE FINALIZER CALLBACKS"
         *    COULD BE MANAGED, E.G. VIA JSP::forceGC()
         *
         * 2) SPIDERMONKEY'S "GC CALLBACK" COULD BE USED
         */
        
        LOGD << "Barker GC-BEGIN" << endl; // LOG: VERBOSE
        
        JS_SetFinalizeCallback(rt, Barker::finalizeCallback);
        JSP::forceGC();
        JS_SetFinalizeCallback(rt, nullptr);
        
        LOGD << "Barker GC-END" << endl; // LOG: VERBOSE
    }
    
    /*
     * TODO:
     *
     * DOUBLE-CHECK THE THREAD POLICY FOR THE 3 FOLLOWING CALLBACKS
     *
     * 1) IF WE'RE NOT *ALWAYS* ON THE MAIN-THREAD:
     *    - SOME LOCKING SYSTEM MAY HAVE TO BE IMPLEMENTED
     *    - IT SEEMS PARTICULARELY RELEVANT IN THE CONTEXT OF "BACKGROUND FINALIZATION"
     *      - SEE COMMENT BELOW, IN "case JSFINALIZE_COLLECTION_END:"
     */
    
    void Barker::finalizeCallback(JSFreeOp *fop, JSFinalizeStatus status, bool isCompartmentGC)
    {
        switch (status)
        {
            case JSFINALIZE_GROUP_START:
                break;
                
            case JSFINALIZE_GROUP_END:
            {
                /*
                 * TODO:
                 *
                 * BEFORE PROGRESSING FURTHER IN THE DOMAIN OF "ASSISTED FINALIZATION"...
                 *
                 * 1) THE FOLLOWING "SWEEP" IMPLEMENTATION SHOULD BE STUDIED:
                 *    https://github.com/mozilla/gecko-dev/blob/esr31/js/src/jswatchpoint.cpp#L215-228
                 */
                
                for (auto &element : barker::instances)
                {
                    if (element.second)
                    {
                        if (!JSP::isPoisoned(element.second))
                        {
                            if (!JSP::isInsideNursery(element.second))
                            {
                                JSObject *forwarded = element.second;
                                
                                if (JS_IsAboutToBeFinalizedUnbarriered(&forwarded))
                                {
                                    finalize(fop, element.second);
                                }
                            }
                        }
                    }
                }
                break;
            }
                
            case JSFINALIZE_COLLECTION_END:
            {
                /*
                 * AT THAT POINT, IT SEEMS THAT ONLY GC-THINGS "STILL IN THE NURSERY" ARE AFFECTED
                 *
                 * TODO:
                 *
                 * 1) INVESTIGATE FURTHER AND UNDERSTAND HOW IT RELATES TO "BACKGROUND FINALIZATION"
                 *    - I.E. POSSIBLIY *NOT* TAKING PLACE ON THE MAIN-THREAD?
                 *    - HINTS: https://github.com/mozilla/gecko-dev/blob/esr31/js/src/jswrapper.cpp#L193-197
                 */
                
                for (auto &element : barker::instances)
                {
                    if (element.second)
                    {
                        if (JSP::isPoisoned(element.second))
                        {
                            finalize(fop, element.second);
                        }
                    }
                }
                break;
            }
        }
    }
    
    /*
     * IMPORTANT: OCCURS *ONLY* DURING "ASSISTED" FINALIZATION
     */
    void Barker::finalize(JSFreeOp *fop, JSObject *obj)
    {
        auto barkerId = getId(obj);
        
        if (barkerId > 0) // PURPOSELY EXCLUDING THE CLASS-INSTANCE
        {
            /*
             * KEEPING A TRACE OF THE BARKER IN barker::names IS NECESSARY
             * FOR BARKER-FORENSIC (CF Barker::isFinalized AND Barker::isHealthy)
             */
            
            barker::instances[barkerId] = nullptr;
            
            finalizeCount++;
            LOGD << "Barker FINALIZED: " << JSP::writeDetailed(obj) << " | " << barker::getName(barkerId) << endl; // LOG: VERBOSE
        }
    }
    
    void Barker::trace(JSTracer *trc, JSObject *obj)
    {
        auto barkerId = getId(obj);
        
        if (barkerId > 0) // PURPOSELY EXCLUDING THE CLASS-INSTANCE
        {
            barker::instances[barkerId] = obj; // NECESSARY IN CASE OF MOVED-POINTER
            
            if (barkerId == barker::lastInstanceId)
            {
//                barker::lastInstance = obj; // PROBABLY OVERKILL SINCE lastInstance IS EFFECTIVELY-USED DURING VERY SHORT PERIODS
            }
            
            traceCount++;
            LOGD << "Barker TRACED: " << JSP::writeDetailed(obj) << " | " << barker::getName(barkerId) << endl; // LOG: VERBOSE
        }
    }
    
    // ---
    
    /*
     * TODO:
     *
     * CHECK IF THIS CAN HELP: https://github.com/mozilla/gecko-dev/blob/esr31/js/src/jsobj.h#L1128-1138
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
    
    bool Barker::bark(const char *name)
    {
        for (auto &element : barker::names)
        {
            if (element.second == name)
            {
                return maybeBark(barker::instances.at(element.first));
            }
        }
        
        return false;
    }
    
    bool Barker::isFinalized(const char *name)
    {
        for (auto &element : barker::names)
        {
            if (element.second == name)
            {
                auto instance = barker::instances.at(element.first); // NULL ONLY IF "ASSISTED" FINALIZATION TOOK PLACE
                return !JSP::isHealthy(instance);
            }
        }
        
        return false;
    }
    
    bool Barker::isHealthy(const char *name)
    {
        for (auto &element : barker::names)
        {
            if (element.second == name)
            {
                return JSP::isHealthy(barker::instances.at(element.first));
            }
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
        
        args.rval().set(construct(name).as<Value>());
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
        if (getId(instance) >= 0)
        {
            if (JSP::isHealthy(instance))
            {
                LOGD << "Barker BARKED: " << JSP::writeDetailed(instance) << " | " << getName(instance) << endl;
                return true;
            }
        }
        
        LOGD << "ONLY HEALTHY BARKERS CAN BARK" << endl;
        return false;
    }
    
    // ---
    
    bool Barker::static_function_instances(JSContext *cx, unsigned argc, Value *vp)
    {
        /*
         * TODO
         */
        
        return true;
    }
    
    bool Barker::static_function_forceGC(JSContext *cx, unsigned argc, Value *vp)
    {
        forceGC();
        
        CallArgsFromVp(argc, vp).rval().setUndefined();
        return true;
    }
}
