/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

/*
 * SPIDERMONKEY LIFE-CYCLE BASED ON:
 *
 * https://github.com/mozilla/gecko-dev/blob/esr31/js/src/jsapi-tests/tests.h
 */

#include "jsp/Manager.h"

#include "jsp/Barker.h"
#include "jsp/Proxy.h"

#include "chronotext/utils/Utils.h"

using namespace std;
using namespace ci;
using namespace chr;

namespace jsp
{
    uint32_t Manager::MAX_BYTES = 8L * 1024 * 1024;
    size_t Manager::STACK_CHUNK_SIZE = 8192;
    size_t Manager::MAX_STACK_SIZE = 128 * sizeof(size_t) * 1024;
    
    JSUseHelperThreads Manager::USE_HELPER_THREADS = JS_NO_HELPER_THREADS;
    bool Manager::EXTRA_WARNINGS = true;
    
    const JSClass Manager::global_class =
    {
        "global",
        JSCLASS_GLOBAL_FLAGS,
        JS_PropertyStub,
        JS_DeletePropertyStub,
        JS_PropertyStub,
        JS_StrictPropertyStub,
        JS_EnumerateStub,
        JS_ResolveStub,
        JS_ConvertStub,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        JS_GlobalObjectTraceHook
    };
    
    const JSFunctionSpec Manager::global_functions[]
    {
        JS_FS("print", function_print, 0, 0),
        JS_FS("forceGC", function_forceGC, 0, 0),
        JS_FS_END
    };
    
#pragma mark ---------------------------------------- CALLBACKS ----------------------------------------
    
    void Manager::reportError(JSContext *cx, const char *message, JSErrorReport *report)
    {
        if (JSREPORT_IS_WARNING(report->flags) && !EXTRA_WARNINGS)
        {
            return;
        }
        
        // ---
        
        string errorPrefix;
        
        if JSREPORT_IS_WARNING(report->flags)
        {
            if JSREPORT_IS_STRICT(report->flags)
            {
                errorPrefix += "STRICT WARNING";
            }
            else
            {
                errorPrefix += "WARNING";
            }
        }
        else
        {
            errorPrefix += "EXCEPTION";
        }
        
        if (report->lineno)
        {
            errorPrefix += " [";
            
            if (report->filename)
            {
                auto filename = string(report->filename);
                
                if (!filename.empty())
                {
                    errorPrefix += filename + " | ";
                }
            }
            
            errorPrefix += "LINE " + ci::toString(report->lineno) + "]";
        }
        
        // ---
        
        string errorBody;
        
        auto typeName = jsp::toString(js::GetErrorTypeName(JS_GetRuntime(cx), report->exnType));
        
        if (!typeName.empty())
        {
            if (!boost::starts_with(message, typeName))
            {
                errorBody += typeName + ": ";
            }
        }
        
        errorBody += message;
        
        // ---
        
        LOGI << "JS " << errorPrefix << " " << errorBody << endl; // TODO: IT SHOULD BE POSSIBLE TO DEFINE WHICH std::ostream IS USED
    }
    
    bool Manager::function_print(JSContext *cx, unsigned argc, Value *vp)
    {
        auto args = CallArgsFromVp(argc, vp);
        
        bool failed = false;
        string buffer;
        RootedString rooted(cx);
        
        for (int i = 0; i < args.length(); i++)
        {
            rooted = ToString(cx, args[i]);
            JSAutoByteString tmp;
            
            if (tmp.encodeUtf8(cx, rooted))
            {
                buffer += (i > 0) ? " " : "";
                buffer += tmp.ptr();
            }
            else
            {
                failed = true;
                break;
            }
        }
        
        if (buffer.size())
        {
            LOGI << buffer << endl; // TODO: IT SHOULD BE POSSIBLE TO DEFINE WHICH std::ostream IS USED
        }
        
        if (failed)
        {
            return false;
        }
        
        args.rval().setUndefined();
        return true;
    }
    
    bool Manager::function_forceGC(JSContext *cx, unsigned argc, Value *vp)
    {
        JSP::forceGC();
        
        CallArgsFromVp(argc, vp).rval().setUndefined();
        return true;
    }
    
#pragma mark ---------------------------------------- LIFECYCLE ----------------------------------------
    
    Manager::~Manager()
    {
        shutdown();
    }

    bool Manager::init()
    {
        if (!initialized)
        {
            if (performInit() && jsp::postInit())
            {
                JS_DefineFunctions(cx, globalHandle(), global_functions);
                
                Barker::init();
                Proxy::init();
                
                // ---
                
                initialized = true;
            }
        }
        
        return initialized;
    }
    
    void Manager::shutdown()
    {
        if (initialized)
        {
            Barker::uninit();
            Proxy::uninit();

            jsp:preShutdown();
            performShutdown();
            
            // ---
            
            initialized = false;
        }
    }
    
    // ---
    
    bool Manager::performInit()
    {
        if (!rt && !cx)
        {
            if (JS_Init())
            {
                if (createRuntime())
                {
                    if (createContext())
                    {
                        JS_BeginRequest(cx);
                        
                        if (createGlobal())
                        {
                            JS_EnterCompartment(cx, global);
                            return true;
                        }
                    }
                }
            }
        }
        
        return false;
    }
    
    void Manager::performShutdown()
    {
        if (rt && cx)
        {
            RemoveObjectRoot(cx, &global);
            global = nullptr;
            
            JS_LeaveCompartment(cx, nullptr);
            JS_EndRequest(cx);
            
            // ---
            
            JS_DestroyContext(cx);
            cx = nullptr;
            
            JS_DestroyRuntime(rt);
            rt = nullptr;
            
            JS_ShutDown();
        }
    }
    
    // ---
    
    bool Manager::createRuntime()
    {
        rt = JS_NewRuntime(MAX_BYTES, USE_HELPER_THREADS);
        
        if (rt)
        {
            JS_SetNativeStackQuota(rt, MAX_STACK_SIZE);
            return true;
        }
        
        return false;
    }
    
    bool Manager::createContext()
    {
        cx = JS_NewContext(rt, STACK_CHUNK_SIZE);
        
        if (cx)
        {
            ContextOptionsRef(cx).setVarObjFix(true).setExtraWarnings(EXTRA_WARNINGS);
            JS_SetErrorReporter(cx, &reportError);
            
            return true;
        }
        
        return false;
    }
    
    bool Manager::createGlobal()
    {
        CompartmentOptions options;
        options.setVersion(JSVersion::JSVERSION_LATEST);
        
        global = JS_NewGlobalObject(cx, &global_class, nullptr, FireOnNewGlobalHook, options);
        
        if (global)
        {
            AddObjectRoot(cx, &global);
            JSAutoCompartment ac(cx, globalHandle());
            
            return JS_InitStandardClasses(cx, globalHandle());
        }
        
        return false;
    }
}
