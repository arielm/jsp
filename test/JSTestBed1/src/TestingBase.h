/*
 * JSP: https://github.com/arielm/jsp
 * COPYRIGHT (C) 2014-2015, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE SIMPLIFIED BSD LICENSE:
 * https://github.com/arielm/jsp/blob/master/LICENSE
 */

#pragma once

#include "chronotext/utils/Utils.h"

class TestingBase
{
public:
    template<class TestingBase>
    static void execute(bool proceed = true, bool force = false)
    {
        if (proceed)
        {
            auto test = std::make_shared<TestingBase>();
            
            try
            {
                test->setup();
                test->run(force);
                test->shutdown();
            }
            catch (std::exception &e)
            {
                LOGI << "TEST FAILED | REASON: " << e.what() << std::endl;
            }
        }
    }
    
    virtual ~TestingBase() {}
    
    virtual void setup() {}
    virtual void shutdown() {}
    virtual void run(bool force = false) = 0;
    
    // ---
    
    static ci::fs::path getPublicDirectory()
    {
#if defined(CINDER_ANDROID)
        return chr::FileHelper::getExternalDataPath();
#else
        return ci::getDocumentsDirectory();
#endif
    }
    
    static void dumpDirectory(const ci::fs::path &directoryPath)
    {
        if (ci::fs::exists(directoryPath) && ci::fs::is_directory(directoryPath))
        {
            for (ci::fs::directory_iterator current(directoryPath), end; current != end; ++current)
            {
                if (ci::fs::is_regular_file(current->status()))
                {
                    auto relativePath = chr::FileHelper::relativizePath(directoryPath, current->path());
                    LOGI << relativePath.string() << std::endl;
                }
            }
        }
    }
};
