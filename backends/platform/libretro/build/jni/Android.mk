LOCAL_PATH := $(call my-dir)

# Reset flags that the common makefile doesn't properly handle
DEFINES   :=
INCLUDES  :=
OBJS_DEPS :=
DETECT_OBJS :=
$(foreach MODULE,$(MODULES),$(MODULE_OBJS-$(MODULE)) :=)
MODULES   :=

CORE_DIR  := $(LOCAL_PATH)/../../../../..
srcdir    := $(CORE_DIR)

USE_ZLIB      := 1
USE_TREMOR    := 0
USE_VORBIS    := 1
USE_FLAC      := 1
USE_MAD       := 1
USE_FAAD      := 1
USE_PNG       := 1
USE_JPEG      := 1
USE_THEORADEC := 1
USE_FREETYPE2 := 1
HAVE_MT32EMU  := 1
USE_FLUIDSYNTH:= 1
USE_LUA       := 1
USE_LIBCO     := 1
LOAD_RULES_MK  = 1
USE_TINYGL     = 1
USE_BINK       = 1
POSIX         := 1
BACKEND       := libretro

ifeq ($(HAVE_MT32EMU),1)
USE_MT32EMU = 1
DEFINES += -DUSE_MT32EMU
endif

include $(LOCAL_PATH)/../Makefile.common
include $(addprefix $(CORE_DIR)/, $(addsuffix /module.mk,$(MODULES)))
OBJS_MODULES := $(addprefix $(CORE_DIR)/, $(foreach MODULE,$(MODULES),$(MODULE_OBJS-$(MODULE))))
SOURCES_C    := $(LIBRETRO_COMM_DIR)/libco/libco.c
SOURCES_CXX  := $(LIBRETRO_DIR)/libretro.cpp $(LIBRETRO_DIR)/libretro_os.cpp $(LIBRETRO_DIR)/libretro-fs.cpp $(LIBRETRO_DIR)/libretro-fs-factory.cpp

COREFLAGS := $(DEFINES) $(INCLUDES) -D__LIBRETRO__ -DNONSTANDARD_PORT -DUSE_RGB_COLOR -DUSE_OSD -DDISABLE_TEXT_CONSOLE -DFRONTEND_SUPPORTS_RGB565 -DUSE_LIBCO  -DUSE_TRANSLATION -DDETECTION_STATIC -DHAVE_CONFIG_H -DUSE_BINK -DUSE_CXX11
COREFLAGS += -Wno-multichar -Wno-undefined-var-template -Wno-pragma-pack

ifeq ($(TARGET_ARCH),arm)
  COREFLAGS += -D_ARM_ASSEM_
endif

# All current 64-bit archs have 64 in the abi name
ifeq ($(filter $(TARGET_ARCH_ABI),64),64)
  COREFLAGS += -DSIZEOF_SIZE_T=8
else
  COREFLAGS += -DSIZEOF_SIZE_T=4
endif

GIT_VERSION := " $(shell git rev-parse --short HEAD || echo unknown)"
ifneq ($(GIT_VERSION)," unknown")
  COREFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
endif

include $(CLEAR_VARS)
LOCAL_MODULE       := retro
LOCAL_SRC_FILES    := $(SOURCES_C) $(SOURCES_CXX) $(DETECT_OBJS:%.o=$(CORE_DIR)/%.cpp)  $(OBJS_DEPS:%.o=%.c) $(OBJS_MODULES:%.o=%.cpp)
LOCAL_CPPFLAGS     := $(COREFLAGS) -std=c++11
LOCAL_CFLAGS       := $(COREFLAGS)
LOCAL_LDFLAGS      := -Wl,-version-script=$(LIBRETRO_DIR)/link.T
LOCAL_LDLIBS       := -lz -llog
LOCAL_CPP_FEATURES := rtti
LOCAL_ARM_MODE     := arm
include $(BUILD_SHARED_LIBRARY)
