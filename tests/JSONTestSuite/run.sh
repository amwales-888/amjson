#!/bin/bash

if [ $# -ne 2 ]
then
    echo "Usage: $0 testdir jsonbin"
    exit 1
fi

TESTDIR=$1
JSONBIN=$2

SUCCESS=0
FAILURE=0

echo 
echo "--------------------------------------------------------------------"
echo "Running JSONTestSuite tests"
echo "--------------------------------------------------------------------"
echo 

for file in ${TESTDIR}/n_*
do
    ${JSONBIN} ${file} >/dev/null 2>&1
    if [ $? -eq 1 ]
    then
	echo "Test Success - Expected fail - ${file}"
	SUCCESS=$(( ${SUCCESS} + 1 ))
    else
	echo "Test Failure - Expected fail - ${file}"
	FAILURE=$(( ${FAILURE} + 1 ))
    fi
done

for file in ${TESTDIR}/y_*
do
    ${JSONBIN} ${file} >/dev/null 2>&1
    if [ $? -eq 0 ]
    then
	echo "Test Success - Expected pass - ${file}"
	SUCCESS=$(( ${SUCCESS} + 1 ))
    else
	echo "Test Failure - Expected pass - ${file}"
	FAILURE=$(( ${FAILURE} + 1 ))
    fi
done
    
echo
echo "--------------------------------------------------------------------"
echo "JSONTestSuite Failed ${FAILURE} out of $(( ${SUCCESS} + ${FAILURE} )) tests run."
echo "--------------------------------------------------------------------"

if [ ${FAILURE} -gt 0 ]
then
    exit 1
fi

exit 0
