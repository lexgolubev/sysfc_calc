echo "$1" > /sys/kernel/calc/arg1
echo "$2" > /sys/kernel/calc/op
echo "$3" > /sys/kernel/calc/arg2

cat /sys/kernel/calc/answer
