# Diretórios de acesso 

INC=../../include
LIB=../../lib
LIBS= -L$(LIB) -L/opt/AMDAPP/lib/x86_64/
BIN=.

INCLUDES= -I$(INC) -I$(OPF_DIR)/include -I/opt/AMDAPP/include

FLAGS= -g -Wall 
#FLAGS= -O3 -Wall

$(LIB)/libift.a:
	cd ../../; make ; cd -


 $@.c: $(LIB)/libift.a $@.c 
	make
	gcc $(FLAGS) $@.c -o $(BIN)/$@ $(INCLUDES) $(LIBS)  -lift -lm -lOpenCL

all:
	cd ../../; make clean && make ; cd -
	make watershed
	cd tools; make ; cd -

clean:
	rm watershed

run:
	cd ../../; make ; cd -
	rm watershed
	make watershed
	./watershed images/flower.pgm images/flower_grad.pgm images/flower-seeds.txt


rerun:
	cd ../../; make clean && make ; cd -
	make watershed
	./watershed images/flower.pgm images/flower_grad.pgm images/flower-seeds.txt

gdb:
	gdb --args ./watershed images/flower.pgm images/flower_grad.pgm images/flower-seeds.txt

sf:
	gdb --batch -x dbg --args ./watershed images/flower.pgm images/flower_grad.pgm images/flower-seeds.txt

sfhd:
	gdb --batch -x dbg --args ./watershed images/mPicchu.pgm images/mPicchu_grad.pgm images/mPicchu_seeds.txt

view:
	make run
	eog images/flower_result*.ppm
