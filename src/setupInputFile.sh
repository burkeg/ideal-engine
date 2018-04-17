seq $1 > input.dat
paste input.dat input.dat > tmp
cp tmp input.dat
rm tmp
