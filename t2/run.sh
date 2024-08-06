#!/bin/bash

for I in {1..4}
do
	echo ${I} > ${I}
	gnome-terminal -- " python3 main.py"
	rm ${I}
done
