#include <stdio.h>
#include <stdint.h>

uint16_t kemuSys_u8Pack_t(uint8_t hi, uint8_t lo){
	return ((uint16_t)hi << 8U) | (lo & 0xFF);
}

int main() {
	uint8_t hi = 0x12;
	uint8_t lo = 0x34;

	uint16_t packed = kemuSys_u8Pack_t(hi, lo);

	printf("Packed value: 0x%04X\n", packed);

	return 0;
}
