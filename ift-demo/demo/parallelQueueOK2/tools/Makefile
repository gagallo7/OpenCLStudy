# Diretórios de acesso 

INC=../../../include
LIB=../../../lib
BIN=.

INCLUDES= -I$(INC) -I$(OPF_DIR)/include

FLAGS= -g -Wall 
#FLAGS= -O3 -Wall

all:
	gcc $(FLAGS) diffMarker.c -o $(BIN)/diffTool $(INCLUDES) -L$(LIB) -lift -lm -lOpenCL
	gcc $(FLAGS) seedMarker.c -o $(BIN)/seedTool $(INCLUDES) -L$(LIB) -lift -lm -lOpenCL
