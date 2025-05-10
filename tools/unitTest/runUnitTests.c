/**
 * @file runUnitTests.c
 * 
 * @brief Run all unit tests
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "libkael/debug/kaelMacros.h"

#include "./include/kaelTreeMemUnit.h"
#include "./include/kaelRandUnit.h"



void unitTest_runTests(){
	void(*unitTest_func[])() = {
		kaelTree_unit		,
		kaelRand_unit		,
	};
	uint16_t unitTestCount = sizeof(unitTest_func)/sizeof(unitTest_func[0]);

	for(uint16_t i=0; i<unitTestCount; i++){
		unitTest_func[i]();
	}
}


int main(){
	unitTest_runTests();
	
   return 0;
}


