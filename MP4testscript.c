/* MP4testscript.c
 * Judson Cooper
 * Lab4: Dynamic Memory Allocation
 *
 * This file is a test scipt to quickly test the functionality of MP4. It
 * automatically runs several of the unit driver included in the lab4.c file,
 * repeating each driver using various settings.
 */

#include <stdio.h>
#include <stdlib.h>

int main(void) {
	// unit driver 0, first fit and no coalescing
	system("./lab4 -u 0");
	puts(" \n \n");
	
	// unit driver 1, first fit and no coalescing
	system("./lab4 -u 1");
	puts(" \n \n");
	
	// unit driver 2, first fit and no coalescing
	system("./lab4 -u 2");
	puts(" \n \n");
	
	// unit driver 3, first fit and no coalescing
	system("./lab4 -u 3");
	puts(" \n \n");
	
	// unit driver 4, first fit and no coalescing
	system("./lab4 -u 4");
	puts(" \n \n");
	
	// unit driver 2, best fit and coalescing
	system("./lab4 -u 2 -c -f best");
	puts(" \n \n");
	
	// unit driver 3, best fit and coalescing
	system("./lab4 -u 3 -c -f best");
	puts(" \n \n");
	
	// unit driver 4, best fit and coalescing
	system("./lab4 -u 4 -c -f best");
	puts(" \n \n");
	
	return 0;
}
