#!/bin/bash
n=8192
rep=13
printf "Tamanho da Entrada,Tempo Serial (ms),Tempo Paralelo (ms),Speedup\n" >> logO3.csv 
while [ $n -lt 10000 ]; do
	printf "Executando multiplicação de matrizes quadradas de tamanho $n\n------\n"

	i=0
	while [ $i -lt $rep ]; do
# Evitando repetir os mesmos resultados demorados...
#		tSerial=$(serializada/mm $n |tail -1)
		tSerial=$(serializada/mmO3 $n |tail -1)
		echo "$tSerial"
		#let mSerial+=$tSerial

		cd MultMatrixLocals
		tParalelo=$(./mmCPU $n |tail -1)
		echo "$tParalelo"
#		mParalelo=$(perl -e 'print $tParalelo + $mParalelo')
#		let mParalelo=$(echo "scale = 2; 20 * 100 / 30" | bc)
#		echo $tParalelo

		tGPU=$(./mmGPU $n |tail -1)
		cd ..
		echo "$tGPU"


		printf "$n, $tSerial, $tParalelo, 0\n" >> logO3xLocals_Completo.csv
		let i+=1
	done
	echo -e
	echo "---------------------"
	echo "Tamanho $n: Média GPU de $rep execuções"
	echo "---------------------"
	echo "+++++++++++++++++++++"
	
	echo "SpeedupCPU = $resultCPU"
	echo "SpeedupGPU = $resultGPU"
	
	let n*=2
done
