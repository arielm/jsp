/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "jsp/Proto.h"

#include "chronotext/utils/Utils.h"

using namespace std;
using namespace ci;
using namespace chr;

namespace jsp
{
    bool Proto::exec(const string &source, const ReadOnlyCompileOptions &options)
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
    
    void Proto::executeScript(const string &source, const string &file, int line)
    {
        OwningCompileOptions options(cx);
        options.setNoScriptRval(true); // TODO: SHOULD BE FORCED, INSIDE exec
        options.setVersion(JSVersion::JSVERSION_LATEST);
        options.setUTF8(true);
        options.setFileAndLine(cx, file.data(), line);
        
        // ---
        
        if (!exec(source, options))
        {
            throw EXCEPTION(Proto, "EXECUTION FAILED");
        }
    }
    
    void Proto::executeScript(InputSource::Ref inputSource)
    {
        executeScript(utils::readText<string>(inputSource), inputSource->getFilePathHint());
    }
    
    // ---
    
    bool Proto::eval(const string &source, const ReadOnlyCompileOptions &options, MutableHandleValue result)
    {
        bool success = Evaluate(cx, globalHandle(), options, source.data(), source.size(), result);
        
        if (JS_IsExceptionPending(cx))
        {
            JS_ReportPendingException(cx);
            JS_ClearPendingException(cx);
        }
        
        return success;
    }
    
    JSObject* Proto::evaluateObject(const string &source, const string &file, int line)
    {
        OwningCompileOptions options(cx);
        options.setForEval(true); // TODO: SHOULD BE FORCED, INSIDE eval
        options.setVersion(JSVersion::JSVERSION_LATEST);
        options.setUTF8(true);
        options.setFileAndLine(cx, file.data(), line);
        
        // ---
        
        RootedValue result(cx);
        
        if (eval(source, options, &result))
        {
            if (!result.isObject())
            {
                throw EXCEPTION(Proto, "EVALUATED VALUE IS NOT AN OBJECT");
            }
            
            return result.toObjectOrNull();
        }
        
        throw EXCEPTION(Proto, "EVALUATION FAILED");
    }
    
    JSObject* Proto::evaluateObject(InputSource::Ref inputSource)
    {
        return evaluateObject(utils::readText<string>(inputSource), inputSource->getFilePathHint());
    }
    
    // ---
    
    Value Proto::call(HandleObject object, const char *functionName, const HandleValueArray& args)
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
        
        throw EXCEPTION(Proto, "FUNCTION-CALL FAILED");
    }
    
    Value Proto::call(HandleObject object, HandleValue functionValue, const HandleValueArray& args)
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
        
        throw EXCEPTION(Proto, "FUNCTION-CALL FAILED");
    }
    
    Value Proto::call(HandleObject object, HandleFunction function, const HandleValueArray& args)
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
        
        throw EXCEPTION(Proto, "FUNCTION-CALL FAILED");
    }
    
    // ---
    
    JSObject* Proto::newPlainObject()
    {
        return JS_NewObject(cx, nullptr, NullPtr(), NullPtr());
    }
    
    JSObject* Proto::newObject(const string &className, const HandleValueArray& args)
    {
        RootedValue value(cx);
        
        if (JS_GetProperty(cx, globalHandle(), className.data(), &value))
        {
            if (value.isObject())
            {
                RootedObject constructor(cx, &value.toObject());
                
                return JS_New(cx, constructor, args);
            }
        }
        
        return nullptr;
    }
    
    bool Proto::hasProperty(HandleObject object, const char *name)
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
    
    bool Proto::hasOwnProperty(HandleObject object, const char *name)
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
    
    bool Proto::getProperty(HandleObject object, const char *name, MutableHandleValue result)
    {
        if (object)
        {
            return JS_GetProperty(cx, object, name, result);
        }
        
        return false;
    }
    
    bool Proto::setProperty(HandleObject object, const char *name, HandleValue value)
    {
        if (object)
        {
            return JS_SetProperty(cx, object, name, value);
        }
        
        return false;
    }
    
    bool Proto::defineProperty(HandleObject object, const char *name, HandleValue value, unsigned attrs)
    {
        if (object)
        {
            return JS_DefineProperty(cx, object, name, value, attrs);
        }
        
        return false;
    }
    
    bool Proto::deleteProperty(HandleObject object, const char *name)
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
    
    // ---
    
    JSObject* Proto::newArray(size_t length)
    {
        return JS_NewArrayObject(cx, length);
    }
    
    JSObject* Proto::newArray(const HandleValueArray& contents)
    {
        return JS_NewArrayObject(cx, contents);
    }
    
    bool Proto::hasElement(HandleObject array, int index)
    {
        if (array)
        {
            bool found;
            
            if (JS_HasElement(cx, array, index, &found))
            {
                return found;
            }
        }
        
        return false;
    }
    
    uint32_t Proto::getElementCount(HandleObject array)
    {
        uint32_t elementCount = 0;
        
        RootedValue iterable(cx, ObjectOrNullValue(array));
        ForOfIterator it(cx);
        
        if (it.init(iterable))
        {
            bool done = false;
            RootedValue value(cx);
            
            while (it.next(&value, &done) && !done)
            {
                if (!value.isUndefined())
                {
                    elementCount++;
                }
            }
        }
        
        return elementCount;
    }
    
    uint32_t Proto::getLength(HandleObject array)
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
    
    bool Proto::setLength(HandleObject array, size_t length)
    {
        if (array)
        {
            return JS_SetArrayLength(cx, array, length);
        }
        
        return false;
    }
    
    bool Proto::getElement(HandleObject array, int index, MutableHandleValue result)
    {
        if (array && (index >= 0))
        {
            return JS_GetElement(cx, array, index, result);
        }
        
        return false;
    }
    
    bool Proto::setElement(HandleObject array, int index, HandleValue value)
    {
        if (array && (index >= 0))
        {
            return JS_SetElement(cx, array, index, value);
        }
        
        return false;
    }
    
    bool Proto::defineElement(HandleObject array, int index, HandleValue value, unsigned attrs)
    {
        if (array && (index >= 0))
        {
            return JS_DefineElement(cx, array, index, value, nullptr, nullptr, attrs);
        }
        
        return false;
    }
    
    bool Proto::deleteElement(HandleObject array, int index)
    {
        if (array && (index >= 0))
        {
            bool success;
            
            if (JS_DeleteElement2(cx, array, index, &success))
            {
                return success;
            }
        }
        
        return false;
    }
    
    uint32_t Proto::getElements(HandleObject sourceArray, AutoValueVector &elements)
    {
        uint32_t getCount = 0;
        
        RootedValue iterable(cx, ObjectOrNullValue(sourceArray));
        ForOfIterator it(cx);
        
        if (it.init(iterable))
        {
            bool done = false;
            RootedValue value(cx);
            
            while (it.next(&value, &done) && !done)
            {
                if (elements.append(value))
                {
                    getCount++;
                }
            }
        }
        
        return getCount;
    }
    
    uint32_t Proto::appendElements(HandleObject targetArray, const HandleValueArray &elements)
    {
        uint32_t appendCount = 0;
        
        if (isArray(targetArray))
        {
            int index1 = getLength(targetArray);
            
            for (auto index2 = 0; index2 < elements.length(); index2++)
            {
                if (setElement(targetArray, index1++, elements[index2]))
                {
                    appendCount++;
                }
            }
        }
        
        return appendCount;
    }
    
    bool Proto::appendElement(HandleObject array, HandleValue value)
    {
        if (array)
        {
            uint32_t length;
            
            if (JS_GetArrayLength(cx, array, &length))
            {
                return JS_SetElement(cx, array, length, value);
            }
        }
        
        return false;
    }
}
