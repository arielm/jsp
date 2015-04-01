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
        endTracing();
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
        addTracerCallback(this, BIND_INSTANCE1(&WrappedObject::trace, this));
        HeapCellPostBarrier(reinterpret_cast<js::gc::Cell**>(&value));
        
        dump(__PRETTY_FUNCTION__);
    }
    
    void WrappedObject::relocate()
    {
        HeapCellRelocate(reinterpret_cast<js::gc::Cell**>(&value));
        endTracing();
        
        dump(__PRETTY_FUNCTION__);
    }
    
    // ---

    void WrappedObject::endTracing()
    {
        removeTracerCallback(this);
    }
    
    void WrappedObject::trace(JSTracer *trc)
    {
        JS_CallObjectTracer(trc, const_cast<JSObject**>(&value), "WrappedObject");
        
        /*
         * MUST TAKE PLACE AFTER JS_CallObjectTracer
         */
        dump(__PRETTY_FUNCTION__);
    }
}
