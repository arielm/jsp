/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "WrappedObject.h"

#include "chronotext/Log.h"

using namespace std;
using namespace chr;

namespace jsp
{
    bool WrappedObject::LOG_VERBOSE = false;

    WrappedObject::WrappedObject()
    :
    WrappedObject(nullptr)
    {}
    
    WrappedObject::~WrappedObject()
    {
        assert(!traced); // TODO: FOLLOW-UP
        
        LOGD_IF(LOG_VERBOSE) << __PRETTY_FUNCTION__ << " " << this << endl;
    }
    
    WrappedObject::WrappedObject(JSObject *object)
    :
    value(object),
    traced(false),
    traceCount(0)
    {
        LOGD_IF(LOG_VERBOSE) << __PRETTY_FUNCTION__ << " " << this << " | value: " << JSP::writeDetailed(value) << endl;
    }
    
    WrappedObject& WrappedObject::operator=(JSObject *object)
    {
        set(object);
        return *this;
    }
    
    WrappedObject::WrappedObject(const HandleObject &handle)
    :
    WrappedObject(handle.get())
    {}
    
    WrappedObject& WrappedObject::operator=(const HandleObject &handle)
    {
        set(handle.get());
        return *this;
    }
    
    WrappedObject::WrappedObject(const WrappedObject &other)
    :
    WrappedObject(other.value)
    {}
    
    void WrappedObject::operator=(const WrappedObject &other)
    {
        set(other.value);
    }
    
    void WrappedObject::set(JSObject *object)
    {
        if (object != value)
        {
            if (traced)
            {
                endTracing();
            }
            
            value = object;
            
            LOGD_IF(LOG_VERBOSE) << __PRETTY_FUNCTION__ << " " << this << " | value: " << JSP::writeDetailed(value) << endl;
        }
    }
    
    void WrappedObject::reset()
    {
        set(nullptr);
    }
    
    // ---
    
    bool WrappedObject::poisoned() const
    {
        /*
         * THE JS::IsPoisonedPtr() USUALLY INVOKED IN SPIDERMONKEY CODE IS DUMMY (ALWAYS RETURNS FALSE),
         * SO USING THE "REAL" JSP::isPoisoned() WOULD BE A WASTE OF CYCLES
         */
        
        return false;
    }
    
    bool WrappedObject::needsPostBarrier() const
    {
        return value;
    }
    
    void WrappedObject::postBarrier()
    {
        beginTracing();
        HeapCellPostBarrier(reinterpret_cast<js::gc::Cell**>(&value));
        
        LOGD_IF(LOG_VERBOSE) << __PRETTY_FUNCTION__ << " " << this << " | value: " << JSP::writeDetailed(value) << endl;
    }
    
    void WrappedObject::relocate()
    {
        HeapCellRelocate(reinterpret_cast<js::gc::Cell**>(&value));
        endTracing();
        
        LOGD_IF(LOG_VERBOSE) << __PRETTY_FUNCTION__ << " " << this << " | value: " << JSP::writeDetailed(value) << endl;
    }
    
    // ---
    
    void WrappedObject::beginTracing()
    {
        assert(!traced); // TODO: FOLLOW-UP
        
        traced = true;
        traceCount = 0;
        
        addTracer(this, bind(&WrappedObject::trace, this, std::placeholders::_1));
    }
    
    void WrappedObject::endTracing()
    {
        assert(traced); // TODO: FOLLOW-UP
        
        traced = false;
//      traceCount = 0;
        
        removeTracer(this);
    }
    
    void WrappedObject::trace(JSTracer *trc)
    {
        JS_CallObjectTracer(trc, const_cast<JSObject**>(&value), "WrappedObject");
        traceCount++;
        
        /*
         * MUST TAKE PLACE AFTER JS_CallObjectTracer
         */
        LOGD_IF(LOG_VERBOSE) << __PRETTY_FUNCTION__ << " " << this << " | value: " << JSP::writeDetailed(value) << endl;
    }
}

// ---

/*
 * REFERENCE FOR SOME OF THE FOLLOWING:
 *
 * https://github.com/mozilla/gecko-dev/blob/esr31/js/public/RootingAPI.h#L865-881
 * https://github.com/mozilla/gecko-dev/blob/esr31/js/src/jsobj.h#L1164-1171
 *
 *
 * UNTESTED ALTERNATIVES:
 *
 * AUGMENTING HandleBase<WrappedObject> WITH IMPLICIT CONSTRUCTION, AS IN:
 * https://github.com/mozilla/gecko-dev/blob/esr31/js/public/RootingAPI.h#L453-471
 */

using namespace JS;

namespace js
{
    HeapBase<WrappedObject>::operator Handle<JSObject*> () const
    {
        const Heap<WrappedObject> &self = *static_cast<const Heap<WrappedObject>*>(this);
        return Handle<JSObject*>::fromMarkedLocation(reinterpret_cast<JSObject* const*>(self.address()));
    }
    
    HeapBase<WrappedObject>::operator Handle<WrappedObject> () const
    {
        const Heap<WrappedObject> &self = *static_cast<const Heap<WrappedObject>*>(this);
        return Handle<WrappedObject>::fromMarkedLocation(reinterpret_cast<WrappedObject const*>(self.address()));
    }
    
    HeapBase<WrappedObject>::operator MutableHandle<JSObject*> ()
    {
        Heap<WrappedObject> &self = *static_cast<Heap<WrappedObject>*>(this);
        return MutableHandle<JSObject*>::fromMarkedLocation(reinterpret_cast<JSObject**>(self.unsafeGet()));
    }
    
    HeapBase<WrappedObject>::operator MutableHandle<WrappedObject> ()
    {
        Heap<WrappedObject> &self = *static_cast<Heap<WrappedObject>*>(this);
        return MutableHandle<WrappedObject>::fromMarkedLocation(reinterpret_cast<WrappedObject*>(self.unsafeGet()));
    }
    
    HeapBase<WrappedObject>::operator Handle<Value> () const
    {
        const Heap<WrappedObject> &self = *static_cast<const Heap<WrappedObject>*>(this);
        return Handle<Value>::fromMarkedLocation(reinterpret_cast<Value const*>(self.address()));
    }
    
    HeapBase<WrappedObject>::operator MutableHandle<Value> ()
    {
        Heap<WrappedObject> &self = *static_cast<Heap<WrappedObject>*>(this);
        return MutableHandle<Value>::fromMarkedLocation(reinterpret_cast<Value*>(self.unsafeGet()));
    }
    
    HeapBase<WrappedObject>::operator const bool () const
    {
        const Heap<WrappedObject> &self = *static_cast<const Heap<WrappedObject>*>(this);
        return self.get();
    }
}
