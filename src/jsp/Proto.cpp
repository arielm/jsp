/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "jsp/Proto.h"

using namespace std;
using namespace ci;
using namespace chr;

namespace jsp
{
#pragma mark ---------------------------------------- EVALUATION ----------------------------------------
    
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
    
    namespace proto
    {
        JSObject* newPlainObject()
        {
            return JS_NewObject(cx, nullptr, NullPtr(), NullPtr());
        }
        
        JSObject* newObject(const std::string &className, const HandleValueArray& args)
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
        
        bool hasProperty(HandleObject object, const char *name)
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
        
        bool hasOwnProperty(HandleObject object, const char *name)
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
        
        bool getOwnPropertyDescriptor(HandleObject object, HandleId id, MutableHandle<JSPropertyDescriptor> desc)
        {
            return js::GetOwnPropertyDescriptor(cx, object, id, desc); // XXX: NON-PUBLIC
        }
        
        bool getProperty(HandleObject object, const char *name, MutableHandleValue result)
        {
            if (object)
            {
                return JS_GetProperty(cx, object, name, result);
            }
            
            return false;
        }
        
        bool setProperty(HandleObject object, const char *name, HandleValue value)
        {
            if (object)
            {
                return JS_SetProperty(cx, object, name, value);
            }
            
            return false;
        }
        
        bool defineProperty(HandleObject object, const char *name, HandleValue value, unsigned attrs)
        {
            if (object)
            {
                return JS_DefineProperty(cx, object, name, value, attrs);
            }
            
            return false;
        }
        
        bool deleteProperty(HandleObject object, const char *name)
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
        
        JSObject* newArray(size_t length)
        {
            return JS_NewArrayObject(cx, length);
        }
        
        JSObject* newArray(const HandleValueArray& contents)
        {
            return JS_NewArrayObject(cx, contents);
        }
        
        bool hasElement(HandleObject array, int index)
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
        
        uint32_t getElementCount(HandleObject array)
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
        
        uint32_t getLength(HandleObject array)
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
        
        bool setLength(HandleObject array, size_t length)
        {
            if (array)
            {
                return JS_SetArrayLength(cx, array, length);
            }
            
            return false;
        }
        
        bool getElement(HandleObject array, int index, MutableHandleValue result)
        {
            if (array && (index >= 0))
            {
                return JS_GetElement(cx, array, index, result);
            }
            
            return false;
        }
        
        bool setElement(HandleObject array, int index, HandleValue value)
        {
            if (array && (index >= 0))
            {
                return JS_SetElement(cx, array, index, value);
            }
            
            return false;
        }
        
        bool defineElement(HandleObject array, int index, HandleValue value, unsigned attrs)
        {
            if (array && (index >= 0))
            {
                return JS_DefineElement(cx, array, index, value, nullptr, nullptr, attrs);
            }
            
            return false;
        }
        
        bool deleteElement(HandleObject array, int index)
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
    }
}
