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

#pragma mark ---------------------------------------- OBJECTS AND PROPERTIES ----------------------------------------
    
    JSObject* BaseProto::newPlainObject()
    {
        return JS_NewObject(cx, nullptr, NullPtr(), NullPtr());
    }
    
    /*
     * REFERENCES:
     *
     * - https://github.com/mozilla/gecko-dev/blob/esr31/js/src/vm/Stack.h#L1100-1113
     * - https://github.com/mozilla/gecko-dev/blob/esr31/js/src/vm/Interpreter.cpp#L504-510
     * - https://github.com/mozilla/gecko-dev/blob/esr31/js/src/vm/Interpreter.cpp#L543-572
     */
    JSObject* BaseProto::newNativeObject(const std::string &className, const HandleValueArray& args)
    {
        RootedValue value(cx);
        
        if (JS_GetProperty(cx, globalHandle(), className.data(), &value))
        {
            if (!value.isUndefined() && value.isObject())
            {
                JSObject &object = value.toObject();
                
                Value values[args.length() + 2];
                CallArgs callArgs = CallArgsFromVp(args.length(), values);
                mozilla::PodCopy(callArgs.array(), args.begin(), args.length());

                callArgs.setCallee(value);
                callArgs.setThis(MagicValue(JS_IS_CONSTRUCTING));
                
                if (object.is<JSFunction>())
                {
                    RootedFunction function(cx, &object.as<JSFunction>());
                    
                    if (function->isNativeConstructor())
                    {
                        if (js::CallJSNative(cx, function->native(), callArgs))
                        {
                            return callArgs.rval().toObjectOrNull();
                        }
                    }
                }
                else
                {
                    RootedObject constructor(cx, &object);
                    return JS_New(cx, constructor, callArgs);
                }
            }
        }

        return nullptr;
    }

    bool BaseProto::hasProperty(HandleObject object, const char *name)
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
    
    bool BaseProto::hasOwnProperty(HandleObject object, const char *name)
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
    
    bool BaseProto::getProperty(HandleObject object, const char *name, MutableHandleValue result)
    {
        if (object)
        {
            return JS_GetProperty(cx, object, name, result);
        }
        
        return false;
    }
    
    bool BaseProto::setProperty(HandleObject object, const char *name, HandleValue value)
    {
        if (object)
        {
            return JS_SetProperty(cx, object, name, value);
        }
        
        return false;
    }
    
    bool BaseProto::deleteProperty(HandleObject object, const char *name)
    {
        if (object)
        {
            bool success;
            
            if (JS_DeleteProperty2(cx, object, name, &success))
            {
                return success;
            }
        }
        
        return false;
    }
    
#pragma mark ---------------------------------------- ARRAYS AND ELEMENTS ----------------------------------------
    
    JSObject* BaseProto::newArray(size_t length)
    {
        return JS_NewArrayObject(cx, length);
    }
    
    JSObject* BaseProto::newArray(const HandleValueArray& contents)
    {
        return JS_NewArrayObject(cx, contents);
    }
    
    uint32_t BaseProto::getLength(HandleObject array)
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
    
    bool BaseProto::getElement(HandleObject array, uint32_t index, MutableHandleValue result)
    {
        if (array)
        {
            return JS_GetElement(cx, array, index, result);
        }
        
        return false;
    }
    
    bool BaseProto::setElement(HandleObject array, uint32_t index, HandleValue value)
    {
        if (array)
        {
            return JS_SetElement(cx, array, index, value);
        }
        
        return false;
    }
    
    bool BaseProto::deleteElement(HandleObject array, uint32_t index)
    {
        if (array)
        {
            return JS_DeleteElement(cx, array, index);
        }
        
        return false;
    }
}
