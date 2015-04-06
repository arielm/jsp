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
    
#pragma mark ---------------------------------------- PROPERTY GETTERS (SAFE) ----------------------------------------
    
    template <>
    JSObject* Proto::get(HandleObject targetObject, const char *propertyName, JSObject *defaultValue)
    {
        RootedValue value(cx);
        
        if (getProperty(targetObject, propertyName, &value))
        {
            return toObjectSafely(value, defaultValue);
        }
        
        return defaultValue;
    }
    
    template <>
    float Proto::get(HandleObject targetObject, const char *propertyName, float defaultValue)
    {
        RootedValue value(cx);
        
        if (getProperty(targetObject, propertyName, &value))
        {
            return toFloat32Safely(value, defaultValue);
        }
        
        return defaultValue;
    }
    
    template <>
    double Proto::get(HandleObject targetObject, const char *propertyName, double defaultValue)
    {
        RootedValue value(cx);
        
        if (getProperty(targetObject, propertyName, &value))
        {
            return toFloat64Safely(value, defaultValue);
        }
        
        return defaultValue;
    }
    
    template <>
    int32_t Proto::get(HandleObject targetObject, const char *propertyName, int32_t defaultValue)
    {
        RootedValue value(cx);
        
        if (getProperty(targetObject, propertyName, &value))
        {
            return toInt32Safely(value, defaultValue);
        }
        
        return defaultValue;
    }

    template <>
    uint32_t Proto::get(HandleObject targetObject, const char *propertyName, uint32_t defaultValue)
    {
        RootedValue value(cx);
        
        if (getProperty(targetObject, propertyName, &value))
        {
            return toUInt32Safely(value, defaultValue);
        }
        
        return defaultValue;
    }

    template <>
    bool Proto::get(HandleObject targetObject, const char *propertyName, bool defaultValue)
    {
        RootedValue value(cx);
        
        if (getProperty(targetObject, propertyName, &value))
        {
            return toBoolean(value); // INFAILIBLE, POSSIBLY SLOW
        }
        
        return defaultValue;
    }
    
    template <>
    string Proto::get(HandleObject targetObject, const char *propertyName, const char *defaultValue)
    {
        RootedValue value(cx);
        
        if (getProperty(targetObject, propertyName, &value))
        {
            return jsp::toString(value); // INFAILIBLE, POSSIBLY SLOW
        }
        
        return defaultValue;
    }
    
#pragma mark ---------------------------------------- ELEMENT GETTERS (CAN THROW) ----------------------------------------
    
    template <>
    JSObject* Proto::get(HandleObject targetArray, uint32_t elementIndex)
    {
        RootedValue value(cx);
        
        if (getElement(targetArray, elementIndex, &value))
        {
            JSObject *result;
            
            if (toObjectMaybe(value, &result))
            {
                return result;
            }
        }
        
        throw EXCEPTION(Proto, "CAN'T GET ELEMENT AT INDEX " + ci::toString(elementIndex));
    }
    
    template <>
    float Proto::get(HandleObject targetArray, uint32_t elementIndex)
    {
        RootedValue value(cx);
        
        if (getElement(targetArray, elementIndex, &value))
        {
            float result;
            
            if (toFloat32Maybe(value, &result))
            {
                return result;
            }
        }
        
        throw EXCEPTION(Proto, "CAN'T GET ELEMENT AT INDEX " + ci::toString(elementIndex));
    }
    
    template <>
    double Proto::get(HandleObject targetArray, uint32_t elementIndex)
    {
        RootedValue value(cx);
        
        if (getElement(targetArray, elementIndex, &value))
        {
            double result;
            
            if (toFloat64Maybe(value, &result))
            {
                return result;
            }
        }
        
        throw EXCEPTION(Proto, "CAN'T GET ELEMENT AT INDEX " + ci::toString(elementIndex));
    }
    
    template <>
    int32_t Proto::get(HandleObject targetArray, uint32_t elementIndex)
    {
        RootedValue value(cx);
        
        if (getElement(targetArray, elementIndex, &value))
        {
            int32_t result;
            
            if (toInt32Maybe(value, &result))
            {
                return result;
            }
        }
        
        throw EXCEPTION(Proto, "CAN'T GET ELEMENT AT INDEX " + ci::toString(elementIndex));
    }
    
    template <>
    uint32_t Proto::get(HandleObject targetArray, uint32_t elementIndex)
    {
        RootedValue value(cx);
        
        if (getElement(targetArray, elementIndex, &value))
        {
            uint32_t result;
            
            if (toUInt32Maybe(value, &result))
            {
                return result;
            }
        }
        
        throw EXCEPTION(Proto, "CAN'T GET ELEMENT AT INDEX " + ci::toString(elementIndex));
    }
    
    template <>
    bool Proto::get(HandleObject targetArray, uint32_t elementIndex)
    {
        RootedValue value(cx);
        
        if (getElement(targetArray, elementIndex, &value))
        {
            return toBoolean(value); // INFAILIBLE, POSSIBLY SLOW
        }
        
        throw EXCEPTION(Proto, "CAN'T GET ELEMENT AT INDEX " + ci::toString(elementIndex));
    }

    template <>
    string Proto::get(HandleObject targetArray, uint32_t elementIndex)
    {
        RootedValue value(cx);
        
        if (getElement(targetArray, elementIndex, &value))
        {
            return jsp::toString(value); // INFAILIBLE, POSSIBLY SLOW
        }
        
        throw EXCEPTION(Proto, "CAN'T GET ELEMENT AT INDEX " + ci::toString(elementIndex));
    }
}
