/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#pragma once

#include "jsp/Proto.h"

namespace jsp
{
    class BaseProto : public Proto
    {
    public:
        static BaseProto* target();

    private:
        BaseProto() = default;
        BaseProto(const BaseProto &other) = delete;
    };
}
