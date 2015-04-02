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
    set<void*> WrappedObject::heapTraced;
    
    // ---

    WrappedObject::WrappedObject()
    :
    object(nullptr)
    {
        dump(__PRETTY_FUNCTION__);
    }
    
    WrappedObject::~WrappedObject()
    {
        heapTraced.erase(this);

        LOGD_IF(LOG_VERBOSE) << __PRETTY_FUNCTION__ << " " << this << endl;
    }
    
    WrappedObject::WrappedObject(JSObject *o)
    :
    object(o)
    {
        dump(__PRETTY_FUNCTION__);
    }
    
    WrappedObject& WrappedObject::operator=(JSObject *newObject)
    {
        set(newObject);
        dump(__PRETTY_FUNCTION__);
        
        return *this;
    }
    
    WrappedObject::WrappedObject(const HandleObject &handle)
    :
    object(handle.get())
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
    object(other.object)
    {
        dump(__PRETTY_FUNCTION__);
    }
    
    void WrappedObject::operator=(const WrappedObject &other)
    {
        set(other.object);
        dump(__PRETTY_FUNCTION__);
    }
    
    void WrappedObject::set(JSObject *newObject)
    {
        if (heapTraced.count(this))
        {
            if (newObject)
            {
                object = newObject;
                beginTracing();
                
                return;
            }
            
            if (object)
            {
                endTracing();
            }
        }
        
        object = newObject;
    }
    
    void WrappedObject::clear()
    {
        set(nullptr);
        dump(__PRETTY_FUNCTION__);
    }
    
    void WrappedObject::dump(const char *prefix)
    {
        LOGD_IF(LOG_VERBOSE) << prefix << " " << this << " | value: " << JSP::writeDetailed(object) << endl;
    }
    
    // ---
    
    void WrappedObject::postBarrier()
    {
        heapTraced.insert(this);
        beginTracing();
        
        dump(__PRETTY_FUNCTION__);
    }
    
    void WrappedObject::relocate()
    {
        heapTraced.erase(this);
        endTracing();
        
        dump(__PRETTY_FUNCTION__);
    }
    
    void WrappedObject::beginTracing()
    {
        addTracerCallback(this, BIND_INSTANCE1(&WrappedObject::trace, this));
        HeapCellPostBarrier(reinterpret_cast<js::gc::Cell**>(&object));
    }
    
    void WrappedObject::endTracing()
    {
        HeapCellRelocate(reinterpret_cast<js::gc::Cell**>(&object));
        removeTracerCallback(this);
    }
    
    void WrappedObject::trace(JSTracer *trc)
    {
        JS_CallObjectTracer(trc, const_cast<JSObject**>(&object), "WrappedObject");
        
        /*
         * MUST TAKE PLACE AFTER JS_CallObjectTracer
         */
        dump(__PRETTY_FUNCTION__);
    }
}
