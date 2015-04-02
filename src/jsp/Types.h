/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#pragma once

#include <string>

class JSObject;

namespace jsp
{
    typedef JSObject* OBJECT;
    typedef int32_t INT32;
    typedef uint32_t UINT32;
    typedef float FLOAT32;
    typedef double FLOAT64;
    typedef bool BOOLEAN;
    typedef std::string STRING;
    
    template <class T>
    class TypeTraits
    {
    public:
        static constexpr bool isNumber = false;
        static constexpr bool isInteger = false;
        static constexpr bool isSigned = false;
        static constexpr bool isStatic = false;
        static constexpr bool isText = false;
        static constexpr bool isObject = false;
        static constexpr bool isMarkable = false;
        
        typedef T defaultType;
        inline static constexpr defaultType defaultValue() noexcept { return defaultType(); }
    };
    
    // ---
    
    template <>
    class TypeTraits<JSObject*>
    {
    public:
        static constexpr bool isNumber = false;
        static constexpr bool isInteger = false;
        static constexpr bool isSigned = false;
        static constexpr bool isStatic = false;
        static constexpr bool isText = false;
        static constexpr bool isObject = true;
        static constexpr bool isMarkable = true;
        
        typedef JSObject* defaultType;
        inline static constexpr defaultType defaultValue() noexcept { return nullptr; }
    };
    
    template <>
    class TypeTraits<int32_t>
    {
    public:
        static constexpr bool isNumber = true;
        static constexpr bool isInteger = true;
        static constexpr bool isSigned = true;
        static constexpr bool isStatic = false;
        static constexpr bool isText = false;
        static constexpr bool isObject = false;
        static constexpr bool isMarkable = false;
        
        typedef int32_t defaultType;
        inline static constexpr defaultType defaultValue() noexcept { return 0; }
    };
    
    template <>
    class TypeTraits<uint32_t>
    {
    public:
        static constexpr bool isNumber = true;
        static constexpr bool isInteger = true;
        static constexpr bool isSigned = false;
        static constexpr bool isStatic = false;
        static constexpr bool isText = false;
        static constexpr bool isObject = false;
        static constexpr bool isMarkable = false;
        
        typedef uint32_t defaultType;
        inline static constexpr defaultType defaultValue() noexcept { return 0; }
    };
    
    template <>
    class TypeTraits<float>
    {
    public:
        static constexpr bool isNumber = true;
        static constexpr bool isInteger = false;
        static constexpr bool isSigned = true;
        static constexpr bool isStatic = false;
        static constexpr bool isText = false;
        static constexpr bool isObject = false;
        static constexpr bool isMarkable = false;
        
        typedef float defaultType;
        inline static constexpr defaultType defaultValue() noexcept { return 0; }
    };
    
    template <>
    class TypeTraits<double>
    {
    public:
        static constexpr bool isNumber = true;
        static constexpr bool isInteger = false;
        static constexpr bool isSigned = true;
        static constexpr bool isStatic = false;
        static constexpr bool isText = false;
        static constexpr bool isObject = false;
        static constexpr bool isMarkable = false;
        
        typedef double defaultType;
        inline static constexpr defaultType defaultValue() noexcept { return 0; }
    };
    
    template <>
    class TypeTraits<bool>
    {
    public:
        static constexpr bool isNumber = true;
        static constexpr bool isInteger = true;
        static constexpr bool isSigned = false;
        static constexpr bool isStatic = false;
        static constexpr bool isText = false;
        static constexpr bool isObject = false;
        static constexpr bool isMarkable = false;

        typedef bool defaultType;
        inline static constexpr defaultType defaultValue() noexcept { return false; }
    };
    
    template <>
    class TypeTraits<std::string>
    {
    public:
        static constexpr bool isNumber = false;
        static constexpr bool isInteger = false;
        static constexpr bool isSigned = false;
        static constexpr bool isStatic = false;
        static constexpr bool isText = true;
        static constexpr bool isObject = false;
        static constexpr bool isMarkable = true;
        
        typedef const char* defaultType; // XXX: std::string CAN'T BE USED IN constexpr
        inline static constexpr defaultType defaultValue() noexcept { return ""; }
    };

    /*
    template <>
    class TypeTraits<const char*>
    {
    public:
        static constexpr bool isNumber = false;
        static constexpr bool isInteger = false;
        static constexpr bool isSigned = false;
        static constexpr bool isStatic = true;
        static constexpr bool isText = true;
        static constexpr bool isObject = false;
        static constexpr bool isMarkable = true;
        
        typedef const char* defaultType;
        inline static constexpr defaultType defaultValue() noexcept { return ""; }
    };
    */
    
    template <size_t N>
    class TypeTraits<char[N]>
    {
    public:
        static constexpr bool isNumber = false;
        static constexpr bool isInteger = false;
        static constexpr bool isSigned = false;
        static constexpr bool isStatic = true;
        static constexpr bool isText = true;
        static constexpr bool isObject = false;
        static constexpr bool isMarkable = true;
        
        typedef const char* defaultType;
        inline static constexpr defaultType defaultValue() noexcept { return ""; }
    };
    
    template <>
    class TypeTraits<std::nullptr_t>
    {
    public:
        static constexpr bool isNumber = false;
        static constexpr bool isInteger = false;
        static constexpr bool isSigned = false;
        static constexpr bool isStatic = false;
        static constexpr bool isText = false;
        static constexpr bool isObject = false;
        static constexpr bool isMarkable = false;
        
        typedef std::nullptr_t defaultType;
        inline static constexpr defaultType defaultValue() noexcept { return nullptr; }
    };
}
