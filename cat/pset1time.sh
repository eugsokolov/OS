#!/bin/bash

clear;
gcc -Wall copycat.c -o pset1;
i=1
while ["$i" < "256000000"]; do
	./pset1 -o out -b i test;
	i = $i ^ 2
	echo time ./
done
