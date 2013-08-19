import os

arqs = os.listdir("../entradas/gigantes/")
arqs.sort()

print(arqs)

os.chdir("..")

for a in arqs: 
	for i in range(25):
		os.system("echo -n \"" + a + ", \" >> Speedup.csv")
		cmd = "./Serial/moco/djs < entradas/gigantes/" + a + " >> Speedup.csv"
		os.system(cmd)
		print(cmd)

		os.system("echo -n \", \" >> Speedup.csv")
		cmd = "./dj2 entradas/gigantes/" + a + " " + a + " | tail -1 >> Speedup.csv"
		print(cmd)
		os.system(cmd)
'''

	os.system("./Serial/moco/djs < Serial/entrada/"+a+" logs/serial/"+a[0:-3])
	os.system("echo "+a+" >> diffs")
	os.system("diff logs/"+a[0:-2]+ " logs/serial/"+a[0:-3]+" >> diffs")
'''
