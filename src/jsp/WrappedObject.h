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

#include <set>

#define DUMP_WRAPPED_OBJECT if (LOG_VERBOSE) { dump(__PRETTY_FUNCTION__); }

namespace jsp
{
    class WrappedObject
    {
    public:
        static bool LOG_VERBOSE;
        static std::set<void*> heapTraced;
        
        // ---
        
        WrappedObject();
        ~WrappedObject();
        
        WrappedObject(JSObject *o);
        WrappedObject& operator=(JSObject *newObject);
        
        WrappedObject(const HandleObject &handle);
        WrappedObject& operator=(const HandleObject &handle);
        
        /*
         * THESE 2 ARE NOT MANDATORY (I.E. COMPILER-GENERATED IN ANY-CASE)
         * BUT WE WANT TO CALL dump() WHENEVER IT HAPPENS
         */
        WrappedObject(const WrappedObject &other);
        void operator=(const WrappedObject &other);
        
        operator JSObject* () const { return object; }
        JSObject* operator->() const { return object; }
        
        operator const Value () const { return ObjectOrNullValue(object); }
        
        const JSObject& get() const { return *object; }
        const JSObject* address() const { return object; }
        JSObject* unsafeGet() { return object; }
        
        void set(JSObject *newObject);
        void clear();
        
    protected:
        friend class Heap<const WrappedObject>;
        friend struct js::GCMethods<const WrappedObject>;
        
        friend class Heap<WrappedObject>;
        friend struct js::GCMethods<WrappedObject>;
        
        JSObject *object;
        
        void postBarrier();
        void relocate();
        
        void beginTracing();
        void endTracing();
        void trace(JSTracer *trc);
        
        void dump(const char *prefix);
    };
}

namespace js
{
    template <>
    struct GCMethods<const WrappedObject>
    {
        static WrappedObject initial() { return nullptr; }
        static ThingRootKind kind() { return THING_ROOT_OBJECT; }
        static bool poisoned(const WrappedObject &wrapped) { return false; }
    };
    
    template <>
    struct GCMethods<WrappedObject>
    {
        static WrappedObject initial() { return nullptr; }
        static ThingRootKind kind() { return THING_ROOT_OBJECT; }
        static bool poisoned(const WrappedObject &wrapped) { return false; }
        static bool needsPostBarrier(const WrappedObject &wrapped) { return wrapped.object; }
#ifdef JSGC_GENERATIONAL
        static void postBarrier(WrappedObject *wrapped) { wrapped->postBarrier(); }
        static void relocate(WrappedObject *wrapped) { wrapped->relocate(); }
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
