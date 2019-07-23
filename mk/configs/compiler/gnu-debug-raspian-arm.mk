include $(BUILD_ROOT)/mk/configs/compiler/raspian_gnu_localbuild.mk
include $(BUILD_ROOT)/mk/configs/compiler/include_common.mk

CFLAGS := 	$(DEBUG) \
			$(LINUX_GNU_CFLAGS) \
			$(LINUX_GNU_INCLUDES)
			

CXXFLAGS := $(DEBUG) \
			$(LINUX_GNU_CXXFLAGS) \
			$(LINUX_GNU_INCLUDES) \
			$(LORA_RASPI_FLAGS) 

COMPILER := raspian-arm-debug-gnu
