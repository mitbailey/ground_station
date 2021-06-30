CC=gcc
CXX=g++
RM= /bin/rm -vf
PYTHON=python3
ARCH=UNDEFINED
PWD=$(shell pwd)
CDR=$(shell pwd)

EDCFLAGS:=$(CFLAGS)
EDLDFLAGS:=$(LDFLAGS)
EDDEBUG:=$(DEBUG)

ifeq ($(ARCH),UNDEFINED)
	ARCH=$(shell uname -m)
endif

UNAME_S := $(shell uname -s)

CXXFLAGS:= -I include/ -I imgui/include -I drivers/ -I imgui/include/imgui -I imgui/include/implot -I ./ -Wall -O2 -fpermissive
LIBS = 

ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	LIBS += -lGL `pkg-config --static --libs glfw3`

	CXXFLAGS += `pkg-config --cflags glfw3`
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"
	LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
	LIBS += -L/usr/local/lib -L/opt/local/lib
	#LIBS += -lglfw3
	LIBS += -lglfw

	CXXFLAGS += -I/usr/local/include -I/opt/local/include
	CFLAGS = $(CXXFLAGS)
endif

EDCFLAGS:= -O3 -Wall -std=gnu11 $(EDCFLAGS)
EDLDFLAGS:= -lm -lpthread -lncursest $(EDLDFLAGS)

LIBS += -lm -lpthread

# Uncomment once si446x_linux library is installed (`make install` in si446x_linux/) and `make` can be run in drivers/sw_upd_gs.
# BUILDDRV=drivers/sw_upd_gs/src/sw_update_gs.o

BUILDGUI=imgui/libimgui_glfw.a

BUILDCPP=guimain.o

GUITARGET=mtq_tester.out

all: $(GUITARGET)
	@echo Finished building $(GUITARGET) for $(ECHO_MESSAGE)
	sudo ./$(GUITARGET)

$(GUITARGET): $(BUILDDRV) $(BUILDGUI) $(BUILDCPP)
	$(CXX) $(BUILDDRV) $(BUILDCPP) -o $(GUITARGET) $(CXXFLAGS) $(BUILDGUI) $(LIBS)

$(BUILDGUI):
	cd $(PWD)/imgui && make -j$(nproc) implot=1 && cd $(PWD)

%.o: %.c
	$(CC) $(EDCFLAGS) $(EDDEBUG) -Iinclude/ -Idrivers/ -I./ -o $@ -c $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

.PHONY: clean

clean:
	$(RM) $(BUILDDRV)
	$(RM) $(GUITARGET)
	$(RM) $(BUILDCPP)

spotless: clean
	$(RM) -R build
	cd $(PWD)/imgui && make spotless && cd $(PWD)

