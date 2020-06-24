CPP=g++
CUR_DIR=${CURDIR}
DIR=${CUR_DIR}/..
PIN_INCLUDE=$(DIR)/pin-3.13/source/include
PIN_EXTRA=$(DIR)/pin-3.13/extras
PIN_RUNTIME=$(DIR)/pin-3.13/ia32/runtime
CPPFLAGS_OBJ=-Wall -Werror -Wno-unknown-pragmas -D__PIN__=1 -DPIN_CRT=1 -fno-stack-protector -fno-exceptions -funwind-tables -fasynchronous-unwind-tables -fno-rtti -DTARGET_IA32 -DHOST_IA32 -DTARGET_LINUX -fabi-version=2 -O3 -fomit-frame-pointer -fno-strict-aliasing
CPPFLAGS_OBJ_INC=-I$(PIN_INCLUDE)/pin -I$(PIN_INCLUDE)/pin/gen -I$(PIN_EXTRA)/components/include -I$(PIN_EXTRA)/xed-ia32/include/xed -I$(PIN_INCLUDE)/../tools/Utils -I$(PIN_INCLUDE)/../tools/InstLib
CPPFLAGS_OBJ_ISYS=-isystem $(PIN_EXTRA)/stlport/include -isystem $(PIN_EXTRA)/libstdc++/include -isystem $(PIN_EXTRA)/crt/include -isystem $(PIN_EXTRA)/crt/include/arch-x86 -isystem $(PIN_EXTRA)/crt/include/kernel/uapi -isystem $(PIN_EXTRA)/crt/include/kernel/uapi/asm-x86
CPPFLAGS_SO=-shared -Wl,--hash-style=sysv $(PIN_RUNTIME)/pincrt/crtbeginS.o -Wl,-Bsymbolic -Wl,--version-script=$(PIN_INCLUDE)/pin/pintool.ver -fabi-version=2
CPPFLAGS_SO_DIR=-L$(PIN_RUNTIME)/pincrt -L$(PIN_RUNTIME)/../lib -L$(PIN_RUNTIME)/../lib-ext -L$(PIN_EXTRA)/xed-ia32/lib
CPPFLAGS_SO_L=-lpin -lxed $(PIN_RUNTIME)/pincrt/crtendS.o -lpin3dwarf  -ldl-dynamic -nostdlib -lstlport-dynamic -lm-dynamic -lc-dynamic -lunwind-dynamic

all: BIN_LOG_DIR btrace.so

BIN_LOG_DIR:
	mkdir -p $(CUR_DIR)/bin $(CUR_DIR)/log

btrace.so: btrace.o handler.o
	$(CPP) $(CPPFLAGS_SO) -o $(CUR_DIR)/bin/btrace.so $(CUR_DIR)/bin/btrace.o $(CUR_DIR)/bin/handler.o $(CPPFLAGS_SO_DIR) $(CPPFLAGS_SO_L)

btrace.o: btrace.cpp
	$(CPP) $(CPPFLAGS_OBJ) $(CPPFLAGS_OBJ_INC) $(CPPFLAGS_OBJ_ISYS) -c -o $(CUR_DIR)/bin/btrace.o btrace.cpp

handler.o: handler.cpp handler.hpp
	$(CPP) $(CPPFLAGS_OBJ) $(CPPFLAGS_OBJ_INC) $(CPPFLAGS_OBJ_ISYS) -c -o $(CUR_DIR)/bin/handler.o handler.cpp

clean:
	rm -rf bin
	rm -rf log