# Diretórios de acesso 

INC=../../include -I/usr/include/nvidia-current
LIB=../../lib -L/usr/lib/nvidia-current
BIN=.

INCLUDES= -I$(INC) -I$(OPF_DIR)/include

#/usr/lib32/nvidia-current/

LIBS=-lOpenCL

ifeq ($(PROC_TYPE),)
	CFLAGS+=-m32
else
	CFLAGS+=-m64
endif
ifdef AMDAPPSDKROOT
	INCLUDES+= -I$(AMDAPPSDKROOT)/include
	ifneq ($(PROC_TYPE),)
		LIB+= -L$(AMDAPPSDKROOT)/lib/x86
	else
		LIB+= -L$(AMDAPPSDKROOT)/lib/x86_64
	endif
#else
#	if [[ -d /usr/lib/nvidia-current/ && -d /usr/include/nvidia-current ]] then
#		INC_DIRS=. /usr/include/nvidia-current
#		LIB_DIRS=/usr/lib/nvidia-current
#	fi
endif

LIB+= -lOpenCL

FLAGS= -g -Wall 
#FLAGS= -O3 -Wall

$(LIB)/libift.a:
	cd ../../; make ; cd -

#LIB+=-L$(LIB_DIRS) -lOpenCL

$@.c: $(LIB)/libift.a $@.c 
	make
	gcc $(FLAGS) $@.c -o $(BIN)/$@ $(INCLUDES) -L$(LIB) -lift -lm 











