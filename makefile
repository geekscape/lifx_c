CFLAGS += -Iinclude
CFLAGS += -Ivendor/aiko_engine/include

OBJECTS =  examples/unix/lifx_dashboard.o
OBJECTS += src/lifx_message.o
OBJECTS += src/lifx_protocol.o
OBJECTS += vendor/aiko_engine/src/common/aiko_engine.o
OBJECTS += vendor/aiko_engine/src/unix/network.o
OBJECTS += vendor/aiko_engine/src/unix/serial.o
OBJECTS += vendor/aiko_engine/src/unix/timer.o

lifx_dashboard:	$(OBJECTS)
	gcc $^ -o $@

$(OBJECTS):	\
	include/lifx.h                                  \
	include/lifx_message.h                          \
	vendor/aiko_engine/include/aiko_engine.h        \
	vendor/aiko_engine/include/aiko_compatibility.h \
	vendor/aiko_engine/include/aiko_network.h       \
	vendor/aiko_engine/include/aiko_serial.h

clean:
	-rm -f $(OBJECTS)

clobber:	clean
	-rm -f lifx_dashboard

help :
	@echo ""
	@echo "make         - Build lifx_dashboard"
	@echo "make clean   - Remove binaries"
	@echo "make clobber - Remove all generated files"
	@echo "make help    - Display usage"
	@echo ""
