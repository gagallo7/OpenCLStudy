#!/bin/bash
n=8
rep=25
printf "Tamanho da Entrada,Tempo Serial sem O3 (ms)" >> log_NO_O3.csv
while [ $n -lt 10000 ]; do
	printf "Executando multiplicação de matrizes quadradas de tamanho $n\n------\n"

	i=0
	while [ $i -lt $rep ]; do
# Evitando repetir os mesmos resultados demorados...
#		tSerial=$(serializada/mm $n |tail -1)
		tSerial=$(serializada/mm $n |tail -1)
		echo "$tSerial"
		#let mSerial+=$tSerial

		printf "$n, $tSerial\n" >> log_NO_O3.csv
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
