/* list.c     
 * Judson Cooper 
 * MP4
 *
 * Purpose: This file provides the functions needed to create and interact
 *		with a two way linked list. 
 *
 * Assumptions: These functions cannot depend on the details of the records
 *		being stored in the list. It cannot access the members of the data
 *		that is being stored. This file implements the interface defined
 *		in list.h.
 *
 * Bugs: None known.
 */

#include <stdlib.h>
#include <assert.h>

#include "datatypes.h"   // defines data_t 
#include "list.h"        // defines public functions for list ADT 

// definitions for private constants used in list.c only 
#define SORTED_LIST   0xCACACAC
#define UNSORTED_LIST 0x3535353

// prototypes for private functions used in list.c only 
void list_debug_validate(list_t *L);
void InsertionSort(ListPtr list_ptr);
void SelectionSortR(list_t *A, list_node_t *m, list_node_t *n);
void SelectionSortI(list_t *A, list_node_t *m, list_node_t *n);
list_node_t* FindMax(list_t *A, list_node_t *m, list_node_t *n);
void MergeSort(list_t *List);
void HalveList(list_t* list_ptr, list_t* rList, list_t* lList);
void CombineLists (list_t* list_ptr, list_t* rList, list_t* lList);

/* ----- below are the functions  ----- */

/* Allocates a new, empty list 
 *
 * By convention, the list is initially assumed to be sorted.  The field sorted
 * can only take values SORTED_LIST or UNSORTED_LIST
 *
 * The inital empty list must have
 *  1. current_list_size = 0
 *  2. list_sorted_state = SORTED_LIST
 *
 * Use list_destruct to remove and deallocate all elements on a list
 * and the header block.
 */
list_t *list_construct(int (*fcomp)(const data_t *, const data_t *))
{
    list_t *L;

    L = (list_t *) malloc(sizeof(list_t));
    L->head = NULL;
    L->tail = NULL;
    L->current_list_size = 0;
    L->list_sorted_state = SORTED_LIST;
	 L->comp_proc = fcomp;

    // the last line of this function must call validate
    // list_debug_validate(L);
    return L;
}

/* Purpose: return the count of number of elements in the list.
 *
 * list_ptr: pointer to list-of-interest.  
 *
 * Return: the number of elements stored in the list.  
 */
int list_size(list_t *list_ptr)
{
    assert(NULL != list_ptr);
    assert(list_ptr->current_list_size >= 0);
    return list_ptr->current_list_size;
}

/* Deallocates the contents of the specified list, releasing associated memory
 * resources for other purposes.
 *
 * Free all elements in the list and the header block.
 */
void list_destruct(list_t *list_ptr)
{
    // the first line must validate the list 
    // list_debug_validate(list_ptr);
	 
	 IteratorPtr currentNode = list_ptr->head;
	 IteratorPtr nextNode = NULL;

	 while (currentNode != NULL) {
		 nextNode = currentNode->next;

		 // get rid of the data
		 free(currentNode->data_ptr);
		 currentNode->data_ptr = NULL;

		 // get rid of the node
		 free(currentNode);
		 currentNode = NULL;

		 // move to next node to free
		 currentNode = nextNode;
	 }

	 // get rid of header
	 free(list_ptr);
	 list_ptr = NULL;
}

/* Return an Iterator that points to the last list_node_t. If the list is empty
 * then the pointer that is returned is NULL.
 */
list_node_t * list_iter_back(list_t *list_ptr)
{
    assert(NULL != list_ptr);
    // list_debug_validate(list_ptr);
    return list_ptr->tail;
}

/* Return an Iterator that points to the first element in the list.  If the
 * list is empty the value that is returned in NULL.
 */
list_node_t * list_iter_front(list_t *list_ptr)
{
    assert(NULL != list_ptr);
    // list_debug_validate(list_ptr);
    return list_ptr->head;
}

/* Advance the Iterator to the next item in the list.  
 * If the iterator points to the last item in the list, then 
 * this function returns NULL to indicate there is no next item.
 * 
 * It is a catastrophic error to call this function if the
 * iterator, idx_ptr, is null.
 */
list_node_t * list_iter_next(list_node_t * idx_ptr)
{
    assert(idx_ptr != NULL);
    return idx_ptr->next;
}

/* Finds an element in a list and returns a pointer to it.
 *
 * list_ptr: pointer to list-of-interest.  
 *
 * elem_ptr: element against which other elements in the list are compared.
 *           Note: uses the comp_proc function pointer found in the list_t
 *           header block. 
 *
 * The function returns an Iterator pointer to the list_node_t that contains 
 * the first matching element if a match if found.  If a match is not found 
 * the return value is NULL.
 *
 * Note: to get a pointer to the matching data_t memory block pass the return
 *       value from this function to the list_access function.
 */
list_node_t * list_elem_find(list_t *list_ptr, data_t *elem_ptr)
{
    // list_debug_validate(list_ptr);

	 IteratorPtr N = list_ptr->head;

	 // look for match until end of list reached
	 while (N != NULL) {
		 if (list_ptr->comp_proc(N->data_ptr, elem_ptr) == 0) {
			 return N;
		 }
		 N = N->next;
	 }

	// no match found
    return NULL;
}

/* Inserts the element into the specified sorted list at the proper position,
 * as defined by the comparison function, comp_proc.
 *
 * list_ptr: pointer to list-of-interest.  
 *
 * elem_ptr: pointer to the element to be inserted into list.
 *
 * If you use list_insert_sorted, the list preserves its sorted nature.
 *
 * If you use list_insert, the list will be considered to be unsorted, even
 * if the element has been inserted in the correct position.
 *
 * If the list is not sorted and you call list_insert_sorted, this subroutine
 * must generate a system error and the program should immediately stop.
 *
 * The comparison procedure must accept two arguments (A and B) which are both
 * pointers to elements of type data_t.  The comparison procedure returns an
 * integer code which indicates the precedence relationship between the two
 * elements.  The integer code takes on the following values:
 *    1: A should be closer to the front of the list than B
 *   -1: B should be closer to the front of the list than A
 *    0: A and B are equal in rank
 *
 * Note: if the element to be inserted is equal in rank to an element already
 * in the list, the newly inserted element will be placed after all the
 * elements of equal rank that are already in the list.
 */
void list_insert_sorted(list_t *list_ptr, data_t *elem_ptr)
{
    assert(NULL != list_ptr);
    assert(SORTED_LIST == list_ptr->list_sorted_state);

	 // create a new node for the data
	 IteratorPtr newNode = (IteratorPtr) malloc(sizeof(list_node_t));
	 newNode->data_ptr = elem_ptr;
	 newNode->next = NULL;
	 newNode->prev = NULL;

	 // if list empty, need to assign this as head and tail.
	 if (list_ptr->current_list_size == 0) {
		 list_ptr->head = newNode;
		 list_ptr->tail = newNode;
		 list_ptr->current_list_size++;
		 
		 // the last line of this function must be the following 
		 // list_debug_validate(list_ptr);
		 return;
	 }

	 IteratorPtr N = list_ptr->head;
	 int result = 0; // for value of comp_proc
	 int inserted = 0; // 0 for not inserted, 1 for inserted

	 while (N != NULL) {
		 result = list_ptr->comp_proc(elem_ptr, N->data_ptr);
		 if (result == 1) { // put it before N
			 if (N == list_ptr->head) {
				 newNode->next = N;
				 N->prev = newNode;
				 list_ptr->head = newNode;
				 N = NULL; // make loop end
			 }
			 else {
				 newNode->next = N;
				 newNode->prev = N->prev;
				 N->prev->next = newNode;
				 N->prev = newNode;
				 N = NULL; // make loop end
			 }
			 inserted = 1;
		 }
		 if (result == -1) { // keep looking through list
			 N = N->next;
		 }
		 if (result == 0) {
			 N = N->next; // so it will go after entries of equal rank
		 }
	 }

	 if (inserted == 0) { // was not inserted, so put at back
		 list_ptr->tail->next = newNode;
		 newNode->prev = list_ptr->tail;
		 list_ptr->tail = newNode;
	 }

	 list_ptr->current_list_size++;


    // the last line of this function must be the following 
    // list_debug_validate(list_ptr);
}

/* Inserts the data element into the list in front of the iterator 
 * position.
 *
 * list_ptr: pointer to list-of-interest.  
 *
 * elem_ptr: pointer to the memory block to be inserted into list.
 *
 * idx_ptr: pointer to a list_node_t.  The element is to be inserted as a 
 *          member in the list at the position that is immediately in front 
 *          of the position of the provided Iterator pointer.
 *
 *          If the idx_ptr pointer is NULL, the the new memory block is
 *          inserted after the last element in the list.  That is, a null
 *          iterator pointer means make the element the new tail.
 *
 * If idx_ptr is set using 
 *    -- list_iter_front, then the new element becomes the first item in 
 *       the list.
 *    -- list_iter_back, then the new element is inserted before the last item 
 *       in the list.
 *    -- for any other idx_ptr, the new element is insert before the 
 *       Iterator
 *
 * For example, to insert at the tail of the list do
 *      list_insert(mylist, myelem, NULL)
 * to insert at the front of the list do
 *      list_insert(mylist, myelem, list_iter_front(mylist))
 *
 * Note that use of this function results in the list to be marked as unsorted,
 * even if the element has been inserted in the correct position.  That is, on
 * completion of this subroutine the list_ptr->list_sorted_state must be equal 
 * to UNSORTED_LIST.
 */
void list_insert(list_t *list_ptr, data_t *elem_ptr, list_node_t * idx_ptr)
{
    assert(NULL != list_ptr);

	// make new node for the data
	 IteratorPtr newNode = (IteratorPtr) malloc(sizeof(list_node_t));
	 newNode->next = NULL;
	 newNode->prev = NULL;
	 newNode->data_ptr = elem_ptr;

	// if list empty, new node both head and tail
	 if (list_ptr->current_list_size == 0) {
		 list_ptr->head = newNode;
		 list_ptr->tail = newNode;
	 }
	 // want it in back of list
	 else if (idx_ptr == NULL) { 
		 list_ptr->tail->next = newNode;
		 newNode->prev = list_ptr->tail;
		 list_ptr->tail = newNode; 
	 }
		 // want it before idx_ptr
	 else { 
		 newNode->prev = idx_ptr->prev;
		 newNode->next = idx_ptr;
		 idx_ptr->prev = newNode;
		 if (idx_ptr == list_ptr->head) {
			 list_ptr->head = newNode;
		 }
		 // this may cause issues if idx_ptr is head, so it is put here
		 else {
			 newNode->prev->next = newNode;
		 }
	 }
	 list_ptr->current_list_size++;

    // the last two lines of this function must be the following 
    if (list_ptr->list_sorted_state == SORTED_LIST) 
	list_ptr->list_sorted_state = UNSORTED_LIST;
    // list_debug_validate(list_ptr);
}

/* Sorts the given list using the specified algorithm.
 * 
 * list_ptr: pointer to list of interest.
 * 
 * sort_type: integer that specifies which sort algorithm to use.
 *					1- insertion sort
 *					2- recursive selection sort
 *					3- iterative selection sort
 *					4- merge sort
 */
void list_sort(ListPtr list_ptr, int sort_type) {
		if (sort_type == 1) { // insertion sort
			InsertionSort(list_ptr);
		}
		else if (sort_type == 2) { // recursive selection sort
			SelectionSortR(list_ptr, list_ptr->head, list_ptr->tail);
		}
		else if (sort_type == 3) { // iterative selection sort
			SelectionSortI(list_ptr, list_ptr->head, list_ptr->tail);
		}
		else if (sort_type == 4) { // merge sort
			MergeSort(list_ptr);
		}
		else { // invalid sort type
			return;
		}
		list_ptr->list_sorted_state = SORTED_LIST;
		list_debug_validate(list_ptr);
}

/* Removes the element from the specified list that is found at the 
 * iterator pointer.  A pointer to the data element is returned.
 *
 * list_ptr: pointer to list-of-interest.  
 *
 * idx_ptr: pointer to position of the element to be accessed.  This pointer
 *          must be obtained from list_elem_find, list_iter_front, or
 *          list_iter_next, or list_iter_back.  
 *
 *          If the idx_ptr is null, then assume that the first item
 *          at the head is to be removed.
 *
 * If the list is empty, then the function should return NULL.  Note: if
 *    the list is empty, then the iterator should be a NULL pointer.
 *
 * Note to remove the element at the front of the list use either
 *     list_remove(mylist, list_iter_front(mylist))
 *  or
 *     list_remove(mylist, NULL);
 *
 * Note: a significant danger with this function is that once
 * an element is removed from the list, the idx_ptr is dangling.  That
 * is, the idx_ptr now points to memory that is no longer valid.
 * You should not call list_iter_next on an iterator after there
 * has been a call to list_remove with the same iterator.
 *
 * When you remove the list_node_t in this function, you should null the next
 * and prev pointers before you free the memory block to avoid accidental use
 * of these now invalid pointers. 
 */
data_t * list_remove(list_t *list_ptr, list_node_t * idx_ptr)
{
    assert(NULL != list_ptr);
    if (0 == list_ptr->current_list_size) {
		 assert(idx_ptr == NULL);
		 return NULL; // list empty
	 }

	 
	 data_t *removedData = NULL; // to save ptr to the data for return
	 // if only 1 entry, handle differently
	 if (1 == list_ptr->current_list_size) {
		 removedData = list_ptr->head->data_ptr;
		 free (list_ptr->head);
		 list_ptr->head = NULL;
		 list_ptr->tail = NULL;
	 }
	 else if ((idx_ptr == NULL) || (idx_ptr == list_ptr->head)) { // 
		 IteratorPtr oldFront = list_ptr->head;
		 removedData = oldFront->data_ptr;
		 list_ptr->head = oldFront->next; // if front removed, select new head
		 list_ptr->head->prev = NULL;

		 oldFront->data_ptr = NULL;
		 oldFront->next = NULL;
		 oldFront->prev = NULL;
		 free(oldFront);
		 oldFront = NULL;
	 }
	 else if (idx_ptr == list_ptr->tail) {
		 IteratorPtr oldBack = list_ptr->tail;
		 removedData = oldBack->data_ptr;
		 list_ptr->tail = oldBack->prev; // if back removed, select new tail
		 list_ptr->tail->next = NULL;

		 oldBack->data_ptr = NULL;
		 oldBack->next = NULL;
		 oldBack->prev = NULL;
		 free(oldBack);
		 oldBack = NULL;
	 }
	 else { // removing entry in middle
		 removedData = idx_ptr->data_ptr;

		 // skip over removed entry
		 idx_ptr->prev->next = idx_ptr->next;
		 idx_ptr->next->prev = idx_ptr->prev;

		 idx_ptr->prev = NULL;
		 idx_ptr->next = NULL;
		 idx_ptr->data_ptr = NULL;
		 free(idx_ptr);
		 idx_ptr = NULL;
	 }
	 list_ptr->current_list_size--;

    // the last line should verify the list is valid after the remove 
    // list_debug_validate(list_ptr);
    return removedData; 
}

/* Return a pointer to an element stored in the list, at the Iterator position
 * 
 * list_ptr: pointer to list-of-interest.  A pointer to an empty list is
 *           obtained from list_construct.
 *
 * idx_ptr: pointer to position of the element to be accessed.  This pointer
 *          must be obtained from list_elem_find, list_iter_front, 
 *          list_iter_back, or list_iter_next.  
 *
 * return value: pointer to the data_t element found in the list at the 
 * iterator position. A value NULL is returned if 
 *     a:  the idx_ptr is NULL.
 *     b:  the list is empty 
 */
data_t * list_access(list_t *list_ptr, list_node_t * idx_ptr)
{
    assert(NULL != list_ptr);
    if (idx_ptr == NULL)
	return NULL;
    // debugging function to verify that the structure of the list is valid 
    // list_debug_validate(list_ptr);

	// check if empty
	 if (list_ptr->current_list_size == 0) {
		 return NULL;
	 }

    return idx_ptr->data_ptr;
}

/*** Private Functions ***/

/* This function verifies that the pointers for the two-way linked list are
 * valid. It checks if the list size equals the number of items in the list.
 *
 * YOU MUST NOT CHANGE THIS FUNCTION.  WE USE IT DURING GRADING TO VERIFY THAT
 * YOUR LIST IS CONSISTENT.
 *
 * No output is produced if the two-way linked list is correct.  
 * The program terminates and prints a line beginning with "Assertion
 * failed:" if an error is detected.
 *
 * The checks are not exhaustive. An error may still exist in the list even 
 * if these checks pass.
 *
 * If the linked list is sorted it also checks that the elements in the list
 * appear in the proper order.
 *
 */
void list_debug_validate(list_t *L)
{
    assert(NULL != L); 
    assert(SORTED_LIST == L->list_sorted_state || UNSORTED_LIST == L->list_sorted_state);
    if (0 == L->current_list_size) assert(NULL == L->head && L->tail == NULL);
    if (NULL == L->tail) 
	assert(NULL == L->head && 0 == L->current_list_size);
    else
	assert(NULL == L->tail->next);
    if (NULL == L->head) 
	assert(NULL == L->tail && 0 == L->current_list_size);
    else
	assert(NULL == L->head->prev);
    if (NULL != L->tail && L->tail == L->head) assert(1 == L->current_list_size);
    if (1 == L->current_list_size) {
        assert(L->head == L->tail && NULL != L->tail);
        assert(NULL != L->tail->data_ptr);
        assert(NULL == L->tail->next && NULL == L->head->prev);
    }
    if (1 < L->current_list_size) {
        assert(L->head != L->tail && NULL != L->tail && NULL != L->head);
        list_node_t *R = L->head;
        int tally = 0;
        while (NULL != R) {
            if (NULL != R->next) assert(R->next->prev == R);
            else assert(R == L->tail);
            assert(NULL != R->data_ptr);
            ++tally;
            R = R->next;
        }
        assert(tally == L->current_list_size);
    }
    if (NULL != L->head && SORTED_LIST == L->list_sorted_state) {
        list_node_t *R = L->head;
        while (NULL != R->next) {
            assert(-1 != L->comp_proc(R->data_ptr, R->next->data_ptr));
            R = R->next;
        }
    }
}

/* Implements an insertion sort to sort a given list.
 * 
 * list_ptr: the list to be sorted
 */
void InsertionSort(ListPtr list_ptr) {
	ListPtr newList = list_construct(list_ptr->comp_proc);
	while (list_ptr->head != NULL) { // continue until list empty
		// put head from given list into new list
		list_insert_sorted(newList, list_remove(list_ptr, list_ptr->head));
	}
	// can't change list_ptr directly because function only passes copy
	list_ptr->head = newList->head;
	list_ptr->tail = newList->tail;
	list_ptr->current_list_size = newList->current_list_size;
	free(newList);
	newList = NULL;
}

/* Implements a recursive selection sort to sort a given list.
 *
 * A: the list to be sorted. 
 * m: node in the lowest position of A to be sorted.
 * n: node in the highest position of A to be sorted.
 *
 * This function assumes m and n are actually in the same list. If they
 * are not, the function will not work properly.
 */
void SelectionSortR(list_t *A, list_node_t *m, list_node_t *n) {
	list_node_t *maxPosition;
	data_t *temp;

	if (m != n) { // if there's more than one node to sort
		maxPosition = FindMax(A, m, n);
		temp = m->data_ptr;
		m->data_ptr = maxPosition->data_ptr;
		maxPosition->data_ptr = temp;
		SelectionSortR(A, m->next, n);
	}
}

/* Finds the maximum node (which should be at the highest position) in the given list.
 *
 * A: the list to be searched.
 * m: the lowest position to search in A.
 * n: the highest position to search in A.
 */
list_node_t* FindMax(list_t *A, list_node_t *m, list_node_t *n) {
	list_node_t *i = m;
	list_node_t *j = m;

	do {
		i = i->next;
		if (A->comp_proc(i->data_ptr, j->data_ptr) == 1) {
			j = i;
		}
	} while (i != n);

	return j;
}

/* Implements an iterative selection sort to sort a given list.
 *
 * A: the list to be sorted. 
 * m: node in the lowest position of A to be sorted.
 * n: node in the highest position of A to be sorted.
 *
 * This function assumes m and n are actually in the same list. If they
 * are not, the function will not work properly.
 */
void SelectionSortI(list_t *A, list_node_t *m, list_node_t *n) {
	list_node_t *maxPosition, *i;
	data_t *temp;

	while (m != n) { // while more than one node to sort
		i = m;
		maxPosition = m;
		do {
			i = i->next;
			if (A->comp_proc(i->data_ptr, maxPosition->data_ptr) == 1) {
				maxPosition = i;
			}
		} while (i != n);
		temp = m->data_ptr;
		m->data_ptr = maxPosition->data_ptr;
		maxPosition->data_ptr = temp;
		m = m->next;
	}
}

/* Sorts a given using using the merge sort algorithm
 *
 * list: the list to be sorted
 */
void MergeSort(list_t *list) {
	if (list->head->next != NULL) { // >1 item in list
		list_t* rList = list_construct(list->comp_proc);
		list_t* lList = list_construct(list->comp_proc);

		HalveList(list, rList, lList);
		MergeSort(lList);
		MergeSort(rList);
		CombineLists(list, rList, lList);

		free(rList);
		free(lList);
		rList = NULL;
		lList = NULL;
	}
}

/* Takes a given list and cuts it into two separate lists. A support function
 * for the merge sort.
 *
 * list_ptr: the list to be halved.
 * rList: ptr to what will be the right list after the split.
 * lList: pointer to what will be the left list after the split.
 */
void HalveList(list_t* list_ptr, list_t* rList, list_t* lList) {
	int halfSize = list_ptr->current_list_size / 2;
	int i = 0;

	for (i = 0; i < halfSize; i++) {
		list_insert(rList, list_remove(list_ptr, list_ptr->tail), rList->head);
	}
	while (list_ptr->head != NULL) {
		list_insert(lList, list_remove(list_ptr, list_ptr->tail), lList->head);
	}
}

/* Takes two given lists and combines them into one. A support function
 * for the merge sort.
 *
 * list_ptr: pointer to what will hold the combined list.
 * rList: the right list.
 * lList: the left list.
 */
void CombineLists (list_t* list_ptr, list_t* rList, list_t* lList) {
	// loop continues until merging finished
	while (1) {
		// both lists non empty
		if ((lList->head != NULL) && (rList->head != NULL)) {
			if (lList->comp_proc(lList->head->data_ptr, rList->head->data_ptr) == 1) {
				list_insert(list_ptr, list_remove(lList, lList->head), NULL);
			}
			else {
				list_insert(list_ptr, list_remove(rList, rList->head), NULL);
			}
		}
		// both lists empty
		else if ((lList->head == NULL) && (rList->head == NULL)) {
			return; // merging complete, exit loop and return
		}
		// one list empty
		else {
			if (lList->head == NULL) {
				list_insert(list_ptr, list_remove(rList, rList->head), NULL);
			}
			else {
				list_insert(list_ptr, list_remove(lList, lList->head), NULL);
			}
		}
	}
}


