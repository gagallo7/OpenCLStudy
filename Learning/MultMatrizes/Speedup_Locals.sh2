#!/bin/bash
n=8
rep=25
printf "Tamanho da Entrada,Tempo Serial (ms),Tempo Paralelo (ms),Speedup\n" >> log.csv 
while [ $n -lt 1024 ]; do
	printf "Executando multiplicação de matrizes quadradas de tamanho $n\n------\n"

	i=0
	mSerial=0
	while [ $i -lt $rep ]; do
		tSerial=$(serializada/mmSerial $n |tail -1)
		echo "$tSerial"
		let mSerial+=$tSerial
		let i+=1
	done
	let mSerial/=$rep
	echo "---------------------"
	echo "Tamanho $n: Média Serial de $rep execuções: $mSerial ms"
	echo "---------------------"
	echo "+++++++++++++++++++++"
	
	i=0
	mParalelo=0
	while [ $i -lt $rep ]; do
		cd MultMatrixLocals
		tParalelo=$(./mm $n |tail -1)
		cd ..
		echo "$tParalelo"
#		mParalelo=$(perl -e 'print $tParalelo + $mParalelo')
#		let mParalelo=$(echo "scale = 2; 20 * 100 / 30" | bc)
		mParalelo=$(awk "BEGIN {print $mParalelo + $tParalelo}")
#		echo $tParalelo
		let i+=1
	done
	mParalelo=$(awk "BEGIN {print $mParalelo/$rep}")
	echo -e
	echo "---------------------"
	echo "Tamanho $n: Média Paralelo de $rep execuções: $mParalelo ms"
	echo "---------------------"
	echo "+++++++++++++++++++++"
	
	result=$(echo "$tSerial / $tParalelo" |bc -l)
	echo "Speedup = $result"
	printf "$n,$mSerial,$mParalelo,$result\n" >> log.csv 
	
	let n*=2
done
