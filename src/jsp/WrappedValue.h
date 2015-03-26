/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

/*
 * BASED ON SPIDERMONKEY'S BarrieredValue CLASS: https://github.com/mozilla/gecko-dev/blob/esr31/js/src/gc/Barrier.h#L611-697
 *
 * RELEVANT PARTS: HOW TO IMPLEMENT YOUR OWN ValueOperations<T>
 * TO BE IGNORED: THE "PRE WRITE-BARRIER" STUFF, BECAUSE WE'RE FOCUSING ONLY ON "GENERATIONAL GC"
 */

#pragma once

#include "jsp/Context.h"

namespace jsp
{
    class WrappedValue : public js::MutableValueOperations<WrappedValue>
    {
    public:
        static bool LOG_VERBOSE;
        
        WrappedValue();
        ~WrappedValue();
        
        WrappedValue(const Value &v);
        WrappedValue& operator=(const Value &v);
        
        /*
         * THESE 2 ARE NOT MANDATORY (I.E. COMPILER-GENERATED IN ANY-CASE)
         * BUT WE WANT TO CALL dump() WHENEVER IT HAPPENS
         */
        WrappedValue(const WrappedValue &other);
        void operator=(const WrappedValue &other);
        
        template<typename T>
        WrappedValue(const T &v)
        {
            assignValue(value, v);
            dump(__PRETTY_FUNCTION__);
        }
    
        template<typename T>
        WrappedValue& operator=(const T &v)
        {
            if (traced)
            {
                endTracing();
            }
            
            assignValue(value, v);
            dump(__PRETTY_FUNCTION__);
            
            return *this;
        }
        
        operator const Value& () const { return value; }
        explicit operator const bool () const;

        bool operator==(const WrappedValue &other) const;
        bool operator!=(const WrappedValue &other) const;

        bool operator==(const Value &other) const;
        bool operator!=(const Value &other) const;
        
        bool operator==(const JSObject *other) const;
        bool operator!=(const JSObject *other) const;

        bool operator==(float f) const;
        bool operator!=(float f) const;
        
        bool operator==(double d) const;
        bool operator!=(double d) const;
        
        bool operator==(int32_t i) const;
        bool operator!=(int32_t i) const;
        
        bool operator==(uint32_t ui) const;
        bool operator!=(uint32_t ui) const;

        bool operator==(bool b) const;
        bool operator!=(bool b) const;
        
        bool operator==(const char *s) const;
        bool operator!=(const char *s) const;

        const Value& get() const { return value; }
        const Value* address() const { return &value; }
        Value* unsafeGet() { return &value; }
        
        void set(const Value &v);
        
        /*
         * TODO: USE "AUTOMATIC CONVERSION" FOR THE FOLLOWING 2 (AS IMPLEMENTED IN WrappedObject)
         */
        
        static inline Handle<WrappedValue> toHandle(const Heap<WrappedValue> &heapWrapped)
        {
            return Handle<WrappedValue>::fromMarkedLocation(heapWrapped.address());
        }
        
        static inline MutableHandle<WrappedValue> toMutableHandle(Heap<WrappedValue> &heapWrapped)
        {
            return MutableHandle<WrappedValue>::fromMarkedLocation(heapWrapped.unsafeGet());
        }
        
    protected:
        friend class ValueOperations<WrappedValue>;
        friend class MutableValueOperations<WrappedValue>;
        
        friend struct js::GCMethods<const WrappedValue>;
        friend struct js::GCMethods<WrappedValue>;
        
        Value value;
        bool traced = false;

        void dump(const char *prefix);

        bool poisoned() const;
        bool needsPostBarrier() const;
        void postBarrier();
        void relocate();
        
        void beginTracing();
        void endTracing();
        void trace(JSTracer *trc);

        const Value* extract() const { return &value; }
        Value* extractMutable() { return &value; }
    };
    
    class AutoWrappedValueVector : public AutoVectorRooter<WrappedValue>
    {
    public:
        explicit AutoWrappedValueVector(JSContext *cx MOZ_GUARD_OBJECT_NOTIFIER_PARAM)
        :
        AutoVectorRooter<WrappedValue>(cx, VALVECTOR)
        {
            MOZ_GUARD_OBJECT_NOTIFIER_INIT;
        }
        
        MOZ_DECL_USE_GUARD_OBJECT_NOTIFIER
    };
}

namespace js
{
    template <>
    struct GCMethods<const WrappedValue>
    {
        static WrappedValue initial() { return JS::UndefinedValue(); }
        static ThingRootKind kind() { return THING_ROOT_VALUE; }
        static bool poisoned(const WrappedValue &v) { return v.poisoned(); }
    };
    
    template <>
    struct GCMethods<WrappedValue>
    {
        static WrappedValue initial() { return JS::UndefinedValue(); }
        static ThingRootKind kind() { return THING_ROOT_VALUE; }
        static bool poisoned(const WrappedValue &v) { return v.poisoned(); }
        static bool needsPostBarrier(const WrappedValue &v) { return v.needsPostBarrier(); }
#ifdef JSGC_GENERATIONAL
        static void postBarrier(WrappedValue *vp) { vp->postBarrier(); }
        static void relocate(WrappedValue *vp) { vp->relocate(); }
#endif
    };
}
