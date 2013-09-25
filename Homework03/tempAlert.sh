#!/bin/bash

if [ $# -eq 0 ]
then
	echo "Usage: ./tempAlert.sh <addr_A> <addr_B> <temp_A> <temp_B> <gpio_no>"
	echo "No addresses or temperatures supplied; exiting."
	exit
fi

AddrA=$1
AddrB=$2
TempA=$3
TempB=$4
gpio=$5

cleanup(){
	echo $gpio > /sys/class/gpio/unexport
	exit
}	

echo $gpio > /sys/class/gpio/export
echo in > /sys/class/gpio/gpio${gpio}/direction

trap cleanup SIGINT

i2cset -y 1 0x${AddrA} 2 0x${TempB}
i2cset -y 1 0x${AddrA} 3 0x${TempA}

i2cset -y 1 0x${AddrB} 2 0x${TempB}
i2cset -y 1 0x${AddrB} 3 0x${TempA}

while [	"1" = "1" ];
do
	if [ "0" = `cat /sys/class/gpio/gpio${gpio}/value` ];
	then
		echo Temperature
		#Initial Temperatures
		temp1=`i2cget -y 1 0x${AddrA} 0`
		temp2=`i2cget -y 1 0x${AddrB} 0`
		#Translate to decimal Celsius
		temp1=$(printf "lower temp: %d\n" ${temp1})
		temp2=$(printf "upper temp: %d\n" ${temp2})
		# convert to F
		#temp1=$((temp1 * 3 / 2 + 32))
		#temp2=$((temp2 * 3 / 2 + 32))
		#print
		echo $temp1
		echo $temp2
	fi
done
