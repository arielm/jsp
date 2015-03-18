/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#pragma once

#include "Context.h"

#include "chronotext/Exception.h"

#include "cinder/DataSource.h"
#include "cinder/DataTarget.h"

namespace jsp
{
    class CloneBuffer
    {
    public:
        static bool DUMP_UNSUPPORTED_OBJECTS;
        static bool DUMP_UNSUPPORTED_FUNCTIONS;
        
        static JSObject* read(ci::DataSourceRef source);
        static size_t write(JSObject *object, ci::DataTargetRef target);
        
        CloneBuffer(JSObject *object);
        CloneBuffer(ci::DataSourceRef source);
        
        JSObject* read();
        size_t write(ci::DataTargetRef target);
        
    protected:
        ci::Buffer buffer;
        uint32_t unsupportedIndex;
        
        JSObject* deserialize();
        void serialize(JSObject *object);
        
        static JSObject* readOp(JSContext *cx, JSStructuredCloneReader *r, uint32_t tag, uint32_t data, void *closure);
        static bool writeOp(JSContext *cx, JSStructuredCloneWriter *w, HandleObject obj, void *closure);
        static void reportOp(JSContext *cx, uint32_t errorid);
    };
}
