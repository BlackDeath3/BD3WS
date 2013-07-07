#!/bin/bash

# Attempt compilation.
echo -e "Compiling...\n"
gcc -g -w -std=gnu99 -pthread -o BD3WS BD3WS.c

# Successful compile. Run the program.
if [ $? -eq 0 ]; then
	echo -e "Successful compilation! Executing program...\n"
	echo -e "--------------------------------------------\n\n"
	if [ "$1" = "-d" ]; then
		gdb BD3WS
	else
		./BD3WS
	fi
# Failed compile. Do not run the program.
else
	echo -e "\n\nFailed to compile!\n"
fi