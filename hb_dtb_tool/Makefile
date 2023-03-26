
ifeq (${BUILD_OUTPUT_PATH},)
export BUILD_OUTPUT_PATH=$(shell pwd)
endif
CC = $(CROSS_COMPILE)gcc
TARGET = ${BUILD_OUTPUT_PATH}/hb_dtb_tool
SRCS = ./hb_dtb_tool.c
all:$(TARGET)

$(TARGET):$(SRCS)
	$(CC) -o $(TARGET) $(SRCS)

clean:
	rm -rf $(TARGET)