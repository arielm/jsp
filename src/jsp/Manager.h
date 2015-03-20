/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#pragma once

#include "Context.h"

namespace jsp
{
    class Manager
    {
    public:
        static uint32_t MAX_BYTES;
        static size_t STACK_CHUNK_SIZE;
        static size_t MAX_STACK_SIZE;
        
        static JSUseHelperThreads USE_HELPER_THREADS;
        static bool EXTRA_WARNINGS;
        
        // ---

        static void reportError(JSContext *cx, const char *message, JSErrorReport *report);
        static bool print(JSContext *cx, unsigned argc, Value *vp);

        static const JSClass global_class;
        static const JSFunctionSpec global_functions[];
        
        // ---
        
        virtual ~Manager();
        
        virtual bool init();
        virtual void shutdown();
        
        virtual bool performInit();
        virtual void performShutdown();
        
        virtual bool createRuntime();
        virtual bool createContext();
        virtual bool createGlobal();
        
    protected:
        bool initialized = false;
    };
}
