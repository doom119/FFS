LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libfaac

FAAC_CODEC_SOURCES = libfaac
LOCAL_SRC_FILES := $(FAAC_CODEC_SOURCES)/aacquant.c \
	$(FAAC_CODEC_SOURCES)/backpred.c \
	$(FAAC_CODEC_SOURCES)/bitstream.c \
	$(FAAC_CODEC_SOURCES)/channels.c \
	$(FAAC_CODEC_SOURCES)/fft.c \
	$(FAAC_CODEC_SOURCES)/filtbank.c \
	$(FAAC_CODEC_SOURCES)/frame.c \
	$(FAAC_CODEC_SOURCES)/huffman.c \
	$(FAAC_CODEC_SOURCES)/ltp.c \
	$(FAAC_CODEC_SOURCES)/midside.c \
	$(FAAC_CODEC_SOURCES)/psychkni.c \
	$(FAAC_CODEC_SOURCES)/tns.c \
	$(FAAC_CODEC_SOURCES)/util.c

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include

include $(BUILD_STATIC_LIBRARY)