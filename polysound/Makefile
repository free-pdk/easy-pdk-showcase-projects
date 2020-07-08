SDCC        ?= sdcc 

#setup targets PFS173
ifeq ($(MAKECMDGOALS),PFS173)
ICTYPE      = pdk15
ICNAME      = PFS173
endif

OUTDIR=build
$(shell mkdir -p $(OUTDIR))

TARGET=$(OUTDIR)/polysound.$(ICNAME).ihx
SRCS := polysound.c

CFLAGS =-m$(ICTYPE) -D$(ICNAME)
LDFLAGS=-m$(ICTYPE)

OBJS = $(addprefix $(OUTDIR)/, $(SRCS:.c=.$(ICNAME).rel))

all:
	$(MAKE) PFS173

PFS173:  $(TARGET)

$(OUTDIR)/%.$(ICNAME).rel: %.c .FORCE
	$(SDCC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(SDCC) $(LDFLAGS) $(OBJS) -o $@

clean:
	rm -rf $(OUTDIR)

.FORCE:

.PHONY: all clean PFS173
