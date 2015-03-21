ifndef JSP_BLOCK_PATH
    $(error JSP_BLOCK_PATH MUST BE DEFINED!)
endif

###

JSP_SRC := $(JSP_BLOCK_PATH)/src
LOCAL_C_INCLUDES += $(JSP_SRC)

LOCAL_SRC_FILES += $(JSP_SRC)/jsp/Context.cpp
LOCAL_SRC_FILES += $(JSP_SRC)/jsp/WrappedObject.cpp
LOCAL_SRC_FILES += $(JSP_SRC)/jsp/WrappedValue.cpp
LOCAL_SRC_FILES += $(JSP_SRC)/jsp/Barker.cpp
LOCAL_SRC_FILES += $(JSP_SRC)/jsp/CloneBuffer.cpp
LOCAL_SRC_FILES += $(JSP_SRC)/jsp/Manager.cpp
LOCAL_SRC_FILES += $(JSP_SRC)/jsp/Proto.cpp
LOCAL_SRC_FILES += $(JSP_SRC)/jsp/Proxy.cpp
LOCAL_SRC_FILES += $(JSP_SRC)/jsp/Base.cpp
