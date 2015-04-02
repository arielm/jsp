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
        DUMP_WRAPPED_OBJECT
    }
    
    WrappedObject::~WrappedObject()
    {
        heapTraced.erase(this);
        DUMP_WRAPPED_OBJECT
    }
    
    WrappedObject::WrappedObject(JSObject *o)
    :
    object(o)
    {
        DUMP_WRAPPED_OBJECT
    }
    
    WrappedObject& WrappedObject::operator=(JSObject *newObject)
    {
        set(newObject);
        DUMP_WRAPPED_OBJECT
        
        return *this;
    }
    
    WrappedObject::WrappedObject(const HandleObject &handle)
    :
    object(handle.get())
    {
        DUMP_WRAPPED_OBJECT
    }
    
    WrappedObject& WrappedObject::operator=(const HandleObject &handle)
    {
        set(handle.get());
        DUMP_WRAPPED_OBJECT
        
        return *this;
    }
    
    WrappedObject::WrappedObject(const WrappedObject &other)
    :
    object(other.object)
    {
        DUMP_WRAPPED_OBJECT
    }
    
    void WrappedObject::operator=(const WrappedObject &other)
    {
        set(other.object);
        DUMP_WRAPPED_OBJECT
    }
    
    void WrappedObject::set(JSObject *newObject)
    {
        if (newObject)
        {
            object = newObject;
            
            if (heapTraced.count(this))
            {
                beginTracing();
            }
        }
        else if (object)
        {
            if (heapTraced.count(this))
            {
                endTracing();
            }
            
            object = newObject;
        }
        else
        {
            object = newObject;
        }
    }
    
    void WrappedObject::clear()
    {
        set(nullptr);
        DUMP_WRAPPED_OBJECT
    }
    
    // ---
    
    void WrappedObject::postBarrier()
    {
        heapTraced.insert(this);
        beginTracing();
        
        DUMP_WRAPPED_OBJECT
    }
    
    void WrappedObject::relocate()
    {
        heapTraced.erase(this);
        endTracing();
        
        DUMP_WRAPPED_OBJECT
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
        DUMP_WRAPPED_OBJECT
    }
    
    // ---
    
    void WrappedObject::dump(const char *prefix)
    {
        LOGD << prefix << " " << this << " | object: " << JSP::writeDetailed(object) << endl;
    }
}
