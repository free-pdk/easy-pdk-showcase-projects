SDCC        ?= sdcc 

#setup targets PMS150C, PFS154, PFS172, PFS173
ifeq ($(MAKECMDGOALS),PMS150C)
ICTYPE      = pdk13
ICNAME      = PMS150C
endif
ifeq ($(MAKECMDGOALS),PFS154)
ICTYPE      = pdk14
ICNAME      = PFS154
endif
ifeq ($(MAKECMDGOALS),PFS172)
ICTYPE      = pdk14
ICNAME      = PFS172
endif
ifeq ($(MAKECMDGOALS),PFS173)
ICTYPE      = pdk15
ICNAME      = PFS173
endif

OUTDIR=build
$(shell mkdir -p $(OUTDIR))

TARGET=$(OUTDIR)/ws2812animation16x16.$(ICNAME).ihx
SRCS := pdkdelay.c pdkws2812.c ws2812animation16x16.c

CFLAGS =-m$(ICTYPE) -D$(ICNAME)
LDFLAGS=-m$(ICTYPE)

OBJS = $(addprefix $(OUTDIR)/, $(SRCS:.c=.$(ICNAME).rel))

all:
	$(MAKE) PMS150C 
	$(MAKE) PFS154
	$(MAKE) PFS172
	$(MAKE) PFS173

PMS150C: $(TARGET)
PFS154:  $(TARGET)
PFS172:  $(TARGET)
PFS173:  $(TARGET)

$(OUTDIR)/%.$(ICNAME).rel: %.c .FORCE
	$(SDCC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(SDCC) $(LDFLAGS) $(OBJS) -o $@

clean:
	rm -rf $(OUTDIR)

.FORCE:

.PHONY: all clean PFS173 PFS172 PFS154 PMS150C
