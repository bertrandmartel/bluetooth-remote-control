PROJECT_NAME := ble_app_bps_s130_pca10028

export OUTPUT_FILENAME
#MAKEFILE_NAME := $(CURDIR)/$(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))
MAKEFILE_NAME := $(MAKEFILE_LIST)
MAKEFILE_DIR := $(dir $(MAKEFILE_NAME) ) 

TEMPLATE_PATH = $(NRF51_SDK_DIR)/components/toolchain/gcc
ifeq ($(OS),Windows_NT)
include $(TEMPLATE_PATH)/Makefile.windows
else
include $(TEMPLATE_PATH)/Makefile.posix
endif

MK := mkdir
RM := rm -rf

#echo suspend
ifeq ("$(VERBOSE)","1")
NO_ECHO := 
else
NO_ECHO := @
endif

# Toolchain commands
CC       := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-gcc'
AS       := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-as'
AR       := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-ar' -r
LD       := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-ld'
NM       := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-nm'
OBJDUMP  := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-objdump'
OBJCOPY  := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-objcopy'
SIZE     := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-size'

#function for removing duplicates in a list
remduplicates = $(strip $(if $1,$(firstword $1) $(call remduplicates,$(filter-out $(firstword $1),$1))))


C_SOURCE_FILES += \
$(abspath $(NRF51_SDK_DIR)/components/libraries/util/app_error.c) \
$(abspath $(NRF51_SDK_DIR)/components/libraries/util/app_error_weak.c) \
$(abspath $(NRF51_SDK_DIR)/components/libraries/fifo/app_fifo.c) \
$(abspath $(NRF51_SDK_DIR)/components/libraries/trace/app_trace.c) \
$(abspath $(NRF51_SDK_DIR)/components/libraries/fstorage/fstorage.c) \
$(abspath $(NRF51_SDK_DIR)/components/libraries/util/nrf_assert.c) \
$(abspath $(NRF51_SDK_DIR)/components/libraries/util/nrf_log.c) \
$(abspath $(NRF51_SDK_DIR)/components/libraries/uart/retarget.c) \
$(abspath $(NRF51_SDK_DIR)/components/libraries/sensorsim/sensorsim.c) \
$(abspath $(NRF51_SDK_DIR)/components/libraries/uart/app_uart_fifo.c) \
$(abspath $(NRF51_SDK_DIR)/components/drivers_nrf/common/nrf_drv_common.c) \
$(abspath $(NRF51_SDK_DIR)/components/libraries/gpiote/app_gpiote.c) \
$(abspath $(NRF51_SDK_DIR)/components/drivers_nrf/gpiote/nrf_drv_gpiote.c) \
$(abspath $(NRF51_SDK_DIR)/components/drivers_nrf/uart/nrf_drv_uart.c) \
$(abspath $(NRF51_SDK_DIR)/components/drivers_nrf/pstorage/pstorage.c) \
$(abspath $(NRF51_SDK_DIR)/components/libraries/button/app_button.c) \
$(abspath $(NRF51_SDK_DIR)/components/drivers_nrf/spi_master/nrf_drv_spi.c) \
$(abspath $(NRF51_SDK_DIR)/components/libraries/scheduler/app_scheduler.c) \
$(abspath $(NRF51_SDK_DIR)/components/libraries/timer/app_timer.c) \
$(abspath $(NRF51_SDK_DIR)/components/libraries/util/app_util_platform.c) \
$(abspath $(NRF51_SDK_DIR)/components/libraries/timer/app_timer_appsh.c) \
$(abspath $(NRF51_SDK_DIR)/components/drivers_nrf/delay/nrf_delay.c) \
$(abspath $(NRF51_SDK_DIR)/examples/bsp/bsp.c) \
$(abspath $(NRF51_SDK_DIR)/examples/bsp/bsp_btn_ble.c) \
$(abspath src/ble_displays.c) \
$(abspath $(NRF51_SDK_DIR)/components/ble/common/ble_advdata.c) \
$(abspath $(NRF51_SDK_DIR)/components/ble/ble_advertising/ble_advertising.c) \
$(abspath $(NRF51_SDK_DIR)/components/ble/ble_services/ble_bas/ble_bas.c) \
$(abspath $(NRF51_SDK_DIR)/components/ble/common/ble_conn_params.c) \
$(abspath $(NRF51_SDK_DIR)/components/ble/ble_services/ble_dis/ble_dis.c) \
$(abspath $(NRF51_SDK_DIR)/components/ble/ble_services/ble_hids/ble_hids.c) \
$(abspath $(NRF51_SDK_DIR)/components/ble/common/ble_srv_common.c) \
$(abspath $(NRF51_SDK_DIR)/components/ble/ble_debug_assert_handler/ble_debug_assert_handler.c) \
$(abspath $(NRF51_SDK_DIR)/components/ble/device_manager/device_manager_peripheral.c) \
$(abspath $(NRF51_SDK_DIR)/components/toolchain/system_nrf51.c) \
$(abspath $(NRF51_SDK_DIR)/components/softdevice/common/softdevice_handler/softdevice_handler.c) \
$(abspath $(NRF51_SDK_DIR)/components/softdevice/common/softdevice_handler/softdevice_handler_appsh.c) \
$(abspath $(NRF51_SDK_DIR)/components/drivers_ext/segger_rtt/SEGGER_RTT.c) \
$(abspath $(NRF51_SDK_DIR)/components/drivers_ext/segger_rtt/SEGGER_RTT_printf.c) \
$(abspath adafruit-oled-st7735/src/common.c) \
$(abspath adafruit-oled-st7735/src/adafruit1_8_oled_library.c) \
$(abspath fastlz/fastlz.c) \
$(abspath src/unpack.c) \
$(abspath src/main.c)

#assembly files common to all targets
ASM_SOURCE_FILES  = $(abspath $(NRF51_SDK_DIR)/components/toolchain/gcc/gcc_startup_nrf51.s)


INC_PATHS += -I$(abspath include)
INC_PATHS += -I$(abspath fastlz)
INC_PATHS += -I$(abspath adafruit-oled-st7735/include)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/libraries/gpiote)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/drivers_nrf/gpiote)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/drivers_nrf/spi_master)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/drivers_nrf/config)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/examples/ble_peripheral/ble_app_bps/config/ble_app_bps_s130_pca10028)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/examples/ble_peripheral/ble_app_bps/config)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/libraries/timer)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/libraries/fifo)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/libraries/fstorage/config)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/ble/ble_services/ble_hids)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/softdevice/s130/headers)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/drivers_nrf/delay)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/libraries/util)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/ble/device_manager)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/drivers_nrf/uart)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/ble/common)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/libraries/sensorsim)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/ble/ble_services/ble_bps)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/drivers_nrf/pstorage)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/ble/ble_services/ble_dis)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/device)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/libraries/uart)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/libraries/button)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/libraries/fstorage)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/libraries/experimental_section_vars)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/external/segger_rtt)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/examples/bsp)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/toolchain/CMSIS/Include)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/drivers_nrf/hal)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/toolchain/gcc)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/toolchain)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/drivers_nrf/common)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/ble/ble_advertising)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/softdevice/s130/headers/nrf51)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/libraries/trace)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/ble/ble_services/ble_bas)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/softdevice/common/softdevice_handler)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/libraries/scheduler)


INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/ble/ble_error_log)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/ble/ble_debug_assert_handler)
INC_PATHS += -I$(abspath $(NRF51_SDK_DIR)/components/drivers_nrf/ble_flash)


OBJECT_DIRECTORY = _build
LISTING_DIRECTORY = $(OBJECT_DIRECTORY)
OUTPUT_BINARY_DIRECTORY = $(OBJECT_DIRECTORY)

# Sorting removes duplicates
BUILD_DIRECTORIES := $(sort $(OBJECT_DIRECTORY) $(OUTPUT_BINARY_DIRECTORY) $(LISTING_DIRECTORY) )

#flags common to all targets
CFLAGS  = -DNRF_LOG_USES_UART=1
CFLAGS += -DBOARD_PCA10028
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -DNRF51
CFLAGS += -DS130
CFLAGS += -DBLE_STACK_SUPPORT_REQD
CFLAGS += -DSWI_DISABLE0
CFLAGS += -mcpu=cortex-m0
CFLAGS += -mthumb -mabi=aapcs --std=gnu99
CFLAGS += -Wall  -O3 -g3 #-Werror
CFLAGS += -mfloat-abi=soft
# keep every function in separate section. This will allow linker to dump unused functions
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin --short-enums 
# keep every function in separate section. This will allow linker to dump unused functions
LDFLAGS += -Xlinker -Map=$(LISTING_DIRECTORY)/$(OUTPUT_FILENAME).map
LDFLAGS += -mthumb -mabi=aapcs -L $(TEMPLATE_PATH) -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m0
# let linker to dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs -lc -lnosys

# Assembler flags
ASMFLAGS += -x assembler-with-cpp
ASMFLAGS += -DNRF_LOG_USES_UART=1
ASMFLAGS += -DBOARD_PCA10028
ASMFLAGS += -DSOFTDEVICE_PRESENT
ASMFLAGS += -DNRF51
ASMFLAGS += -DS130
ASMFLAGS += -DBLE_STACK_SUPPORT_REQD
ASMFLAGS += -DSWI_DISABLE0

#default target - first one defined
default: clean nrf51422_xxac_s130

#building all targets
all: clean
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e cleanobj
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e nrf51422_xxac_s130

#target for printing all targets
help:
	@echo following targets are available:
	@echo 	nrf51422_xxac_s130
	@echo 	flash_softdevice

C_SOURCE_FILE_NAMES = $(notdir $(C_SOURCE_FILES))
C_PATHS = $(call remduplicates, $(dir $(C_SOURCE_FILES) ) )
C_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(C_SOURCE_FILE_NAMES:.c=.o) )

ASM_SOURCE_FILE_NAMES = $(notdir $(ASM_SOURCE_FILES))
ASM_PATHS = $(call remduplicates, $(dir $(ASM_SOURCE_FILES) ))
ASM_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(ASM_SOURCE_FILE_NAMES:.s=.o) )

vpath %.c $(C_PATHS)
vpath %.s $(ASM_PATHS)

OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

nrf51422_xxac_s130: OUTPUT_FILENAME := nrf51422_xxac_s130
nrf51422_xxac_s130: LINKER_SCRIPT=ble_app_bps_gcc_nrf51.ld

nrf51422_xxac_s130: $(BUILD_DIRECTORIES) $(OBJECTS)
	@echo Linking target: $(OUTPUT_FILENAME).out
	$(NO_ECHO)$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -lm -o $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e finalize

## Create build directories
$(BUILD_DIRECTORIES):
	echo $(MAKEFILE_NAME)
	$(MK) $@

# Create objects from C SRC files
$(OBJECT_DIRECTORY)/%.o: %.c
	@echo Compiling file: $(notdir $<)
	$(NO_ECHO)$(CC) $(CFLAGS) $(INC_PATHS) -c -o $@ $<

# Assemble files
$(OBJECT_DIRECTORY)/%.o: %.s
	@echo Assembly file: $(notdir $<)
	$(NO_ECHO)$(CC) $(ASMFLAGS) $(INC_PATHS) -c -o $@ $<
# Link
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out: $(BUILD_DIRECTORIES) $(OBJECTS)
	@echo Linking target: $(OUTPUT_FILENAME).out
	$(NO_ECHO)$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -lm -o $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
## Create binary .bin file from the .out file
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin: $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	@echo Preparing: $(OUTPUT_FILENAME).bin
	$(NO_ECHO)$(OBJCOPY) -O binary $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin

## Create binary .hex file from the .out file
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex: $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	@echo Preparing: $(OUTPUT_FILENAME).hex
	$(NO_ECHO)$(OBJCOPY) -O ihex $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex

finalize: genbin genhex echosize

genbin:
	@echo Preparing: $(OUTPUT_FILENAME).bin
	$(NO_ECHO)$(OBJCOPY) -O binary $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin

## Create binary .hex file from the .out file
genhex: 
	@echo Preparing: $(OUTPUT_FILENAME).hex
	$(NO_ECHO)$(OBJCOPY) -O ihex $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex
echosize:
	-@echo ''
	$(NO_ECHO)$(SIZE) $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	-@echo ''

clean:
	$(RM) $(BUILD_DIRECTORIES)

cleanobj:
	$(RM) $(BUILD_DIRECTORIES)/*.o
flash: nrf51422_xxac_s130
	@echo Flashing: $(OUTPUT_BINARY_DIRECTORY)/$<.hex
	nrfjprog --program $(OUTPUT_BINARY_DIRECTORY)/$<.hex -f nrf51  --sectorerase
	nrfjprog --reset -f nrf51

## Flash softdevice
flash_softdevice:
	@echo Flashing: s130_nrf51_2.0.0_softdevice.hex
	nrfjprog --program $(abspath $(NRF51_SDK_DIR)/components/softdevice/s130/hex/s130_nrf51_2.0.0_softdevice.hex) -f nrf51 --chiperase
	nrfjprog --reset -f nrf51
