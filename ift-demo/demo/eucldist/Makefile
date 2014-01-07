# Diretórios de acesso 

INC=../../include -I/usr/include/nvidia-current
LIB=../../lib -L/usr/lib/nvidia-current -lOpenCL
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
	INC_DIRS=. $(AMDAPPSDKROOT)/include
	ifeq ($(PROC_TYPE),)
		LIB_DIRS=$(AMDAPPSDKROOT)/lib/x86
	else
		LIB_DIRS=$(AMDAPPSDKROOT)/lib/x86_64
	endif
#else
#	if [[ -d /usr/lib/nvidia-current/ && -d /usr/include/nvidia-current ]] then
#		INC_DIRS=. /usr/include/nvidia-current
#		LIB_DIRS=/usr/lib/nvidia-current
#	fi
endif



FLAGS= -g -Wall 
#FLAGS= -O3 -Wall

$(LIB)/libift.a:
	cd ../../; make ; cd -

#LIB+=-L$(LIB_DIRS) -lOpenCL

$@.c: $(LIB)/libift.a $@.c 
	make
	gcc $(FLAGS) $@.c -o $(BIN)/$@ $(INCLUDES) -L$(LIB) -lift -lm 











