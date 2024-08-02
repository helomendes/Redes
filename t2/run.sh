#!/bin/bash

GEOMETRIES=("80x24+100+100" "80x24+500+100" "80x24+100+400" "80x24+500+400")

for I in {1..4}
do
	echo ${I} > ${I}
	gnome-terminal --title=Player${I} --geometry=${GEOMETRIES[${I}-1]} -- bash -c " python3 main.py < ${I}; exec bash"
	rm ${I}
done
