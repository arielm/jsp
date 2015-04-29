LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

ifndef CINDER_PATH
    $(error CINDER_PATH MUST BE DEFINED!)
endif

ifndef SPIDERMONKEY_SDK_PATH
    $(error SPIDERMONKEY_SDK_PATH MUST BE DEFINED!)
endif

###

CHR_BLOCK_PATH = $(CINDER_PATH)/blocks/new-chronotext-toolkit
include $(CHR_BLOCK_PATH)/android/Android.mk

###

JSP_BLOCK_PATH = $(CINDER_PATH)/blocks/jsp
include $(JSP_BLOCK_PATH)/android/Android.mk

###

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../src
FILE_LIST := $(wildcard $(LOCAL_PATH)/../../src/*.cpp)
LOCAL_SRC_FILES += $(FILE_LIST:$(LOCAL_PATH)/%=%)

###

LOCAL_CFLAGS += -DJS_POSIX_NSPR -DJSGC_USE_EXACT_ROOTING -DJSGC_GENERATIONAL -DJSP_USE_PRIVATE_APIS -DFORCE_LOG
LOCAL_CFLAGS += -ffast-math -O3
#LOCAL_CFLAGS += -g -DDEBUG

###

LOCAL_LDLIBS := -llog -landroid
LOCAL_STATIC_LIBRARIES := cinder android_native_app_glue boost_system boost_filesystem boost_thread
LOCAL_STATIC_LIBRARIES += spidermonkey

LOCAL_MODULE := JSTestBed1
include $(BUILD_SHARED_LIBRARY)

###

$(call import-module,android/native_app_glue)

$(call import-add-path, $(CINDER_PATH)/android/prebuilt)
$(call import-module,cinder)
$(call import-module,boost)

$(call import-add-path, $(SPIDERMONKEY_SDK_PATH)/android/prebuilt)
$(call import-module,spidermonkey)
