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
        
        /*
         * THESE 2 ARE NOT MANDATORY (I.E. COMPILER-GENERATED IN ANY-CASE)
         * BUT WE WANT TO CALL dump() WHENEVER IT HAPPENS
         */
        WrappedObject(const WrappedObject &other);
        void operator=(const WrappedObject &other);
        
        operator JSObject* () const { return value; }
        JSObject* operator->() const { return value; }
        
        operator const Value () const { return ObjectOrNullValue(value); }
        
        const JSObject& get() const { return *value; }
        const JSObject* address() const { return value; }
        JSObject* unsafeGet() { return value; }
        
        void set(JSObject *object);
        void clear();
        
    protected:
        friend class Heap<const WrappedObject>;
        friend struct js::GCMethods<const WrappedObject>;
        
        friend class Heap<WrappedObject>;
        friend struct js::GCMethods<WrappedObject>;
        
        JSObject *value;
        
        void dump(const char *prefix);

        bool poisoned() const;
        bool needsPostBarrier() const;
        void postBarrier();
        void relocate();
        
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
    
    /*
     * REFERENCES:
     *
     * - https://github.com/mozilla/gecko-dev/blob/esr31/js/public/RootingAPI.h#L865-881
     * - https://github.com/mozilla/gecko-dev/blob/esr31/js/src/jsobj.h#L1164-1171
     */
    
    MOZ_ALWAYS_INLINE HeapBase<WrappedObject>::operator const bool () const
    {
        const JS::Heap<WrappedObject> &self = *static_cast<const JS::Heap<WrappedObject>*>(this);
        return self.get();
    }
    
    MOZ_ALWAYS_INLINE HeapBase<WrappedObject>::operator JS::Handle<JSObject*> () const
    {
        const JS::Heap<WrappedObject> &self = *static_cast<const JS::Heap<WrappedObject>*>(this);
        return JS::Handle<JSObject*>::fromMarkedLocation(reinterpret_cast<JSObject* const*>(self.address()));
    }
    
    MOZ_ALWAYS_INLINE HeapBase<WrappedObject>::operator JS::Handle<WrappedObject> () const
    {
        const JS::Heap<WrappedObject> &self = *static_cast<const JS::Heap<WrappedObject>*>(this);
        return JS::Handle<WrappedObject>::fromMarkedLocation(reinterpret_cast<WrappedObject const*>(self.address()));
    }
    
    MOZ_ALWAYS_INLINE HeapBase<WrappedObject>::operator JS::MutableHandle<JSObject*> ()
    {
        JS::Heap<WrappedObject> &self = *static_cast<JS::Heap<WrappedObject>*>(this);
        return JS::MutableHandle<JSObject*>::fromMarkedLocation(reinterpret_cast<JSObject**>(self.unsafeGet()));
    }
    
    MOZ_ALWAYS_INLINE HeapBase<WrappedObject>::operator JS::MutableHandle<WrappedObject> ()
    {
        JS::Heap<WrappedObject> &self = *static_cast<JS::Heap<WrappedObject>*>(this);
        return JS::MutableHandle<WrappedObject>::fromMarkedLocation(reinterpret_cast<WrappedObject*>(self.unsafeGet()));
    }
    
    template <class U>
    MOZ_ALWAYS_INLINE Handle<U*> HeapBase<WrappedObject>::as() const
    {
        const JS::Heap<WrappedObject> &self = *static_cast<const JS::Heap<WrappedObject>*>(this);
        JS_ASSERT(self.get()->is<U>());
        return JS::Handle<U*>::fromMarkedLocation(reinterpret_cast<U* const*>(self.address()));
    }
}
