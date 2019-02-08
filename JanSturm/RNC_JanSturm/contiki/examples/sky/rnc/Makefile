CONTIKI = ../../..
ifndef TARGET
TARGET=sky
endif

MODULES+= core/net/seemoo-lab/rnc/
APPS += powertrace

all: test-rnc

%.tgz: %.ihex
	mkdir $(basename $<) ; \
	mv $< $(basename $<) ; \
	echo $(basename $<)/$(basename $<).ihex 600 > $(basename $<)/runfile ; \
	tar czf $@ $(basename $<)

CONTIKI_WITH_IPV4 = 0
CONTIKI_WITH_RIME = 1
include $(CONTIKI)/Makefile.include
