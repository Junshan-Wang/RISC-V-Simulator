echo "add.c"
echo y | ./run.sh add.c 0 1
echo y | ./run.sh add.c 0 2

echo "mul-div.c"
echo y | ./run.sh mul-div.c 0 1
echo y | ./run.sh mul-div.c 0 2

echo "simple-fuction.c"
echo y | ./run.sh simple-fuction.c 0 1
echo y | ./run.sh simple-fuction.c 0 2

echo "qsort.c"
echo y | ./run.sh qsort.c 0 1
echo y | ./run.sh qsort.c 0 2

echo "n!.c"
echo y | ./run.sh n\!.c 0 1
echo y | ./run.sh n\!.c 0 2

for loop in {1..10}
do
	echo ${loop}".cpp"
	echo y | ./run.sh ${loop}".cpp" 0 1
	echo y | ./run.sh ${loop}".cpp" 0 2
done

