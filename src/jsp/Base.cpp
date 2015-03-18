/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "Base.h"

using namespace std;
using namespace chr;

namespace jsp
{
#pragma mark ---------------------------------------- EVALUATION ----------------------------------------
    
    bool Base::exec(const string &source, const ReadOnlyCompileOptions &options)
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
    
    bool Base::eval(const std::string &source, const ReadOnlyCompileOptions &options, MutableHandleValue result)
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
    
    Value Base::callFunction(HandleObject object, const char *name, const HandleValueArray& args)
    {
        RootedValue result(cx);
        bool success = JS_CallFunctionName(cx, object, name, args, &result);
        
        if (JS_IsExceptionPending(cx))
        {
            JS_ReportPendingException(cx);
            JS_ClearPendingException(cx);
        }
        
        if (success)
        {
            return result;
        }
        
        throw EXCEPTION(Base, "FUNCTION-CALL FAILED");
    }
    
    Value Base::callFunction(HandleObject object, HandleValue function, const HandleValueArray& args)
    {
        if (!isFunction(function))
        {
            throw EXCEPTION(Base, "INVALID FUNCTION");
        }
        
        RootedValue result(cx);
        bool success = JS_CallFunctionValue(cx, object, function, args, &result);
        
        if (JS_IsExceptionPending(cx))
        {
            JS_ReportPendingException(cx);
            JS_ClearPendingException(cx);
        }
        
        if (success)
        {
            return result;
        }
        
        throw EXCEPTION(Base, "FUNCTION-CALL FAILED");
    }
    
    Value Base::callFunction(HandleObject object, HandleFunction function, const HandleValueArray& args)
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
        
        throw EXCEPTION(Base, "FUNCTION-CALL FAILED");
    }

#pragma mark ---------------------------------------- CALLBACKS ----------------------------------------

    bool Base::invokeCallback(std::function<bool(JS::CallArgs args)> &fn, JS::CallArgs args)
    {
        return fn(args);
    }

#pragma mark ---------------------------------------- OBJECTS AND PROPERTIES ----------------------------------------
    
    JSObject* Base::newObject()
    {
        return JS_NewObject(cx, nullptr, NullPtr(), NullPtr());
    }

    bool Base::hasProperty(HandleObject object, const char *name)
    {
        if (object)
        {
            bool found;
            
            if (JS_HasProperty(cx, object, name, &found))
            {
                return found;
            }
        }
        
        return false;
    }
    
    bool Base::hasOwnProperty(HandleObject object, const char *name)
    {
        if (object)
        {
            bool found;
            
            if (JS_AlreadyHasOwnProperty(cx, object, name, &found))
            {
                return found;
            }
        }
        
        return false;
    }
    
    bool Base::getProperty(HandleObject object, const char *name, MutableHandleValue result)
    {
        if (object)
        {
            return JS_GetProperty(cx, object, name, result);
        }
        
        return false;
    }
    
    bool Base::setProperty(HandleObject object, const char *name, HandleValue value)
    {
        if (object)
        {
            return JS_SetProperty(cx, object, name, value);
        }
        
        return false;
    }
    
    bool Base::deleteProperty(HandleObject object, const char *name)
    {
        if (object)
        {
            return JS_DeleteProperty(cx, object, name);
        }
        
        return false;
    }
    
#pragma mark ---------------------------------------- ARRAYS AND ELEMENTS ----------------------------------------
    
    JSObject* Base::newArray(size_t length)
    {
        return JS_NewArrayObject(cx, length);
    }
    
    JSObject* Base::newArray(const HandleValueArray& contents)
    {
        return JS_NewArrayObject(cx, contents);
    }
    
    uint32_t Base::getLength(HandleObject array)
    {
        if (array)
        {
            uint32_t length;
            
            if (JS_GetArrayLength(cx, array, &length))
            {
                return length;
            }
        }
        
        return 0;
    }
    
    bool Base::getElement(HandleObject array, uint32_t index, MutableHandleValue result)
    {
        if (array)
        {
            return JS_GetElement(cx, array, index, result);
        }
        
        return false;
    }
    
    bool Base::setElement(HandleObject array, uint32_t index, HandleValue value)
    {
        if (array)
        {
            return JS_SetElement(cx, array, index, value);
        }
        
        return false;
    }
    
    bool Base::deleteElement(HandleObject array, uint32_t index)
    {
        if (array)
        {
            return JS_DeleteElement(cx, array, index);
        }
        
        return false;
    }

}
