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

    WrappedValue::WrappedValue()
    :
    value(UndefinedValue())
    {
        dump(__PRETTY_FUNCTION__);
    }
    
    WrappedValue::~WrappedValue()
    {
        LOGD_IF(LOG_VERBOSE) << __PRETTY_FUNCTION__ << " " << this << endl;
    }
    
    WrappedValue::WrappedValue(const Value &v)
    :
    value(v)
    {
        dump(__PRETTY_FUNCTION__);
    }
    
    WrappedValue& WrappedValue::operator=(const Value &v)
    {
        endTracing();
        value = v;
        dump(__PRETTY_FUNCTION__);

        return *this;
    }
    
    WrappedValue::WrappedValue(const WrappedValue &other)
    :
    value(other.value)
    {
        dump(__PRETTY_FUNCTION__);
    }
    
    void WrappedValue::operator=(const WrappedValue &other)
    {
        endTracing();
        value = other.value;
        dump(__PRETTY_FUNCTION__);
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
    
    bool WrappedValue::operator==(const JSObject *other) const
    {
        return compare(value, other);
    }
    
    bool WrappedValue::operator!=(const JSObject *other) const
    {
        return !compare(value, other);
    }
    
    bool WrappedValue::operator==(float other) const
    {
        return compare(value, other);
    }
    
    bool WrappedValue::operator!=(float other) const
    {
        return !compare(value, other);
    }
    
    bool WrappedValue::operator==(double other) const
    {
        return compare(value, other);
    }
    
    bool WrappedValue::operator!=(double other) const
    {
        return !compare(value, other);
    }
    
    bool WrappedValue::operator==(int32_t other) const
    {
        return compare(value, other);
    }
    
    bool WrappedValue::operator!=(int32_t other) const
    {
        return !compare(value, other);
    }
    
    bool WrappedValue::operator==(uint32_t other) const
    {
        return compare(value, other);
    }
    
    bool WrappedValue::operator!=(uint32_t other) const
    {
        return !compare(value, other);
    }
    
    bool WrappedValue::operator==(bool other) const
    {
        RootedValue rooted(cx, value);
        return ToBoolean(rooted) == other;
    }
    
    bool WrappedValue::operator!=(bool other) const
    {
        RootedValue rooted(cx, value);
        return ToBoolean(rooted) != other;
    }
    
    bool WrappedValue::operator==(const char *other) const
    {
        RootedValue rooted(cx, value);
        return compare(rooted, other);
    }
    
    bool WrappedValue::operator!=(const char *other) const
    {
        RootedValue rooted(cx, value);
        return !compare(rooted, other);
    }
    
    // ---
    
    void WrappedValue::set(const Value &v)
    {
        endTracing();
        value = v;
        dump(__PRETTY_FUNCTION__);
    }
    
    void WrappedValue::dump(const char *prefix)
    {
        LOGD_IF(LOG_VERBOSE) << prefix << " " << this << " | value: " << JSP::writeDetailed(value) << endl;
    }
    
    // ---
    
    bool WrappedValue::poisoned() const
    {
        /*
         * THE JS::IsPoisonedValue() USUALLY INVOKED IN SPIDERMONKEY CODE IS DUMMY (ALWAYS RETURNS FALSE),
         * SO USING THE "REAL" JSP::isPoisoned() WOULD BE A WASTE OF CYCLES
         */
        return false;
    }
    
    bool WrappedValue::needsPostBarrier() const
    {
        return value.isMarkable();
    }
    
    void WrappedValue::postBarrier()
    {
        addTracerCallback(this, BIND_INSTANCE1(&WrappedValue::trace, this));
        HeapValuePostBarrier(&value);
        
        dump(__PRETTY_FUNCTION__);
    }
    
    void WrappedValue::relocate()
    {
        HeapValueRelocate(&value);
        endTracing();
        
        dump(__PRETTY_FUNCTION__);
    }
    
    // ---
    
    void WrappedValue::endTracing()
    {
        removeTracerCallback(this);
    }
    
    void WrappedValue::trace(JSTracer *trc)
    {
        JS_CallValueTracer(trc, &value, "WrappedValue");
        
        /*
         * MUST TAKE PLACE AFTER JS_CallValueTracer
         */
        dump(__PRETTY_FUNCTION__);
    }
}
