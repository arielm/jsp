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
    
    // ---

    WrappedObject::WrappedObject()
    :
    object(nullptr)
    {
        DUMP_WRAPPED_OBJECT
    }
    
    WrappedObject::~WrappedObject()
    {
        DUMP_WRAPPED_OBJECT
    }
    
    WrappedObject::WrappedObject(const WrappedObject &other)
    :
    object(other.object)
    {
        DUMP_WRAPPED_OBJECT
    }
    
    void WrappedObject::operator=(const WrappedObject &other)
    {
        object = other.object;
        DUMP_WRAPPED_OBJECT
    }
    
    WrappedObject::WrappedObject(const HandleObject &handle)
    :
    object(handle.get())
    {
        DUMP_WRAPPED_OBJECT
    }
    
    WrappedObject& WrappedObject::operator=(const HandleObject &handle)
    {
        object = handle.get();
        DUMP_WRAPPED_OBJECT
        
        return *this;
    }
    
    WrappedObject::WrappedObject(JSObject *o)
    :
    object(o)
    {
        DUMP_WRAPPED_OBJECT
    }
    
    WrappedObject& WrappedObject::operator=(JSObject *newObject)
    {
        object = newObject;
        DUMP_WRAPPED_OBJECT
        
        return *this;
    }
    
    // ---
    
    void WrappedObject::postBarrier()
    {
        JSP::addTracerCallback(this, BIND_INSTANCE1(&WrappedObject::trace, this));
        HeapCellPostBarrier(reinterpret_cast<js::gc::Cell**>(&object));
        
        DUMP_WRAPPED_OBJECT
    }
    
    void WrappedObject::relocate()
    {
        HeapCellRelocate(reinterpret_cast<js::gc::Cell**>(&object));
        JSP::removeTracerCallback(this);
        
        DUMP_WRAPPED_OBJECT
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
