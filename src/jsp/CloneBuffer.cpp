/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "jsp/CloneBuffer.h"

#include "js/StructuredClone.h"

#include "chronotext/Log.h"

using namespace std;
using namespace ci;
using namespace chr;

namespace jsp
{
    bool CloneBuffer::DUMP_UNSUPPORTED_OBJECTS = false;
    bool CloneBuffer::DUMP_UNSUPPORTED_FUNCTIONS = false;

    JSObject* CloneBuffer::read(DataSourceRef source)
    {
        CloneBuffer tmp(source);
        return tmp.read();
    }
    
    size_t CloneBuffer::write(JSObject *object, DataTargetRef target)
    {
        CloneBuffer tmp(object);
        return tmp.write(target);
    }
    
    CloneBuffer::CloneBuffer(JSObject *object)
    {
        serialize(object);
    }
    
    CloneBuffer::CloneBuffer(DataSourceRef source)
    :
    buffer(source)
    {}
    
    JSObject* CloneBuffer::read()
    {
        return deserialize();
    }
    
    size_t CloneBuffer::write(DataTargetRef target)
    {
        if (buffer.getDataSize() > 0)
        {
            buffer.write(target);
        }
        
        return buffer.getDataSize();
    }
    
    JSObject* CloneBuffer::deserialize()
    {
        if (buffer.getDataSize() > 0)
        {
            JSStructuredCloneCallbacks callbacks;
            callbacks.read = CloneBuffer::readOp;
            callbacks.reportError = CloneBuffer::reportOp; // XXX: CAN'T REPRODUCE USAGE
            
            RootedValue out(cx);
            
            if (JS_ReadStructuredClone(cx, (uint64_t*)buffer.getData(), buffer.getDataSize(), JS_STRUCTURED_CLONE_VERSION, &out, &callbacks, this))
            {
                if (!out.isNullOrUndefined())
                {
                    return out.toObjectOrNull();
                }
            }
        }
        
        throw EXCEPTION(CloneBuffer, "DESERIALIZATION FAILED");
    }
    
    void CloneBuffer::serialize(JSObject *object)
    {
        if (object)
        {
            auto value = ObjectOrNullValue(object);
            
            if (value.isObject())
            {
                unsupportedIndex = 0;
                
                JSStructuredCloneCallbacks callbacks;
                callbacks.write = CloneBuffer::writeOp;
                callbacks.reportError = CloneBuffer::reportOp; // XXX: CAN'T REPRODUCE USAGE
                
                RootedValue in(cx, value);
                
                uint64_t *datap;
                size_t nbytes;
                
                if (JS_WriteStructuredClone(cx, in, &datap, &nbytes, &callbacks, this, UndefinedHandleValue))
                {
                    buffer = Buffer(nbytes);
                    buffer.copyFrom(datap, nbytes); // TODO: COPY ONLY WHEN NECESSARY
                    
                    JS_ClearStructuredClone(datap, nbytes, nullptr, nullptr);
                    return;
                }
            }
        }
        
        throw EXCEPTION(CloneBuffer, "SERIALIZATION FAILED");
    }
    
    JSObject* CloneBuffer::readOp(JSContext *cx, JSStructuredCloneReader *r, uint32_t tag, uint32_t data, void *closure)
    {
        auto unsupportedIndex = data;
        
        LOGD_IF(DUMP_UNSUPPORTED_OBJECTS) << "UNSUPPORTED OBJECT: " << unsupportedIndex << endl;
        
        RootedObject dummyObject(cx, JS_NewObject(cx, nullptr, NullPtr(), NullPtr()));
        RootedValue dummyValue(cx, NumberValue(unsupportedIndex));
        JS_SetProperty(cx, dummyObject, "unsupported-index", dummyValue);
        
        return dummyObject;
    }
    
    bool CloneBuffer::writeOp(JSContext *cx, JSStructuredCloneWriter *w, HandleObject obj, void *closure)
    {
        auto self = reinterpret_cast<CloneBuffer*>(closure);
        auto unsupportedIndex = self->unsupportedIndex;
        
        if (DUMP_UNSUPPORTED_OBJECTS)
        {
            LOGD << "UNSUPPORTED OBJECT: " << unsupportedIndex << endl;
            
            JSP::dumpObject(obj);
            
            if (DUMP_UNSUPPORTED_FUNCTIONS)
            {
                if (JS_ObjectIsFunction(cx, obj))
                {
                    LOGD << JSP::toSource(obj) << endl << endl;
                }
            }
        }
        
        /*
         * IN ORDER TO AVOID FAILURE UPON DESERIALIZATION, WE MUST FULLFILL THE CONTRACT:
         * https://github.com/mozilla/gecko-dev/blob/esr31/js/public/StructuredClone.h#L65-75
         */
        JS_WriteUint32Pair(w, JS_SCTAG_USER_MIN + 1, unsupportedIndex);
        self->unsupportedIndex++;
        
        return true;
    }
    
    void CloneBuffer::reportOp(JSContext *cx, uint32_t errorid)
    {}
}
