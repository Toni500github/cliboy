CC	?= cc
SRC 	 = miniaudio.c
TARGET   = miniaudio.o
CFLAGS 	 = -I../../../include/libs $(LTO_FLAGS) -fvisibility=hidden -fPIC
CFLAGS += -DMA_NO_ENCODING

all: $(TARGET)

$(TARGET):
	$(CC) $(SRC) $(CFLAGS) -c -o ../../../$(BUILDDIR)/$@

clean:
	rm -rf ../../../$(BUILDDIR)/$(TARGET)

.PHONY: $(TARGET) clean all
