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
        assert(!traced); // TODO: FOLLOW-UP
        
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
        set(v);
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
        set(other.value);
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
        if (traced)
        {
            endTracing();
        }
        
        value = v;
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
        beginTracing();
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
    
    void WrappedValue::beginTracing()
    {
        assert(!traced && value.isMarkable()); // TODO: FOLLOW-UP
        
        traced = true;
        addTracer(this, bind(&WrappedValue::trace, this, placeholders::_1));
    }
    
    void WrappedValue::endTracing()
    {
        assert(traced); // TODO: FOLLOW-UP
        
        traced = false;
        removeTracer(this);
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
