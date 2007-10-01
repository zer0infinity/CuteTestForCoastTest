#!/bin/sh

g++ -c *.cpp
ld -r -o tstfw.o Test.o TestCase.o TString.o TestLocation.o TestList.o TestLocList.o TestSuite.o
ld -r -o textui.o TestRunner.o TfWMain.o TextTestResult.o

ar rv tstfw.a tstfw.o
ar rv testfw.a tstfw.o textui.o

# Use testfw.a to build your own test application. If you want to create your own
# UI use tstfw.a and look at the sourcefiles of TEXTUIFILES as a starting point.

