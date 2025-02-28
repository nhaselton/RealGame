#pragma once
#include "def.h"

ENT_CLASS TestClass{ //This class will show up in the result
public:
	EVAR int a;		//This variable will show up 
	int c;			//This wont show up
	EVAR float b;		//This variable will show up
	EVAR Vec3 d;			//This will show up

	//These wont show up	
	void Test();
	void Test2();
	void Test3();
};