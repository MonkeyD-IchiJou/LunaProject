APP_PLATFORM := android-23
APP_STL := c++_static
APP_CPPFLAGS += -frtti
APP_CPPFLAGS += -fexceptions
APP_CPPFLAGS += -DANDROID
APP_CPPFLAGS += -std=c++11
APP_ABI := armeabi-v7a
NDK_TOOLCHAIN_VERSION := clang