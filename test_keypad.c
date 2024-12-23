#include "crt0.c"
#include "ChrFont0.h"

void interrupt_handler() {}

int kypd_scan() {
	volatile int *ioa_ptr = (int *)0xff14;
	*ioa_ptr = 0x07;                /* 0111 */
	for (int i = 0; i < 1; i++);    /* Wait */
	if ((*ioa_ptr & 0x80) == 0)
		 return 0x1;
	if ((*ioa_ptr & 0x40) == 0)
		 return 0x4;
	if ((*ioa_ptr & 0x20) == 0)
		 return 0x7;
	if ((*ioa_ptr & 0x10) == 0)
		 return 0x0;
	*ioa_ptr = 0x0b;                /* 1011 */
	for (int i = 0; i < 1; i++);    /* Wait */
	if ((*ioa_ptr & 0x80) == 0)
		 return 0x2;
	if ((*ioa_ptr & 0x40) == 0)
		 return 0x5;
	if ((*ioa_ptr & 0x20) == 0)
		 return 0x8;
	if ((*ioa_ptr & 0x10) == 0)
		 return 0xf;
	*ioa_ptr = 0x0d;                /* 1101 */
	for (int i = 0; i < 1; i++);    /* Wait */
	if ((*ioa_ptr & 0x80) == 0)
		 return 0x3;
	if ((*ioa_ptr & 0x40) == 0)
		 return 0x6;
	if ((*ioa_ptr & 0x20) == 0)
		 return 0x9;
	if ((*ioa_ptr & 0x10) == 0)
		 return 0xe;
	*ioa_ptr = 0x0e;                /* 1110 */
	for (int i = 0; i < 1; i++);    /* Wait */
	if ((*ioa_ptr & 0x80) == 0)
		 return 0xa;
	if ((*ioa_ptr & 0x40) == 0)
		 return 0xb;
	if ((*ioa_ptr & 0x20) == 0)
		 return 0xc;
	if ((*ioa_ptr & 0x10) == 0)
		 return 0xd;
	return -1;
}

void main() {
	volatile int *seg7_ptr = (int *)0xff10;
	for (;;) *seg7_ptr = kypd_scan();
}

