# Diretórios de acesso 

INC=../../include
LIB=../../lib
BIN=.

INCLUDES= -I$(INC) -I$(OPF_DIR)/include

FLAGS= -g -Wall 
#FLAGS= -O3 -Wall

$(LIB)/libift.a:
	cd ../../; make ; cd -


$@.c: $(LIB)/libift.a $@.c 
	make
	gcc $(FLAGS) $@.c -o $(BIN)/$@ $(INCLUDES) -L$(LIB) -lift -lm -lOpenCL


all:
	cd ../../; make clean && make ; cd -
	make watershed
	cd tools; make ; cd -

clean:
	touch watershed
	rm watershed


run: clean
	cd ../../; make ; cd -
	rm -f watershed
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

%: images/%.pgm clean all
	@echo $@
	make watershed
	./watershed images/$@.pgm images/$@_grad.pgm images/$@-seeds.txt
	eog images/$@_result.ppm

%: images/blocks/%.pgm clean
	@echo $@
	make watershed
	./watershed images/blocks/$@.pgm images/blocks/$@_grad.pgm images/blocks/$@-seeds.txt
	eog images/blocks/$@_result.ppm
