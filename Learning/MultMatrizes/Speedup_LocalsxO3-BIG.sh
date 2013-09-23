#!/bin/bash
let n=8192*2
rep=3
printf "Tamanho da Entrada,Tempo Serial (ms),Tempo Paralelo (ms),Speedup\n" >> logO3.csv 
while [ $n -lt 50000 ]; do
	printf "Executando multiplicação de matrizes quadradas de tamanho $n\n------\n"
	i=0
	mSerial=0
: '
	while [ $i -lt $rep ]; do
# Evitando repetir os mesmos resultados demorados...
#		tSerial=$(serializada/mm $n |tail -1)
		tSerial=$(serializada/mmO3 $n |tail -1)
		echo "$tSerial"
		#let mSerial+=$tSerial
		mSerial=$(awk "BEGIN {print $mSerial + $tSerial}")
		let i+=1
	done
#	let mSerial/=$rep
	mSerial=$(awk "BEGIN {print $mSerial/$rep}")
	
	echo "---------------------"
	echo "Tamanho $n: Média Serial de $rep execuções: $mSerial ms"
	echo "---------------------"
	echo "+++++++++++++++++++++"
'

	i=0
	mParalelo=0
	while [ $i -lt $rep ]; do
		cd MultMatrixLocals
		tParalelo=$(./mmCPU $n |tail -1)
		cd ..
		echo "$tParalelo"
#		mParalelo=$(perl -e 'print $tParalelo + $mParalelo')
#		let mParalelo=$(echo "scale = 2; 20 * 100 / 30" | bc)
		mParalelo=$(awk "BEGIN {print $mParalelo + $tParalelo}")
#		echo $tParalelo
		let i+=1
	done
	mParalelo=$(awk "BEGIN {print 1000*$mParalelo/$rep}")
	echo -e
	echo "---------------------"
	echo "Tamanho $n: Média Paralelo de $rep execuções: $mParalelo ms"
	echo "---------------------"
	echo "+++++++++++++++++++++"
	
	i=0
	mGPU=0
	while [ $i -lt $rep ]; do
		cd MultMatrixLocals
		tGPU=$(./mmGPU $n |tail -1)
		cd ..
		echo "$tGPU"
		mGPU=$(awk "BEGIN {print $mGPU + $tGPU}")
		let i+=1
	done
	mGPU=$(awk "BEGIN {print 1000*$mGPU/$rep}")
	echo -e
	echo "---------------------"
	echo "Tamanho $n: Média GPU de $rep execuções: $mGPU ms"
	echo "---------------------"
	echo "+++++++++++++++++++++"
	
	resultCPU=$(echo "$tSerial / $tParalelo" |bc -l)
	resultGPU=$(echo "$tSerial / $tGPU" |bc -l)
	echo "SpeedupCPU = $resultCPU"
	echo "SpeedupGPU = $resultGPU"
	printf "$n,$mSerial,$mParalelo,$mGPU,$resultCPU,$resultGPU\n" >> logO3.csv 
	
	let n*=2
done
