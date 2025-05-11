/**
 * @file instructions.c
 * 
 * @brief header, list of CPU instructions. Avoid declaring in global space
 */

#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"

typedef enum {
	R0, R1, R2, R3, R4, R5, R6, R7, //registers 
	PC, SP, //program counter, stack pointer 
	LD,ST, //Load, store

	JMP, //jump 
	TRM, //terminate

	ADD, SUB, MUL, DIV,

	SHL, SHR, //shift left right
	AND, OR,
	NOP, //No operation
}KemuSys_ins;

#pragma GCC diagnostic pop