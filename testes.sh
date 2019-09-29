echo "Execução serial"
./ts.sh > results/res1

echo "Execução paralela 2T"
./tp2.sh > results/res2

echo "Execução paralela 4T"
./tp4.sh > results/re4

echo "Execução paralela 8T"
./tp8.sh > results/res8

echo "Execução paralela 16T"
./tp16.sh > results/res16

echo "Execução paralela 32T"
./tp32.sh > results/res32
