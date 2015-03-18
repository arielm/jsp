/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "WrappedValue.h"

#include "chronotext/Log.h"

using namespace std;
using namespace chr;

namespace jsp
{
    bool WrappedValue::LOG_VERBOSE = false;

    WrappedValue::WrappedValue()
    :
    WrappedValue(UndefinedValue())
    {}
    
    WrappedValue::~WrappedValue()
    {
        assert(!traced); // TODO: FOLLOW-UP
        
        LOGD_IF(LOG_VERBOSE) << __PRETTY_FUNCTION__ << " " << this << endl;
    }
    
    WrappedValue::WrappedValue(const Value &v)
    :
    value(v),
    traced(false),
    traceCount(0)
    {
        LOGD_IF(LOG_VERBOSE) << __PRETTY_FUNCTION__ << " " << this << " | value: " << JSP::writeDetailed(value) << endl;
    }
    
    WrappedValue& WrappedValue::operator=(const Value &v)
    {
        set(v);
        return *this;
    }
    
    WrappedValue::WrappedValue(const WrappedValue &other)
    :
    WrappedValue(other.value)
    {}
    
    void WrappedValue::operator=(const WrappedValue &other)
    {
        set(other.value);
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
    
    bool WrappedValue::operator==(float f) const
    {
        return compare(value, f);
    }
    
    bool WrappedValue::operator!=(float f) const
    {
        return !compare(value, f);
    }
    
    bool WrappedValue::operator==(double d) const
    {
        return compare(value, d);
    }
    
    bool WrappedValue::operator!=(double d) const
    {
        return !compare(value, d);
    }
    
    bool WrappedValue::operator==(int32_t i) const
    {
        return compare(value, i);
    }
    
    bool WrappedValue::operator!=(int32_t i) const
    {
        return !compare(value, i);
    }
    
    bool WrappedValue::operator==(uint32_t ui) const
    {
        return compare(value, ui);
    }
    
    bool WrappedValue::operator!=(uint32_t ui) const
    {
        return !compare(value, ui);
    }
    
    bool WrappedValue::operator==(bool b) const
    {
        RootedValue rooted(cx, value);
        return ToBoolean(rooted) == b;
    }
    
    bool WrappedValue::operator!=(bool b) const
    {
        RootedValue rooted(cx, value);
        return ToBoolean(rooted) != b;
    }
    
    bool WrappedValue::operator==(const char *s) const
    {
        RootedValue rooted(cx, value);
        return compare(rooted, s);
    }
    
    bool WrappedValue::operator!=(const char *s) const
    {
        RootedValue rooted(cx, value);
        return !compare(rooted, s);
    }
    
    // ---
    
    void WrappedValue::set(const Value &v)
    {
        if (v != value)
        {
            if (traced)
            {
                endTracing();
            }
            
            value = v;
            
            LOGD_IF(LOG_VERBOSE) << __PRETTY_FUNCTION__ << " " << this << " | value: " << JSP::writeDetailed(value) << endl;
        }
    }
    
    void WrappedValue::reset()
    {
        set(UndefinedValue());
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
        beginTracing();
        HeapValuePostBarrier(&value);
        
        LOGD_IF(LOG_VERBOSE) << __PRETTY_FUNCTION__ << " " << this << " | value: " << JSP::writeDetailed(value) << endl;
    }
    
    void WrappedValue::relocate()
    {
        HeapValueRelocate(&value);
        endTracing();
        
        LOGD_IF(LOG_VERBOSE) << __PRETTY_FUNCTION__ << " " << this << " | value: " << JSP::writeDetailed(value) << endl;
    }
    
    // ---
    
    void WrappedValue::beginTracing()
    {
        assert(!traced && value.isMarkable()); // TODO: FOLLOW-UP
        
        traced = true;
        traceCount = 0;
        
        addTracer(this, bind(&WrappedValue::trace, this, std::placeholders::_1));
    }
    
    void WrappedValue::endTracing()
    {
        assert(traced); // TODO: FOLLOW-UP
        
        traced = false;
//      traceCount = 0;
        
        removeTracer(this);
    }
    
    void WrappedValue::trace(JSTracer *trc)
    {
        JS_CallValueTracer(trc, &value, "WrappedValue");
        traceCount++;
        
        /*
         * MUST TAKE PLACE AFTER JS_CallValueTracer
         */
        LOGD_IF(LOG_VERBOSE) << __PRETTY_FUNCTION__ << " " << this << " | value: " << JSP::writeDetailed(value) << endl;
    }
}
