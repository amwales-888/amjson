#!/bin/bash

if [ $# -ne 2 ]
then
    echo "Usage: $0 testdir jsonbin"
    exit 1
fi

TESTDIR=$1
JSONBIN=$2

for file in ${TESTDIR}/datasets/1mb.json ${TESTDIR}/datasets/50mb.json ${TESTDIR}/datasets/100mb.json ${TESTDIR}/datasets/500mb.json
do
    echo 
    echo "--------------------------------------------------------------------"
    echo "Running performance test on ${file}"
    echo "--------------------------------------------------------------------"
    echo 

    echo >${TESTDIR}/result

    count=0
    while [ $count -lt 10 ]
    do
	${JSONBIN} ${file} --benchmark >/tmp/out
	if [ $? -ne 0 ]
	then
	    cat /tmp/out
	    echo "Failed running test."
	    echo "--------------------------------------------------------------------"
	    exit 1
	fi

	cat /tmp/out
	cat /tmp/out >>${TESTDIR}/result
	count=$(( $count + 1 ))
    done

    ELLAPSED=$( awk -F: '/Ellapsed/{ sum += $2; n++ } END { if (n > 0) print sum / n; }' ${TESTDIR}/result )

    echo
    echo "--------------------------------------------------------------------"
    echo "Average ellapsed time parsing '${TESTDIR}/datasets/out':${ELLAPSED}" 
    echo "--------------------------------------------------------------------"
    echo
done

exit 0
