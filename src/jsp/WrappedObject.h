/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

/*
 * ROOTING STRATEGY BASED ON http://trac.wildfiregames.com/wiki/JSRootingGuide#UsingHeapTincombinationwithcustomtracers
 *
 *
 * TODO:
 *
 * 1) IMPLEMENT "COMPARISON OPERATORS", AS IN WrappedValue
 *    - TO TAKE IN COUNT: https://github.com/mozilla/gecko-dev/blob/esr31/js/src/jsobj.h#L1173-1188
 *
 * 2) SEE IF WE NEED A AutoWrappedObjectVector (I.E. SIMILAR TO AutoWrappedValueVector)
 */

#pragma once

#include "jsp/Context.h"

namespace jsp
{
    class WrappedObject
    {
    public:
        static bool LOG_VERBOSE;
        
        WrappedObject();
        ~WrappedObject();
        
        WrappedObject(JSObject *object);
        WrappedObject& operator=(JSObject *object);
        
        WrappedObject(const HandleObject &handle);
        WrappedObject& operator=(const HandleObject &handle);
        
        WrappedObject(const WrappedObject &other);
        void operator=(const WrappedObject &other);
        
        operator JSObject* () const { return value; }
        operator const Value () const { return ObjectOrNullValue(value); }
        
        const JSObject& get() const { return *value; }
        const JSObject* address() const { return value; }
        JSObject* unsafeGet() { return value; }
        
        void set(JSObject *object);
        void reset();
        
    protected:
        friend class Heap<const WrappedObject>;
        friend struct js::GCMethods<const WrappedObject>;
        
        friend class Heap<WrappedObject>;
        friend struct js::GCMethods<WrappedObject>;
        
        JSObject *value;
        
        bool traced;
        int traceCount;
        
        bool poisoned() const;
        bool needsPostBarrier() const;
        void postBarrier();
        void relocate();
        
        void beginTracing();
        void endTracing();
        void trace(JSTracer *trc);
    };
}

namespace js
{
    template <>
    struct GCMethods<const WrappedObject>
    {
        static WrappedObject initial() { return nullptr; }
        static ThingRootKind kind() { return RootKind<JSObject*>::rootKind(); }
        static bool poisoned(const WrappedObject &v) { return v.poisoned(); }
    };
    
    template <>
    struct GCMethods<WrappedObject>
    {
        static WrappedObject initial() { return nullptr; }
        static ThingRootKind kind() { return RootKind<JSObject*>::rootKind(); }
        static bool poisoned(const WrappedObject &v) { return v.poisoned(); }
        static bool needsPostBarrier(const WrappedObject &v) { return v.needsPostBarrier(); }
#ifdef JSGC_GENERATIONAL
        static void postBarrier(WrappedObject *vp) { vp->postBarrier(); }
        static void relocate(WrappedObject *vp) { vp->relocate(); }
#endif
    };
}
