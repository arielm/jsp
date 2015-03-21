/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "jsp/Proxy.h"

#include "chronotext/utils/Utils.h"

using namespace std;
using namespace chr;

namespace jsp
{
    Proxy::Proxy(Proto *target)
    {
        if (!setTarget(target))
        {
            throw EXCEPTION(Proxy, "INVALID TARGET");
        }
    }
    
    Proxy::Proxy(Proxy *target)
    {
        if (!setTarget(target))
        {
            throw EXCEPTION(Proxy, "INVALID TARGET");
        }
    }
    
    bool Proxy::setTarget(Proto *target)
    {
        if (target && (target != this) && (target != handler))
        {
            this->target = target;
            return true;
        }
        
        return false;
    }
    
    bool Proxy::setTarget(Proxy *target)
    {
        if (target && (target->target != this))
        {
            return setTarget(static_cast<Proto*>(target));
        }
        
        return false;
    }
    
    bool Proxy::setHandler(Proto *handler)
    {
        if ((handler != this) && (handler != target))
        {
            this->handler = handler;
            return true;
        }
        
        return false;
    }
    
    bool Proxy::setHandler(Proxy *handler)
    {
        if (handler->target != this)
        {
            return setHandler(static_cast<Proto*>(handler));
        }
        
        return false;
    }
    
    Proxy::~Proxy()
    {
        // TODO: UNREGISTER ASSOCIATED NATIVE-CALLBACKS
    }
    
    // ---
    
    /*
     * TODO: CALLBACK-MAPPING SHOULD TAKE PLACE PER GLOBAL-OBJECT (I.E. PER COMPARTMENT)
     */
    
    vector<Callback> Proxy::callbacks {};
    
    void Proxy::registerCallback(HandleObject object, const string &name, const function<bool(CallArgs args)> &fn, Proxy *proxy)
    {
        int32_t key = callbacks.size();
        callbacks.emplace_back(fn, proxy);
        
        RootedFunction function(cx, DefineFunctionWithReserved(cx, object, name.data(), dispatchCallback, 0, 0));
        SetFunctionNativeReserved(function, 0, NumberValue(key));
    }
    
    bool Proxy::dispatchCallback(JSContext *cx, unsigned argc, Value *vp)
    {
        auto args = CallArgsFromVp(argc, vp);
        JSObject &callee = args.callee();
        
        JSFunction *function = &callee.as<JSFunction>();
        int32_t key = GetFunctionNativeReserved(function, 0).toInt32();
        
        auto &callback = callbacks[key];
        return callback.proxy->applyCallback(callback.fn, args);
    }
}
