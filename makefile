CFLAGS += -DMMEM_CONF_SIZE=256
CFLAGS += -Iinclude
CFLAGS += -Ivendor/aiko_engine/include
CFLAGS += -Ivendor/aiko_engine/examples/common/aiko_server

OBJECTS += src/lifx_extend.o
OBJECTS += src/lifx_message.o
OBJECTS += src/lifx_protocol.o
OBJECTS += vendor/aiko_engine/src/common/aiko_engine.o
OBJECTS += vendor/aiko_engine/src/unix/engine.o
OBJECTS += vendor/aiko_engine/src/unix/network.o
OBJECTS += vendor/aiko_engine/src/unix/serial.o
OBJECTS += vendor/aiko_engine/src/unix/store.o
OBJECTS += vendor/aiko_engine/src/unix/timer.o
OBJECTS += vendor/aiko_engine/src/unix/wifi.o
OBJECTS += vendor/aiko_engine/src/common/lisp/expression.o
OBJECTS += vendor/aiko_engine/src/common/lisp/interface.o
OBJECTS += vendor/aiko_engine/src/common/lisp/parser.o
OBJECTS += vendor/aiko_engine/src/common/lisp/primitives.o
OBJECTS += vendor/aiko_engine/src/common/lisp/utility.o
OBJECTS += vendor/aiko_engine/src/common/memory/list.o
OBJECTS += vendor/aiko_engine/src/common/memory/mmem.o
OBJECTS += vendor/aiko_engine/src/common/state/machine.o
OBJECTS += vendor/aiko_engine/examples/common/aiko_server/lisp_extend.o

LIFX_DASHBOARD_OBJECTS = examples/unix/lifx_dashboard.o

CONFIGURE_TARGETS_OBJECTS = examples/unix/configure_targets.o

all:	configure_targets lifx_dashboard

GIT_VERSION := $(shell git describe --abbrev=8 --dirty --always --tags)

version:	version_lifx
ifeq ("$(wildcard vendor/aiko_engine/makefile)","")
	@(git submodule update --init)
endif
	@(cd vendor/aiko_engine; $(MAKE) version)

version_lifx:
	@echo '#define LIFX_VERSION  "$(GIT_VERSION)"' >include/lifx_version.h

configure_targets:	version $(CONFIGURE_TARGETS_OBJECTS) $(OBJECTS)
	gcc $(filter %.o, $^) -o $@

lifx_dashboard:	version $(LIFX_DASHBOARD_OBJECTS) $(OBJECTS)
	gcc $(filter %.o, $^) -o $@

$(OBJECTS):	\
	include/lifx.h                                                \
	include/lifx_message.h                                        \
	vendor/aiko_engine/include/aiko_engine.h                      \
	vendor/aiko_engine/include/aiko_compatibility.h               \
	vendor/aiko_engine/include/aiko_network.h                     \
	vendor/aiko_engine/include/aiko_serial.h                      \
	vendor/aiko_engine/include/aiko_state.h                       \
	vendor/aiko_engine/include/aiko_store.h                       \
        vendor/aiko_engine/include/aiko_wifi.h                        \
        vendor/aiko_engine/include/lisp.h                             \
        vendor/aiko_engine/include/memory/list.h                      \
        vendor/aiko_engine/include/memory/mmem.h                      \
	vendor/aiko_engine/examples/common/aiko_server/lisp_extend.h

clean:
	-rm -f $(CONFIGURE_TARGETS_OBJECTS)
	-rm -f $(LIFX_DASHBOARD_OBJECTS)
	-rm -f $(OBJECTS) include/lifx_version.h

clobber:	clean
	-rm -f configure_targets lifx_dashboard

help :
	@echo ""
	@echo "make         - Build lifx_dashboard"
	@echo "make clean   - Remove binaries"
	@echo "make clobber - Remove all generated files"
	@echo "make help    - Display usage"
	@echo ""
