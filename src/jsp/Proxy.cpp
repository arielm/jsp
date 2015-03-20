/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "Proxy.h"

#include "chronotext/utils/Utils.h"

using namespace std;
using namespace ci;
using namespace chr;

namespace jsp
{
    /*
     * TODO: CALLBACK-MAPPING SHOULD TAKE PLACE PER JS GLOBAL-OBJECT
     */
    
    vector<Callback> Proxy::callbacks {};
    
    void Proxy::registerCallback(HandleObject object, const string &name, function<bool(CallArgs args)> fn)
    {
        int32_t key = callbacks.size();
        callbacks.emplace_back(this, fn);
        
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
        return callback.target->invokeCallback(callback.fn, args);
    }
}
