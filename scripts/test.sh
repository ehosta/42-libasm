#!/bin/sh

make
make -C tester
make vg -C tester
