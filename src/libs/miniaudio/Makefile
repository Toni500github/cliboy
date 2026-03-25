CC	?= cc
SRC 	 = miniaudio.c
TARGET   = miniaudio.o
CFLAGS 	 = -I../../../include/libs $(LTO_FLAGS) -fvisibility=hidden -fPIC
CFLAGS += -DMA_NO_ENCODING \
          -DMA_ENABLE_ONLY_SPECIFIC_BACKENDS \
          -DMA_ENABLE_ALSA \
          -DMA_ENABLE_OSS \
          -DMA_ENABLE_JACK \
          -DMA_ENABLE_SNDIO \
          -DMA_ENABLE_WASAPI \
          -DMA_ENABLE_COREAUDIO

all: $(TARGET)

$(TARGET):
	$(CC) $(SRC) $(CFLAGS) -c -o ../../../$(BUILDDIR)/$@

clean:
	rm -rf ../../../$(BUILDDIR)/$(TARGET)

.PHONY: $(TARGET) clean all
