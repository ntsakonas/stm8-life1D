SDCC=sdcc
SDLD=sdld
OBJECTS=life.ihx

.PHONY: all clean flash

all: $(OBJECTS)

clean:
	rm -f $(OBJECTS)

flash: $(OBJECTS)
	stm8flash -c stlinkv2 -p stm8s103?3 -w $(OBJECTS)

%.ihx: %.c
	$(SDCC) -lstm8 -mstm8 --out-fmt-ihx $(CFLAGS) $(LDFLAGS) $<
