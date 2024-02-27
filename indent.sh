#!/bin/sh

usage(){
	echo "usage: indent *.[ch]" 1>&2;
	exit 1
}

case $# in
0)
	usage
esac

types(){
	fgrep typedef $1 | fgrep struct | sed -E 's/.*[ \t]([a-zA-Z_]+) *[;}].*/\1/g'| sort -u
}

for i in "$@"; do
	if ! echo $i|grep '.*\.[hc]' > /dev/null 2>&1; then
		echo "$i" is not a C file 1>&2;
		usage
	fi
	if ! test -f $i > /dev/null 2>&1; then
		echo "$i does not exist or is not a file" 1>&2;
		usage
	fi
	if ! file "$i" |grep 'C source' > /dev/null 2>&1; then
		echo "$i does not contain C code" 1>&2;
	else
		indent -bad -bap -nbc -bbo -hnl -br -brs -c33 -cd33 -ncdb -ce -ci4 -cli0 -d0 -di1 -nfc1 -i8 -ip0 -l100 -lp -npcs -nprs -npsl -sai -saf -saw -ncs -nsc -sob -nfca -cp33 -ss -ts8 -nut -il1 "$i"
		rm "$i"~
	fi
done
