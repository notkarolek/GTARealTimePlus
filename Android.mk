LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cpp .cc
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
    LOCAL_MODULE := GTARealTimePlus
    # ARM32 specific flags
    LOCAL_CFLAGS := -O2 -mfloat-abi=softfp -DNDEBUG -std=c++14
else
    LOCAL_MODULE := GTARealTimePlus64
    # ARM64 flags - NO float-abi flag
    LOCAL_CFLAGS := -O2 -DNDEBUG -std=c++14
endif
LOCAL_SRC_FILES := main.cpp mod/logger.cpp mod/config.cpp
LOCAL_C_INCLUDES += ./include
LOCAL_LDLIBS += -llog
include $(BUILD_SHARED_LIBRARY)