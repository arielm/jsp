/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "jsp/BaseProto.h"

using namespace std;
using namespace chr;

namespace jsp
{
    BaseProto* BaseProto::target()
    {
        static shared_ptr<BaseProto> target;

        if (!target)
        {
            target = shared_ptr<BaseProto>(new BaseProto);
        }
        
        return target.get();
    }

#pragma mark ---------------------------------------- EVALUATION ----------------------------------------
    
    bool BaseProto::exec(const string &source, const ReadOnlyCompileOptions &options)
    {
        RootedValue result(cx);
        bool success = Evaluate(cx, globalHandle(), options, source.data(), source.size(), &result);
        
        if (JS_IsExceptionPending(cx))
        {
            JS_ReportPendingException(cx);
            JS_ClearPendingException(cx);
        }
        
        return success;
    }
    
    bool BaseProto::eval(const std::string &source, const ReadOnlyCompileOptions &options, MutableHandleValue result)
    {
        bool success = Evaluate(cx, globalHandle(), options, source.data(), source.size(), result);
        
        if (JS_IsExceptionPending(cx))
        {
            JS_ReportPendingException(cx);
            JS_ClearPendingException(cx);
        }
        
        return success;
    }
    
#pragma mark ---------------------------------------- FUNCTIONS ----------------------------------------
    
    Value BaseProto::call(HandleObject object, const char *functionName, const HandleValueArray& args)
    {
        RootedValue result(cx);
        bool success = JS_CallFunctionName(cx, object, functionName, args, &result);
        
        if (JS_IsExceptionPending(cx))
        {
            JS_ReportPendingException(cx);
            JS_ClearPendingException(cx);
        }
        
        if (success)
        {
            return result;
        }
        
        throw EXCEPTION(BaseProto, "FUNCTION-CALL FAILED");
    }
    
    Value BaseProto::call(HandleObject object, HandleValue functionValue, const HandleValueArray& args)
    {
        RootedValue result(cx);
        bool success = JS_CallFunctionValue(cx, object, functionValue, args, &result);
        
        if (JS_IsExceptionPending(cx))
        {
            JS_ReportPendingException(cx);
            JS_ClearPendingException(cx);
        }
        
        if (success)
        {
            return result;
        }
        
        throw EXCEPTION(BaseProto, "FUNCTION-CALL FAILED");
    }
    
    Value BaseProto::call(HandleObject object, HandleFunction function, const HandleValueArray& args)
    {
        RootedValue result(cx);
        bool success = JS_CallFunction(cx, object, function, args, &result);
        
        if (JS_IsExceptionPending(cx))
        {
            JS_ReportPendingException(cx);
            JS_ClearPendingException(cx);
        }
        
        if (success)
        {
            return result;
        }
        
        throw EXCEPTION(BaseProto, "FUNCTION-CALL FAILED");
    }
}
