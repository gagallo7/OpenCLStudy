import os

arqs = os.listdir("../Serial/entrada")

print(arqs[0])

os.chdir("..")

for a in arqs: 
	os.system("./dj2 Serial/entrada/" + a + " " + a[0:-3])
	os.system("./Serial/moco/djs < Serial/entrada/"+a+" logs/serial/"+a[0:-3])
	os.system("echo "+a+" >> diffs")
	os.system("diff logs/"+a[0:-2]+ " logs/serial/"+a[0:-3]+" >> diffs")
