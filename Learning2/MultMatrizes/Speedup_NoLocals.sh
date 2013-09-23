#!/bin/bash
let n=4096*2
rep=25
printf "Tamanho da Entrada,Tempo Serial (ms),Tempo Paralelo (ms),Speedup\n" >> log.csv 
while [ $n -lt 10000 ]; do
	printf "Executando multiplicação de matrizes quadradas de tamanho $n\n------\n"

	i=0
	mSerial=0
	while [ $i -lt $rep ]; do
		tSerial=$(serializada/mm $n |tail -1)
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
		cd MultMatrixNoLocals
		tParalelo=$(./mmCPU $n |tail -1)
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
	
	i=0
	mGPU=0
	while [ $i -lt $rep ]; do
		cd MultMatrixNoLocals
		tGPU=$(./mmGPU $n |tail -1)
		cd ..
		echo "$tGPU"
		mGPU=$(awk "BEGIN {print $mGPU + $tGPU}")
		let i+=1
	done
	mGPU=$(awk "BEGIN {print $mGPU/$rep}")
	echo -e
	echo "---------------------"
	echo "Tamanho $n: Média GPU de $rep execuções: $mGPU ms"
	echo "---------------------"
	echo "+++++++++++++++++++++"
	
	resultCPU=$(echo "$tSerial / $tParalelo" |bc -l)
	resultGPU=$(echo "$tSerial / $tGPU" |bc -l)
	echo "SpeedupCPU = $resultCPU"
	echo "SpeedupGPU = $resultGPU"
	printf "$n,$mSerial,$mParalelo,$mGPU,$resultCPU,$resultGPU\n" >> logNoLocals.csv 
	
	let n*=2
done
