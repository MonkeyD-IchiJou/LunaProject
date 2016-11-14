APP_PLATFORM := android-23
APP_STL := c++_static
APP_CPPFLAGS += -frtti
APP_CPPFLAGS += -fexceptions
APP_CPPFLAGS += -DANDROID
APP_CPPFLAGS += -std=c++11
APP_ABI := arm64-v8a
NDK_TOOLCHAIN_VERSION := clang