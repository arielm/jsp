/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

/*
 * ROOTING STRATEGY BASED ON http://trac.wildfiregames.com/wiki/JSRootingGuide#UsingHeapTincombinationwithcustomtracers
 */

#pragma once

#include "jsp/Context.h"

#define DUMP_WRAPPED_VALUE if (LOG_VERBOSE) { dump(__PRETTY_FUNCTION__); }

namespace jsp
{
    class WrappedValue : public js::MutableValueOperations<WrappedValue>
    {
    public:
        static bool LOG_VERBOSE;
        
        // ---
        
        WrappedValue();
        ~WrappedValue();

        /*
         * THESE 2 ARE NOT MANDATORY (I.E. COMPILER-GENERATED IN ANY-CASE)
         * BUT WE WANT TO "DUMP" WHENEVER IT HAPPENS
         */
        WrappedValue(const WrappedValue &other);
        void operator=(const WrappedValue &other);

        WrappedValue(const Value &v);
        WrappedValue& operator=(const Value &newValue);
        
        template<typename T>
        WrappedValue(const T &v)
        {
            assignValue(value, v);
            DUMP_WRAPPED_VALUE
        }
    
        template<typename T>
        WrappedValue& operator=(const T &newValue)
        {
            assignValue(value, newValue);
            DUMP_WRAPPED_VALUE
            
            return *this;
        }
        
        operator const Value& () const { return value; }
        const Value* operator->() const { return &value; }

        explicit operator const bool () const;

        bool operator==(const WrappedValue &other) const;
        bool operator!=(const WrappedValue &other) const;

        bool operator==(const Value &other) const;
        bool operator!=(const Value &other) const;
        
        template<typename T>
        bool operator==(const T &other) const
        {
            return compare(value, other);
        }
        
        template<typename T>
        bool operator!=(const T &other) const
        {
            return !compare(value, other);
        }

        const Value& get() const { return value; }
        const Value* address() const { return &value; }
        Value* unsafeGet() { return &value; }
        
    protected:
        friend class ValueOperations<WrappedValue>;
        friend class MutableValueOperations<WrappedValue>;
        
        friend class Heap<WrappedValue>;
        friend struct js::GCMethods<WrappedValue>;
        
        Value value;
        
        const Value* extract() const { return &value; }
        Value* extractMutable() { return &value; }

        void postBarrier();
        void relocate();
        void trace(JSTracer *trc);
        
        void dump(const char *prefix);
    };
    
    class AutoWrappedValueVector : public AutoVectorRooter<WrappedValue>
    {
    public:
        explicit AutoWrappedValueVector()
        :
        AutoVectorRooter<WrappedValue>(cx, VALVECTOR)
        {}
        
        operator const HandleValueArray () const
        {
            return HandleValueArray::fromMarkedLocation(length(), reinterpret_cast<const Value*>(begin()));
        }
    };
}

namespace js
{
    template <>
    struct GCMethods<WrappedValue>
    {
        static WrappedValue initial() { return JS::UndefinedValue(); }
        static ThingRootKind kind() { return THING_ROOT_VALUE; }
        static bool poisoned(const WrappedValue &wrapped) { return false; }
        static bool needsPostBarrier(const WrappedValue &wrapped) { return wrapped.value.isMarkable(); }
#ifdef JSGC_GENERATIONAL
        static void postBarrier(WrappedValue *wrapped) { wrapped->postBarrier(); }
        static void relocate(WrappedValue *wrapped) { wrapped->relocate(); }
#endif
    };
    
    MOZ_ALWAYS_INLINE HeapBase<WrappedValue>::operator const bool () const
    {
        const JS::Heap<WrappedValue> &self = *static_cast<const JS::Heap<WrappedValue>*>(this);
        return bool(self.get()); // ENFORCING WrappedValue::operator const bool()
    }
    
    MOZ_ALWAYS_INLINE HeapBase<WrappedValue>::operator JS::Handle<JS::Value> () const
    {
        const JS::Heap<WrappedValue> &self = *static_cast<const JS::Heap<WrappedValue>*>(this);
        return JS::Handle<JS::Value>::fromMarkedLocation(reinterpret_cast<JS::Value const*>(self.address()));
    }
}
