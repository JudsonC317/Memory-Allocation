/* mem.c 
 * Judson Cooper
 * Lab4: Dynamic Memory Allocation
 * 
 * Purpose: this file provides functions for management of a memory heap.
 * 	The functions are intended to be able to replace the standard malloc
 *		and free functions.
 *
 * Assumptions: Each block is to have a header that contains the size of
 * 	the block and a pointer to the next block. This header is omitted
 *		in the pointer that is passed to the user.
 *
 * Bugs: none known.
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>

#include "mem.h"

// Global variables required in mem.c only
static chunk_t Dummy = {&Dummy, 0};
static chunk_t * Rover = &Dummy;

int NumSbrkCalls = 0;
int P = 0; // number of pages of memory


// private function prototypes
void mem_validate(void);


/* function to request 1 or more pages from the operating system.
 *
 * new_bytes must be the number of bytes that are being requested from
 *           the OS with the sbrk command.  It must be an integer 
 *           multiple of the PAGESIZE
 *
 * returns a pointer to the new memory location.  If the request for
 * new memory fails this function simply returns NULL, and assumes some
 * calling function will handle the error condition.  Since the error
 * condition is catastrophic, nothing can be done but to terminate 
 * the program.
 */
chunk_t *morecore(int new_bytes) {
    char *cp;
    chunk_t *new_p;
    // preconditions
    assert(new_bytes % PAGESIZE == 0 && new_bytes > 0);
    assert(PAGESIZE % sizeof(chunk_t) == 0);
    cp = sbrk(new_bytes);
    if (cp == (char *) -1)  /* no space available */
        return NULL;
    new_p = (chunk_t *) cp;
    // You should add some code to count the number of calls
    // to sbrk, and the number of pages that have been requested
    NumSbrkCalls++;
	 P += new_bytes/PAGESIZE;
   
	 return new_p; }

/* deallocates the space pointed to by return_ptr; it does nothing if
 * return_ptr is NULL.  
 *
 * This function assumes that the Rover pointer has already been 
 * initialized and points to some memory block in the free list.
 */
void Mem_free(void *return_ptr) {
    // precondition
    assert(Rover != NULL && Rover->next != NULL);

	 if (return_ptr == NULL) {
		 return;
	 }

	 // get back to position of chunk_t header
	 chunk_t *newMem = (chunk_t*) return_ptr;
	 newMem--;

	 if (Coalescing == TRUE) {
		 int pointFound = 0; // 0 for no, 1 yes
		 chunk_t *originPoint = Rover;
		 chunk_t *previous = Rover;
		 Rover = Rover->next;

		 // find place to put newMem
		 do {
			 if ((newMem >= previous) && (newMem <= Rover)) {
				 // put between previous and Rover
				 previous->next = newMem;
				 newMem->next = Rover;
				 Rover = newMem; // so Rover points to newly added memory
				 pointFound = 1;
				 break;
			 }
			 if ((newMem >= previous) && (Rover <= previous)) {
				 // looped around entire list, or dummy is only entry
				 previous->next = newMem;
				 newMem->next = Rover;
				 Rover = newMem;
				 pointFound = 1;
				 break;
			 }
			 if (pointFound != 1) {
				// will mess up coalesce if newMem already inserted
			 	previous = previous->next;
			 }
			 Rover = Rover->next;
		 } while (previous != originPoint);
		 assert (pointFound == 1); // make sure successful
	
		// now coalesce if possible
		if (((newMem + newMem->size) == newMem->next) && (newMem->next->size != 0)) {
			// block after newMem is continuous and not the dummy
	 		newMem->size = newMem->size + newMem->next->size;
	 		newMem->next = newMem->next->next;
			pointFound = 1;
		}
		if (((previous + previous->size) == newMem) && (previous->size != 0)) {
	 		// block before newMem is continuous and not the dummy
		 	previous->size = previous->size + newMem->size;
		 	previous->next = newMem->next;
		 	Rover = previous; // so Rover ends pointing to new memory
			pointFound = 1;
		}
	 }

	 else if (Coalescing == FALSE) {
		 newMem->next = Rover->next;
		 Rover->next = newMem;
		 // so that Rover points to memory that was just returned
		 Rover = Rover->next;
	 }
	 

	 else {
		 puts("ERROR: Invalid Freeing Policy");
	 }

//	puts("*********************************");
//	 Mem_print(); // DEBUGGING
//	puts("*********************************");
}

/* returns a pointer to space for an object of size nbytes, or NULL if the
 * request cannot be satisfied.  The memory is uninitialized.
 *
 * This function assumes that there is a Rover pointer that points to
 * some item in the free list.  The first time the function is called,
 * Rover is null, and must be initialized with a dummy block whose size
 * is one, but set the size field to zero so this block can never be 
 * removed from the list.  After the first call, the Rover can never be null
 * again.
 */
void *Mem_alloc(const int nbytes) {
    // precondition
    assert(nbytes > 0);
    assert(Rover != NULL && Rover->next != NULL);

	 // convert nbytes to units 
	 // (round up to nearest mult of chunk_t, add 1 for header)
	 int nunits = (nbytes + sizeof(chunk_t) - 1) / sizeof(chunk_t) + 1; 

	 chunk_t *p = NULL; // pointer to mem block given to user
	 chunk_t *q = NULL; // pointer to address given to user
	 	 
	 // will hold beginning of circle for search
	 chunk_t *originPoint = Rover;
	 // needed for eventual removal of Rover from list
	 chunk_t *previous = NULL;
	 
    // find memory block
	 if (SearchPolicy == FIRST_FIT) {
		 while (Rover->size < nunits) {
		//	printf("****\nNeed: %d, Rover: %d\n****\n", nunits, Rover->size); //DEBUG
			 previous = Rover;
			 Rover = Rover->next;
			 if (Rover == originPoint) {
			 	 // entire circle searched, must make more memory
				 int neededPages = ((nunits*sizeof(chunk_t)) + (PAGESIZE - 1)) / PAGESIZE; 
				 chunk_t *newMemory = morecore(neededPages * PAGESIZE); 
				 if (newMemory != NULL) {
				 	 // memory given, put in list and try again
					 newMemory->next = NULL;
					 newMemory->size = (neededPages * PAGESIZE) / sizeof(chunk_t);
					 // +1 is done because Mem_free assumes given pointer is one
					 // past the chunk_t header
					 Mem_free(newMemory + 1);
					 return (Mem_alloc(nbytes));
					 // the postconditions will be checked in this next call of Mem_alloc
				 }
				 else {
				 	 // memory couldn't be given by system
					 return NULL;
				 }
			 }
		 }
		 // Rover at position of acceptable memory block, give to user
		 p = Rover;
		 q = Rover + 1;
		 assert(p->size >= nunits); // would mean it found too small a block
		 if (p->size != nunits) {
			 // need to carve out memory
			 Rover = p + nunits;
			 Rover->next = p->next;
			 Rover->size = p->size - nunits;
			 p->next = Rover;
			 p->size = nunits;
		 }
	 }

	 else if (SearchPolicy == BEST_FIT) {
		 int bestSize = -1;
		 chunk_t *bestBlock = NULL;
		 // check initial Rover first, so it can be moved past originPoint for loop
		 if (Rover->size >= nunits) {
			 bestSize = Rover->size;
			 bestBlock = Rover;
		 }
		 previous = Rover;
		 Rover = Rover->next;

		 // search whole circle for the best fit
		 while (Rover != originPoint) {
			 if (Rover->size >= nunits) {
				 // check if new match is better fit
				 if (Rover->size < bestSize) {
					 bestSize = Rover->size;
					 bestBlock = Rover;
					 assert(bestSize >= nunits);
			 	 }
			 }
			 previous = Rover;
			 Rover = Rover->next;
		 }

		 if (bestSize == -1) {
			 // no match at all found, make more memory
			 int neededPages = ((nunits*sizeof(chunk_t)) + (PAGESIZE - 1)) / PAGESIZE;
			 chunk_t *newMemory = morecore(neededPages * PAGESIZE); 
			 if (newMemory != NULL) {
			 	 // memory given, put in list and try again
				 newMemory->next = NULL;
				 newMemory->size = (neededPages * PAGESIZE) / sizeof(chunk_t);
				 Mem_free(newMemory + 1);
				 return (Mem_alloc(nbytes));
			 }
			 else {
			 	 // memory couldn't be given by system
				 return NULL;
			 }
		 }
		 else {
			 // bestBlock hold position of smallest matching block, give to user
			 p = bestBlock;
			 q = bestBlock + 1;
		 	 if (p->size != nunits) {
			 	// need to carve out memory
			 	bestBlock = p + nunits;
			 	bestBlock->next = p->next;
			 	bestBlock->size = p->size - nunits;
			 	p->next = bestBlock;
				p->size = nunits;
			 }
			 // also, get Rover back to position of new mem before return
			 while (Rover != p) {
				 previous = Rover;
				 Rover = Rover->next;
			 }
		 }
	 }

	 else {
		 puts("ERROR: Invalid Search Policy");
		 return NULL;
	 }

	 // this would mean only one item (the dummy) is in the list, but
	 // the function should not get this far if that's the case
	 assert (previous != Rover);
	 assert (Rover != &Dummy);

	 // this means match was the initial value of Rover and previous never set,
	 // loop around list once to get previous
	 if (previous == NULL) {
		 do {
			 previous = Rover;
			 Rover = Rover->next;
		 } while (Rover != p);
	 }

	 // now remove block to be returned from free list
	 previous->next = p->next;
	 Rover = p->next;
	 p->next = NULL;

    // here are possible post-conditions, depending on your design
    //
    // assume p is a pointer to memory block that will be given to the user
    // and q is the address given to the user
    assert(p + 1 == q);
    // the minus one in the next two tests accounts for the header
    assert((p->size - 1)*sizeof(chunk_t) >= nbytes);
    assert((p->size - 1)*sizeof(chunk_t) < nbytes + sizeof(chunk_t));
    assert(p->next == NULL);  // saftey first!
    return q;
}

/* prints stats about the current free list
 *
 * -- number of items in the linked list including dummy item
 * -- min, max, and average size of each item (in bytes)
 * -- total memory in list (in bytes)
 * -- number of calls to sbrk and number of pages requested
 *
 * A message is printed if all the memory is in the free list
 */
void Mem_stats(void) {
    // One of the stats you must collect is the total number
    // of pages that have been requested using sbrk.
    // Say, you call this NumPages.  You also must count M,
    // the total number of bytes found in the free list 
    // (including all bytes used for headers).  If it is the case
    // that M == NumPages * PAGESiZE then print

	 int M = 0; // num bytes in free list
	 int numItems = 0, maxSize = 0, minSize = 0;
	 float avgSize = 0;


	 chunk_t *start = Rover;

	 // initialize minSize to first size (or next if Rover is dummy)
	 if (Rover->size != 0) {
		 minSize = Rover->size;
	 }
	 else {
		 minSize = Rover->next->size;
	 }

	 do {
		 numItems++;
		 if (Rover->size > maxSize) {
			 maxSize = Rover->size;
		 }
		 if ((Rover->size < minSize) && (Rover->size != 0)) {
			 minSize = Rover->size;
		 }
		 M += Rover->size;
		 Rover = Rover->next;
	 } while (Rover != start);
	 
	 numItems--; // we don't want to count the dummy
	 M = M * sizeof(chunk_t);
	 maxSize = maxSize * sizeof(chunk_t);
	 minSize = minSize * sizeof(chunk_t);
	 avgSize = (float)M / (float)numItems;

	 printf("Number of items in free list: %d\n", numItems);
	 printf("Min size in list: %d bytes\n", minSize);
	 printf("Max size in list: %d bytes \n", maxSize);
	 printf("Average size in list: %0.3f bytes\n", avgSize);
	 printf("Total bytes in free list: %d bytes\n", M);
	 printf("sbrk calls: %d\n", NumSbrkCalls);
	 printf("Number of pages: %d\n", P);

	 if (M == P * PAGESIZE) {
    	 printf("all memory is in the heap -- no leaks are possible\n");
	 }
	 else {
		 puts("Not all memory has been returned to free list");
	 }
}

/* print table of memory in free list 
 *
 * The print should include the dummy item in the list 
 */
void Mem_print(void) {
    assert(Rover != NULL && Rover->next != NULL);
    chunk_t *p = Rover;
    chunk_t *start = p;
    do {
        // example format.  Modify for your design
        printf("p=%p, size=%d, end=%p, next=%p %s\n", 
                p, p->size, p + p->size, p->next, p->size!=0?"":"<-- dummy");
        p = p->next;
    } while (p != start);
    mem_validate();
}

/* This is an experimental function to attempt to validate the free
 * list when coalescing is used.  It is not clear that these tests
 * will be appropriate for all designs.  If your design utilizes a different
 * approach, that is fine.  You do not need to use this function and you
 * are not required to write your own validate function.
 */
void mem_validate(void) {
    assert(Rover != NULL && Rover->next != NULL);
    assert(Rover->size >= 0);
    int wrapped = FALSE;
    int found_dummy = FALSE;
    int found_rover = FALSE;
    chunk_t *p, *largest, *smallest;

    // for validate begin at Dummy
    p = &Dummy;
    do {
        if (p->size == 0) {
            assert(found_dummy == FALSE);
            found_dummy = TRUE;
        } else {
            assert(p->size > 0);
        }
        if (p == Rover) {
            assert(found_rover == FALSE);
            found_rover = TRUE;
        }
        p = p->next;
    } while (p != &Dummy);
    assert(found_dummy == TRUE);
    assert(found_rover == TRUE);

    if (Coalescing) {
        do {
            if (p >= p->next) {
                // this is not good unless at the one wrap
                if (wrapped == TRUE) {
                    printf("validate: List is out of order, already found wrap\n");
                    printf("first largest %p, smallest %p\n", largest, smallest);
                    printf("second largest %p, smallest %p\n", p, p->next);
                    assert(0);   // stop and use gdb
                } else {
                    wrapped = TRUE;
                    largest = p;
                    smallest = p->next;
                }
            } else {
                assert(p + p->size < p->next);
            }
            p = p->next;
        } while (p != &Dummy);
        assert(wrapped == TRUE);
    }
}

