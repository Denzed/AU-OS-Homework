#!/bin/bash

make clean
if (( $1 == "debug" ))
then
	make DEBUG=1
	qemu-system-x86_64 -kernel kernel -s -serial stdio &
	sleep 1
	gdb -x gdb_initial_commands kernel
	killall qemu-system-x86_64
else
	make 
	qemu-system-x86_64 -kernel kernel  -serial stdio
fi
