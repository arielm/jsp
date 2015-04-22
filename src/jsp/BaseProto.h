/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#pragma once

#include "jsp/Proto.h"

#include "chronotext/Exception.h"

namespace jsp
{
    class BaseProto : public Proto
    {
    public:
        static BaseProto* target();

        /*
         * EVALUATION
         */
        
        bool exec(const std::string &source, const ReadOnlyCompileOptions &options) override;
        bool eval(const std::string &source, const ReadOnlyCompileOptions &options, MutableHandleValue result) override;
        
        /*
         * FUNCTIONS
         */
        
        Value call(HandleObject object, const char *functionName, const HandleValueArray& args = HandleValueArray::empty()) override;
        Value call(HandleObject object, HandleValue functionValue, const HandleValueArray& args = HandleValueArray::empty()) override;
        Value call(HandleObject object, HandleFunction function, const HandleValueArray& args = HandleValueArray::empty()) override;
        
        /*
         * NATIVE CALLS
         */
        
        inline bool apply(const NativeCall &nativeCall, CallArgs args) override
        {
            return nativeCall.fn(args);
        }
        
    private:
        BaseProto() = default;
        BaseProto(const BaseProto &other) = delete;
    };
}
