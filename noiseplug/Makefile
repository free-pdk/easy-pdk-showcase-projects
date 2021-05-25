SDASPDK13   ?= sdaspdk13
SDASPDK14   ?= sdaspdk14
SDASPDK15   ?= sdaspdk15
SDLD        ?= sdldpdk

#setup targets PMS150C, PFS154, PFS172, PFS173
ifeq ($(MAKECMDGOALS),PMS150C)
ICNAME      = PMS150C
SDAS        = $(SDASPDK13)
endif
ifeq ($(MAKECMDGOALS),PFS154)
ICNAME      = PFS154
SDAS        = $(SDASPDK14)
endif
ifeq ($(MAKECMDGOALS),PFS172)
ICNAME      = PFS172
SDAS        = $(SDASPDK14)
endif
ifeq ($(MAKECMDGOALS),PFS173)
ICNAME      = PFS173
SDAS        = $(SDASPDK15)
endif

OUTDIR=build
$(shell mkdir -p $(OUTDIR))

TARGET=$(OUTDIR)/noiseplug.$(ICNAME).ihx
SRCS := noiseplug.s

AFLAGS = -pblos
LDFLAGS= -muwie

OBJS = $(addprefix $(OUTDIR)/, $(SRCS:.s=.$(ICNAME).rel))

all:
	$(MAKE) PMS150C
	$(MAKE) PFS154
	$(MAKE) PFS172
	$(MAKE) PFS173

PMS150C: $(TARGET)
PFS154:  $(TARGET)
PFS172:  $(TARGET)
PFS173:  $(TARGET)

$(OUTDIR)/%.$(ICNAME).rel: %.s .FORCE
	cat easypdk/$(ICNAME).s $< > $(OUTDIR)/$(ICNAME)$<
	$(SDAS) $(AFLAGS) $@  $(OUTDIR)/$(ICNAME)$<

$(TARGET): $(OBJS)
	$(SDLD) $(LDFLAGS) $@ $(OBJS) 

clean:
	rm -rf $(OUTDIR)

.FORCE:

.PHONY: all clean PFS173 PFS172 PFS154 PMS150C
