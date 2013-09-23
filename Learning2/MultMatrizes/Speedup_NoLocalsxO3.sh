#!/bin/bash
n=8
rep=12
printf "Tamanho da Entrada,Tempo CPU (ms),Tempo GPU No Locals (ms),GPU com Locals\n" >> logO3xNoLocals_Completo.csv
while [ $n -lt 20000 ]; do
	printf "Executando multiplicação de matrizes quadradas de tamanho $n\n------\n"

	i=0
	while [ $i -lt $rep ]; do
# Evitando repetir os mesmos resultados demorados...
#		tSerial=$(serializada/mm $n |tail -1)
		#tSerial=$(serializada/mmO3 $n |tail -1)
		#echo "$tSerial"
		#let mSerial+=$tSerial

		cd MultMatrixNoLocals
		tParalelo=$(./mmCPU $n |tail -1)
		echo "$tParalelo"
#		mParalelo=$(perl -e 'print $tParalelo + $mParalelo')
#		let mParalelo=$(echo "scale = 2; 20 * 100 / 30" | bc)
#		echo $tParalelo

		tGPU=$(./mmGPU $n |tail -1)
		cd ..
		echo "$tGPU"

		cd MultMatrixLocals
		tGPU2=$(./mmGPU $n |tail -1)
		echo "$tGPU2"
		cd ..

		printf "$n, $tParalelo, $tGPU, $tGPU2\n" >> logO3xNoLocals_Completo.csv
		let i+=1
	done
	echo -e
	echo "---------------------"
	echo "Tamanho $n: Média GPU de $rep execuções"
	echo "---------------------"
	echo "+++++++++++++++++++++"
	
	let n*=2
done
