/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

/*
 * BARKER BASED ON: js/src/jsapi-tests/testPersistentRooted.cpp
 *
 * FOLLOW-UP: IT WAS A PRETTY MISLEADING CODE-SAMPLE!
 *
 * - NOT ADAPTED TO "GENERATIONAL GC"
 * - FOCUSED ON TRYING TO AVOID "STRAY INSTANCES" ON THE C-STACK (A PURE "INCREMENTAL GC" TOPIC)
 * - USING A CUSTOM CLASS FINALIZER:
 *   - THE LAST THING TO DO WHEN STUDYING "GENERATIONAL GC":
 *     - SUCH OBJECTS ARE CREATED DIRECTLY ON THE THE "TENURED HEAP"!
 *     - I.E. IT WAS THEREFORE IMPOSSIBLE TO WITNESS "MOVING FROM THE NURSERY"
 *
 *
 * TODO:
 *
 * 0) WE COULD DIFFERENTIATE BETWEEN JS-CREATED OR CPP-CREATED BARKERS
 *    - E.G. VIA A METHOD OR WHEN WRITING THE BARKER'S NAME...
 *
 * 1) TRY TO USE std:forward IN TEMPLATES WHENEVER POSSIBLE
 *
 * 2) FOR THE NEXT "CUSTOM NATIVE OBJECT" (E.G. Profiler):
 *    - THE JS construct() COULD USE JS_NewObjectForConstructor
 *      - STUDY constructHook IN testNewObject.cpp:
 *        - TO UNDERSTAND:
 *          - JS_New()
 *          - js::Jsvalify()
 *          - args.isConstructing()
 *        - REPORT JS ERRORS IF NECESSARY
 *    - WORTHWILE TOO:
 *      - CUSTOM NATIVE OBJECT EXTENDING JSObject:
 *        - https://github.com/mozilla/gecko-dev/blob/esr31/js/src/builtin/TestingFunctions.cpp#L1262-1421
 *        - WITH CUSTOM PROPERTY WITH GETTER / SETTER
 *        - DEMONSTRATES USAGE OF THE "CallNonGenericMethod" TEMPLATE!
 *    - ALTERNATIVELY:
 *      - CHECK THE "Helper Macros for creating JSClasses that function as proxies" (EG. "PROXY_CLASS_WITH_EXT") IN jsfriendapi.h
 *        - EXAMPLE IN jsapi-tests/testBug604087.cpp
 *
 * 3) STUDY testProfileStrings.cpp AND THE Probes MECHANISM
 */

#pragma once

#include "jsp/Context.h"

namespace jsp
{
    class Barker
    {
    public:
        static int finalizeCount;
        static int traceCount;
        
        static void forceGC();
        
        // ---
        
        operator JSObject* () const
        {
            return object;
        }
        
        operator const Value () const
        {
            return ObjectOrNullValue(object);
        }

        template<class T>
        T as() const;

        // ---
        
        /*
         * ONLY HEALTHY BARKERS CAN BARK
         */
        
        static bool bark(const char *name);
        
        template <typename T>
        static bool bark(T &&thing)
        {
            return maybeBark(toObject(std::forward<T>(thing)));
        }
        
        // ---
        
        static ptrdiff_t getId(JSObject *instance); // CAN RETURN THE ID OF A POISONED BARKER (I.E. AFTER "NON-ASSISTED" FINALIZATION)
        static std::string getName(JSObject *instance); // CAN RETURN THE NAME OF A POISONED BARKER (I.E. AFTER "NON-ASSISTED" FINALIZATION)
        
        static bool isFinalized(const char *name); // I.E. ONCE A BARKER, NOW DEAD
        static bool isHealthy(const char *name); // I.E. IT'S A BARKER, AND IT'S ALIVE!
        
        // ---

        /*
         * MANDATORY PRIOR TO BARKER CREATION ON THE JS-SIDE
         */
        static bool init();
        
        /*
         * C++ CONSTRUCTOR
         *
         * REMINISCENT OF INCREMENTAL-GC DAYS: SOME "EXTRA-CARE" TO AVOID "STRAY INSTANCES" ON THE C-HEAP
         */
        MOZ_NEVER_INLINE static const Barker& construct(const std::string &name = "");
        
        /*
         * JS CONSTRUCTOR
         */
        static bool construct(JSContext *cx, unsigned argc, Value *vp);
        
        /*
         * JS FUNCTIONS
         */
        static bool function_bark(JSContext *cx, unsigned argc, Value *vp);
        
        /*
         * STATIC JS FUNCTIONS
         */
        static bool static_function_instances(JSContext *cx, unsigned argc, Value *vp);
        static bool static_function_forceGC(JSContext *cx, unsigned argc, Value *vp);
        
    protected:
        static const JSClass clazz;
        static const JSFunctionSpec functions[];
        static const JSFunctionSpec static_functions[];
        
        JSObject* object = nullptr;
        
        static void finalizeCallback(JSFreeOp *fop, JSFinalizeStatus status, bool isCompartmentGC);
        static void finalize(JSFreeOp *fop, JSObject *obj);
        static void trace(JSTracer *trc, JSObject *obj);
        
        Barker() = default;
        Barker(const Barker &other) = delete;
        void operator=(const Barker &other) = delete;
        
        static bool maybeBark(JSObject *instance);
    };
}
