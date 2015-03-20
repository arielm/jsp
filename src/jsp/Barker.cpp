/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "Barker.h"
#include "WrappedObject.h"
#include "WrappedValue.h"

#include "chronotext/utils/Utils.h"

using namespace std;
using namespace chr;

namespace jsp
{
#pragma mark ---------------------------------------- barker NAMESPACE ----------------------------------------
    
    namespace barker
    {
        ptrdiff_t lastInstanceId = 0; // ID 0 IS RESERVED FOR THE CLASS-INSTANCE (WHICH IS NOT NECESSARILY CREATED)
        ptrdiff_t constructCount = 0; // CLASS-INSTANCE IS NOT COUNTED
        
        map<ptrdiff_t, string> names;
        map<ptrdiff_t, JSObject*> instances;
        
        JSObject* classInstance = nullptr;
        JSObject* lastInstance = nullptr;
        
        void setup(JSObject *instance, ptrdiff_t barkerId, const string &name = "");
        string getName(ptrdiff_t barkerId);
    }
    
    void barker::setup(JSObject *instance, ptrdiff_t barkerId, const string &name)
    {
        if (barkerId > 0)
        {
            lastInstanceId = barkerId;
        }
        
        instances[barkerId] = instance;
        JS_SetPrivate(instance, reinterpret_cast<void*>(barkerId));
        
        if (name.empty())
        {
            names[barkerId] = "ANONYMOUS-" + ci::toString(barkerId);
        }
        else
        {
            names[barkerId] = name;
        }
        
        LOGD << "Barker CONSTRUCTED: " << JSP::writeDetailed(instance) << " | " << getName(barkerId) << endl;
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
        JS_FS("bark", bark, 0, 0),
        JS_FS_END
    };
    
    int Barker::finalizeCount = 0;
    int Barker::traceCount = 0;
    
    // ---
    
    ptrdiff_t Barker::getId(JSObject *instance)
    {
        if (instance)
        {
            if (JSP::isPoisoned(instance)) // FIXME: USE !JSP::isHealthy() INSTEAD
            {
                for (auto &element : barker::instances)
                {
                    if (instance == element.second)
                    {
                        return element.first;
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
        
        LOGD << "Barker GC-BEGIN" << endl;
        
        JS_SetFinalizeCallback(rt, Barker::finalizeCallback);
        JSP::forceGC();
        JS_SetFinalizeCallback(rt, nullptr);
        
        LOGD << "Barker GC-END" << endl;
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
    
    void Barker::finalize(JSFreeOp *fop, JSObject *obj)
    {
        auto barkerId = getId(obj);
        
        if (barkerId >= 0)
        {
            finalizeCount++;
            LOGD << "Barker FINALIZED: " << JSP::writeDetailed(obj) << " | " << barker::getName(barkerId) << endl;
            
            /*
             * KEEPING A TRACE OF THE BARKER IN barker::names IS NECESSARY
             * FOR BARKER-FORENSIC (CF Barker::isFinalized AND Barker::isHealthy)
             */
            barker::instances[barkerId] = nullptr;
            
            if (barkerId == 0)
            {
                barker::classInstance = nullptr;
            }
            else if (barkerId == barker::lastInstanceId)
            {
                barker::lastInstance = nullptr;
            }
        }
    }
    
    void Barker::trace(JSTracer *trc, JSObject *obj)
    {
        auto barkerId = getId(obj);
        
        if (barkerId >= 0)
        {
            barker::instances[barkerId] = obj; // NECESSARY IN CASE OF MOVED-POINTER
            
            if (barkerId > 0) // PURPOSELY NOT TRACING THE CLASS-INSTANCE
            {
                traceCount++;
                LOGD << "Barker TRACED: " << JSP::writeDetailed(obj) << " | " << barker::getName(barkerId) << endl;
            }
        }
    }
    
    // ---
    
    Barker::operator JSObject* () const
    {
        return barker::lastInstance;
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
        return barker::lastInstance;
    }
    
    template <>
    WrappedObject Barker::as() const
    {
        return WrappedObject(barker::lastInstance);
    }
    
    template <>
    Value Barker::as() const
    {
        return ObjectOrNullValue(barker::lastInstance);
    }
    
    template <>
    WrappedValue Barker::as() const
    {
        return ObjectOrNullValue(barker::lastInstance);
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
                auto instance = barker::instances.at(element.first);
                return !instance || JSP::isPoisoned(instance); // FIXME: USE !JSP::isHealthy()
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
                auto instance = barker::instances.at(element.first);
                return instance && !JSP::isPoisoned(instance); // FIXME: USE JSP::isHealthy()
            }
        }
        
        return false;
    }
    
    // ---
    
    bool Barker::init()
    {
        if (!barker::classInstance)
        {
            barker::classInstance = JS_InitClass(cx, globalHandle(), NullPtr(), &clazz, construct, 0, nullptr, functions, nullptr, nullptr);
            
            if (barker::classInstance)
            {
                barker::setup(barker::classInstance, 0, "CLASS-INSTANCE");
            }
            else
            {
                LOGD << "Barker CONSTRUCTION FAILED" << endl;
                return false;
            }
        }
        
        return true;
    }
    
    const Barker& Barker::construct(const string &name)
    {
        barker::lastInstance = JS_NewObject(cx, &clazz, NullPtr(), NullPtr());
        
        if (barker::lastInstance)
        {
            barker::setup(barker::lastInstance, ++barker::constructCount, name);
        }
        else
        {
            LOGD << "Barker CONSTRUCTION FAILED" << endl;
        }
        
        static Barker delegate;
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
    
    bool Barker::bark(JSContext *cx, unsigned argc, Value *vp)
    {
        auto args = CallArgsFromVp(argc, vp);
        auto instance = args.thisv().toObjectOrNull();
        
        args.rval().setBoolean(maybeBark(instance));
        return true;
    }
    
    bool Barker::maybeBark(JSObject *instance)
    {
        if (getId(instance) != -1) // I.E. ONCE A BARKER
        {
            if (!JSP::isPoisoned(instance)) // FIXME: USE JSP::isHealthy()
            {
                LOGD << "Barker BARKED: " << JSP::writeDetailed(instance) << " | " << getName(instance) << endl;
                return true;
            }
        }
        
        LOGD << "ONLY HEALTHY BARKERS CAN BARK" << endl;
        return false;
    }
}
