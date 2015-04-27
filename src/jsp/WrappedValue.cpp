/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "jsp/WrappedValue.h"

#include "chronotext/Log.h"

using namespace std;
using namespace chr;

namespace jsp
{
    bool WrappedValue::LOG_VERBOSE = false;

    // ---
    
    WrappedValue::WrappedValue()
    :
    value(UndefinedValue())
    {
        DUMP_WRAPPED_VALUE
    }
    
    WrappedValue::~WrappedValue()
    {
        DUMP_WRAPPED_VALUE
    }
    
    WrappedValue::WrappedValue(const WrappedValue &other)
    :
    value(other.value)
    {
        DUMP_WRAPPED_VALUE
    }
    
    void WrappedValue::operator=(const WrappedValue &other)
    {
        value = other.value;
        DUMP_WRAPPED_VALUE
    }
    
    WrappedValue::WrappedValue(const Value &v)
    :
    value(v)
    {
        DUMP_WRAPPED_VALUE
    }
    
    WrappedValue& WrappedValue::operator=(const Value &newValue)
    {
        value = newValue;
        DUMP_WRAPPED_VALUE

        return *this;
    }
    
    WrappedValue::operator const bool () const
    {
        RootedValue rooted(cx, value);
        return ToBoolean(rooted);
    }
    
    // ---
    
    bool WrappedValue::operator==(const WrappedValue &other) const
    {
        return compare(value, other.value);
    }
    
    bool WrappedValue::operator!=(const WrappedValue &other) const
    {
        return !compare(value, other.value);
    }
    
    bool WrappedValue::operator==(const Value &other) const
    {
        return compare(value, other);
    }
    
    bool WrappedValue::operator!=(const Value &other) const
    {
        return !compare(value, other);
    }
    
    // ---
    
    void WrappedValue::postBarrier()
    {
        JSP::addTracerCallback(this, BIND_INSTANCE1(&WrappedValue::trace, this));
        HeapValuePostBarrier(&value);
        
        DUMP_WRAPPED_VALUE
    }
    
    void WrappedValue::relocate()
    {
        JSP::removeTracerCallback(this);
        HeapValueRelocate(&value);
        
        DUMP_WRAPPED_VALUE
    }
    
    void WrappedValue::trace(JSTracer *trc)
    {
        JS_CallValueTracer(trc, &value, "WrappedValue");
        
        /*
         * MUST TAKE PLACE AFTER JS_CallValueTracer
         */
        DUMP_WRAPPED_VALUE
    }
    
    // ---
    
    void WrappedValue::dump(const char *prefix)
    {
        LOGD << prefix << " " << this << " | value: " << JSP::writeDetailed(value) << endl;
    }
}
