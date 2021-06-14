#!/bin/bash
echo "\n\n\nCheck OK inputs"
for F in $(find test -name 'input_ok_[0-9].ppm')
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
rm test/input_ok_*_out.ppm

echo "\n\n\nCheck FAIL inputs:"
for F in $(find test -name input_fail_[0-9].ppm)
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

echo "\n\n\nCheck output conversion -16"


echo "Test: ./test/input_ok_0.ppm 8->16 == ./test/input_ok_2.ppm"
./build/pipeline -i ./test/input_ok_0.ppm -o ./build/output.ppm -16
if cmp -s "./test/input_ok_2.ppm" "./build/output.ppm"; then
  echo "PASS"
else
  echo "FAIL"
  diff ./test/input_ok_2.ppm ./build/output.ppm
fi


echo "Test: ./test/input_ok_0.ppm 16->8 == ./test/input_ok_0.ppm"
./build/pipeline -i ./test/input_ok_2.ppm -o ./build/output.ppm
if cmp -s "./test/input_ok_0.ppm" "./build/output.ppm"; then
  echo "PASS"
else
  echo "FAIL"
  diff ./test/input_ok_0.ppm ./build/output.ppm
fi

echo "Test: ./test/input_ok_0.ppm -gray == ./test/input_ok_4.ppm"
./build/pipeline -i ./test/input_ok_0.ppm -gray -o ./build/output.ppm
if cmp -s "./test/input_ok_4.ppm" "./build/output.ppm"; then
  echo "PASS"
else
  echo "FAIL"
  diff ./test/input_ok_4.ppm ./build/output.ppm
fi

echo "Test: ./test/input_ok_0.ppm -flip == ./test/input_ok_3.ppm"
./build/pipeline -i ./test/input_ok_0.ppm -flip -o ./build/output.ppm
if cmp -s "./test/input_ok_3.ppm" "./build/output.ppm"; then
  echo "PASS"
else
  echo "FAIL"
  diff ./test/input_ok_3.ppm ./build/output.ppm
fi
