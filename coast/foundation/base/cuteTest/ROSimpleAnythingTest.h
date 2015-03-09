#ifndef _ROSimpleAnythingTest_H
#define _ROSimpleAnythingTest_H

#include "cute.h"
#include "ide_listener.h"
#include "cute_runner.h"

class ROSimpleAnythingTest {
public:
	void runAllTests(int argc, char const *argv[]);
	void ConstructorsAndConversions();
	void AssignmentsAndConversions();

	void EmptyConstructor();
	void AnyConstructor();
	void AnyIntConstructor();
	void AnyBoolConstructor();
	void AnyLongConstructor();
	void AnyFloatConstructor();
	void AnyDoubleConstructor();
	void AnyIFAObjConstructor();
};

#endif		//not defined _ROSimpleAnythingTest_H
