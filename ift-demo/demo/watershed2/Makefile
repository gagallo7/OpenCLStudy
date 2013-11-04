# Diretórios de acesso 

INC=../../include
LIB=../../lib
BIN=.

INCLUDES= -I$(INC) -I$(OPF_DIR)/include

#FLAGS= -g -Wall 
FLAGS= -O3 -Wall
#FLAGS= -O0 -g -pg

$(LIB)/libift.a:
	cd ../../; make ; cd -


 $@.c: $(LIB)/libift.a $@.c 
	make
	gcc $(FLAGS) $@.c -o $(BIN)/$@ $(INCLUDES) -L$(LIB) -lift -lm -lOpenCL

all:
	cd ../../; make clean && make ; cd -
	make watershed

clean:
	rm watershed

run:
	cd ../../; make ; cd -
	rm watershed
	make watershed
	./watershed flower.pgm flower_grad.pgm flower-seeds.txt


rerun:
	cd ../../; make clean && make ; cd -
	make watershed
	./watershed flower.pgm flower_grad.pgm flower-seeds.txt

gdb:
	gdb --args ./watershed flower.pgm flower_grad.pgm flower-seeds.txt

sf:
	gdb --batch -x .dbg --args ./watershed flower.pgm flower_grad.pgm flower-seeds.txt

view:
	make run
	eog flower_result*.ppm

comp:
	gwenview fr.ppm flower_result.ppm &
