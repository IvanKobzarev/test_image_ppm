#!/bin/bash
echo "\n\n\nCheck OK inputs"
for F in $(find test_input -name input_ok_*.ppm)
do
  echo "\ninput:$F"
  ./build/pipeline -i $F
  if [ $? -eq 0 ]
  then
    echo "TEST PASS"
  else
    echo "TEST FAIL"
  fi
done

echo "\n\n\nCheck FAIL inputs:"
for F in $(find test_input -name input_fail_*.ppm)
do
  echo "\ninput:$F"
  ./build/pipeline -i $F
  if [ $? -eq 0 ]
  then
    echo "TEST FAIL"
  else
    echo "TEST PASS"
  fi
done
