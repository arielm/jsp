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
 * 1) IT WOULD BE BETTER TO HAVE:
 *    - Barker.instances['some barker name']
 *    - INSTEAD OF AS CURRENTLY:
 *      - Barker.instances('some barker name']
 *    - NOT CLEAR HOW TO ACHIEVE THIS SO FAR...
 *
 * 2) CONSIDER SWITCHING TO int32_t INSTEAD OF ptrdiff_t FOR BARKER IDS
 *
 * 3) CONSIDER USING SOME KIND OF "BI-MAP" FOR barker::instances AND barker::names
 *
 * 4) FOR THE NEXT "CUSTOM NATIVE OBJECT" (E.G. Profiler):
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
 * 5) STUDY testProfileStrings.cpp AND THE Probes MECHANISM
 */

#pragma once

#include "jsp/Context.h"

namespace jsp
{
    class Barker
    {
    public:
        operator JSObject* () const
        {
            return object;
        }
        
        operator const Value () const
        {
            return ObjectOrNullValue(object);
        }

        // ---
        
        static ptrdiff_t getId(JSObject *instance); // RETURNS -1 IF THERE IS NO SUCH A LIVING BARKER
        static std::string getName(JSObject *instance); // RETURNS AN EMPTY-STRING IF THERE IS NO SUCH A LIVING BARKER
        
        static bool isFinalized(const char *name); // I.E. ONCE A BARKER, NOW DEAD
        static bool isHealthy(const char *name); // I.E. IT'S A BARKER, AND IT'S ALIVE!
        
        // ---
        
        /*
         * ONLY HEALTHY BARKERS CAN BARK
         */
        
        static bool bark(const char *name);
        
        template <typename T>
        static bool bark(T&& thing)
        {
            return maybeBark(toObject(std::forward<T>(thing)));
        }
        
        // ---

        /*
         * MANDATORY PRIOR TO BARKER CREATION ON THE JS-SIDE
         */
        static bool init();
        
        /*
         * C++ CONSTRUCTOR
         *
         * REMINISCENT OF INCREMENTAL-GC DAYS: SOME EXTRA-CARE TO AVOID "STRAY INSTANCES" ON THE C-HEAP
         */
        MOZ_NEVER_INLINE static const Barker& construct(const std::string &name = "");
        
    protected:
        static const JSClass clazz;
        static const JSFunctionSpec functions[];
        static const JSFunctionSpec static_functions[];
        
        JSObject* object = nullptr;
        
        static void gcCallback(JSRuntime *rt, JSGCStatus status, void *data);
        static void finalize(JSFreeOp *fop, JSObject *obj);
        static void trace(JSTracer *trc, JSObject *obj);
        
        Barker() = default;
        Barker(const Barker &other) = delete;
        void operator=(const Barker &other) = delete;
        
        static bool maybeBark(JSObject *instance);
        
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
    };
}
