/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "jsp/WrappedObject.h"

#include "chronotext/Log.h"

using namespace std;
using namespace chr;

namespace jsp
{
    bool WrappedObject::LOG_VERBOSE = false;

    WrappedObject::WrappedObject()
    :
    value(nullptr)
    {
        dump(__PRETTY_FUNCTION__);
    }
    
    WrappedObject::~WrappedObject()
    {
        assert(!traced); // TODO: FOLLOW-UP
        
        LOGD_IF(LOG_VERBOSE) << __PRETTY_FUNCTION__ << " " << this << endl;
    }
    
    WrappedObject::WrappedObject(JSObject *object)
    :
    value(object)
    {
        dump(__PRETTY_FUNCTION__);
    }
    
    WrappedObject& WrappedObject::operator=(JSObject *object)
    {
        set(object);
        dump(__PRETTY_FUNCTION__);
        
        return *this;
    }
    
    WrappedObject::WrappedObject(const HandleObject &handle)
    :
    value(handle.get())
    {
        dump(__PRETTY_FUNCTION__);
    }
    
    WrappedObject& WrappedObject::operator=(const HandleObject &handle)
    {
        set(handle.get());
        dump(__PRETTY_FUNCTION__);
        
        return *this;
    }
    
    WrappedObject::WrappedObject(const WrappedObject &other)
    :
    value(other.value)
    {
        dump(__PRETTY_FUNCTION__);
    }
    
    void WrappedObject::operator=(const WrappedObject &other)
    {
        set(other.value);
        dump(__PRETTY_FUNCTION__);
    }
    
    void WrappedObject::set(JSObject *object)
    {
        if (traced)
        {
            endTracing();
        }
        
        value = object;
    }
    
    void WrappedObject::dump(const char *prefix)
    {
        LOGD_IF(LOG_VERBOSE) << prefix << " " << this << " | value: " << JSP::writeDetailed(value) << endl;
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
        addTracerCallback(this, BIND_INSTANCE1(&WrappedObject::trace, this));
    }
    
    void WrappedObject::endTracing()
    {
        assert(traced); // TODO: FOLLOW-UP
        
        traced = false;
        removeTracerCallback(this);
    }
    
    void WrappedObject::trace(JSTracer *trc)
    {
        JS_CallObjectTracer(trc, const_cast<JSObject**>(&value), "WrappedObject");
        
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
