/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "Context.h"

#include "jsp/Manager.h"
#include "jsp/Base.h"

using namespace std;
using namespace ci;
using namespace chr;

namespace context
{
    namespace intern
    {
        shared_ptr<jsp::Manager> jsManager;
        shared_ptr<jsp::Proto> jsProto;

        // ---
        
        bool setup = false;
    }
    
    void setup()
    {
        if (!intern::setup)
        {
            intern::jsManager = make_shared<jsp::Manager>();
            intern::jsProto = make_shared<jsp::Base>();
            
            intern::jsManager->init(); // TODO: HANDLE FAILURE
            
            // ---
            
            intern::setup = true;
        }
    }
    
    void shutdown()
    {
        if (intern::setup)
        {
            intern::jsManager->shutdown();

            intern::jsProto.reset();
            intern::jsManager.reset();

            // ---
            
            intern::setup = false;
        }
    }
    
    // ---
    
    jsp::Proto* jsProto()
    {
        return intern::jsProto.get();
    }
}
