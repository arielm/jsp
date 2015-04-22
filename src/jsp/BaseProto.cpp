/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#include "jsp/BaseProto.h"

using namespace std;
using namespace chr;

namespace jsp
{
    BaseProto* BaseProto::target()
    {
        static shared_ptr<BaseProto> target;

        if (!target)
        {
            target = shared_ptr<BaseProto>(new BaseProto);
        }
        
        return target.get();
    }
}
