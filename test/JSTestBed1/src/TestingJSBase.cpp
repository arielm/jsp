/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "TestingJSBase.h"

using namespace std;
using namespace ci;
using namespace chr;

using namespace jsp;

void TestingJSBase::setup()
{
    JSP::forceGC();
    performSetup();
}

void TestingJSBase::shutdown()
{
    performShutdown();
    JSP::forceGC();
}

void TestingJSBase::run(bool force)
{
    performRun(force);
}

// ---

/*
 * QUICK AND DIRTY IMPLEMENTATION
 *
 * TODO: CONSIDER IMPLEMENTING "TRUE" FUNCTION CREATION AND COMPILATION, ETC.
 */
bool TestingJSBase::evaluateBoolean(const string &source)
{
    string wrapped = "(function() { " + source + " })()";
    
    // ---
    
    OwningCompileOptions options(cx);
    options.setForEval(true);
    options.setVersion(JSVersion::JSVERSION_LATEST);
    options.setUTF8(true);
    options.setFileAndLine(cx, "", 1);
    
    // ---
    
    RootedValue result(cx);
    
    if (eval(wrapped, options, &result))
    {
        return ToBoolean(result);
    }
    
    throw EXCEPTION(TestingJSBase, "EVALUATION FAILED");
}

/*
 * QUICK AND DIRTY IMPLEMENTATION
 *
 * TODO: SHOULD BE DONE VIA THE Proto API
 */
const string TestingJSBase::evaluateString(const string &source)
{
    OwningCompileOptions options(cx);
    options.setForEval(true);
    options.setVersion(JSVersion::JSVERSION_LATEST);
    options.setUTF8(true);
    options.setFileAndLine(cx, "", 1);
    
    // ---
    
    RootedValue result(cx);
    
    if (eval(source, options, &result))
    {
        return jsp::toString(result);
    }
    
    throw EXCEPTION(TestingJSBase, "EVALUATION FAILED");
}

// ---

/*
 * SIMILAR TO Manager::function_print
 */
const string TestingJSBase::write(const HandleValueArray& args)
{
    string buffer;
    RootedString rooted(cx);
    
    for (auto i = 0; i < args.length(); i++)
    {
        rooted = ToString(cx, args[i]);
        JSAutoByteString tmp;
        
        if (tmp.encodeUtf8(cx, rooted))
        {
            if (i > 0) buffer += ' ';
            buffer += tmp.ptr();
        }
        else
        {
            break;
        }
    }
    
    return buffer;
}

// ---

bool TestingJSBase::fail(const string &file, int line, const string &reason)
{
    LOGI << "FAILURE";
    
    if (!reason.empty())
    {
        LOGI << ": " << reason;
    }
    
    if (!file.empty())
    {
        LOGI << " [" << file; // TODO: POSSIBILITY TO EXCLUDE FULL-PATH (RELATIVE TO SOME DEFINABLE "ROOT PATH")
        
        if (line > 0)
        {
            LOGI << " | LINE: " << line;
        }
        
        LOGI << "]";
    }
    
    LOGI << endl;
    
    return false;
}
