#Running multiple programs

./test 1 a b &
P1=$!
echo $P1 1
./test 2 c d &
P2=$!
echo $P2 2
./test 3 e f &
P3=$!
echo $P3 3
./test 4 g h &
P4=$!
echo $P4 4 
./test 5 k l &
P5=$!
echo $P5 5

wait $P1 $P2 $P3 $P4 $P5 