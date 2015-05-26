#!/bin/bash

if [[ $# != 3 ]]; then
	echo "Usage : $0 infile layer size"
	echo "Example : $0 foo.mod 21 13.00mm"
	exit 0
fi

infile="$1"
layer="$2"
size="$3"
ssize="$(echo $size | sed 's/\./_/')"

case "$layer" in 
	15)
		layername="cutop";;
	0)
		layername="cubot";;
	21)
		layername="silktop";;
	20)
		layername="silkbot";;
	*)
		echo "Unknown layer number $layer"
esac 

name=${infile%.*}
outfile="${name}_${layername}_${ssize}.mod"
outname="${name}_${layername}_${ssize}"

echo "Scaling $infile in layer $layer to $size"
./scale.pl "$infile" "$outfile" "$layer" "$size"
cat "$outfile" | sed "s/LOGO/$outname/g" | sponge "$outfile"

