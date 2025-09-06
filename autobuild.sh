#!/bin/bash


rm -rf ${PWD}/build/*
cd ${PWD}/build
cmake ..
make
