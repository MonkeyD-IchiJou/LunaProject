LOCAL_PATH := $(call my-dir)/../../Source

include $(CLEAR_VARS)

LS_CPP=$(subst $(1)/,,$(wildcard $(1)/*.cpp))

LOCAL_MODULE := androidapp

LOCAL_CPPFLAGS := -std=c++11
LOCAL_CPPFLAGS += -D__STDC_LIMIT_MACROS
LOCAL_CPPFLAGS += -DVK_NO_PROTOTYPES
LOCAL_CPPFLAGS += -DVK_USE_PLATFORM_ANDROID_KHR

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../Libraries
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../Libraries/glm
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../Libraries/gli

LOCAL_SRC_FILES := $(call LS_CPP,$(LOCAL_PATH))

LOCAL_LDLIBS := -landroid -llog -lz

LOCAL_DISABLE_FORMAT_STRING_CHECKS := true
LOCAL_DISABLE_FATAL_LINKER_WARNINGS := true

LOCAL_STATIC_LIBRARIES += android_native_app_glue
LOCAL_STATIC_LIBRARIES += cpufeatures

include $(BUILD_SHARED_LIBRARY)

$(call import-module, android/native_app_glue)
$(call import-module, android/cpufeatures)