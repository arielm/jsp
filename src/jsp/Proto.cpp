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
}
