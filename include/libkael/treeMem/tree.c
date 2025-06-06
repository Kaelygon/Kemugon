
// 
/**
	@file tree.c

	@brief uint c++ std::vector like, dynamic array stored in heap
	
	(void *)data can hold any type of an element, structs, pointers or even *KaelTree
	Memory is allocated dynamically, see growth/shrink factor macros below

	example
	@code
	{ 
	//TODO:
	}
	@endcode
	
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#include "libkael/debug/kaelMacros.h"
#include "libkael/treeMem/tree.h"
#include "libkael/math/math.h"

//if used memory exceeds capacity, scale it by GROWTH_NUMER/GROWTH_DENOM times
//if capacity requires is less than GROWTH_DENOM/GROWTH_NUMER, it's scaled by that amount
#define GROWTH_NUMER 3U 
#define GROWTH_DENOM 2U 

#define ELEMS_MAX (SIZE_MAX/(GROWTH_NUMER/GROWTH_DENOM))
#define ELEMS_MIN 0U

//---alloc and free---

/**
 * @brief Initialize and allocate tree
 */
uint8_t kaelTree_alloc(KaelTree *tree, const size_t size) {
	if(NULL_CHECK(tree)){return KAEL_ERR_NULL;}
	tree->length = 0;
	tree->data 		= NULL;
	tree->capacity 	= 0;
	kaelTree_setWidth(tree, size);
	return KAEL_SUCCESS;
}

/**
 * @brief Free including all the elements
 * 
 * @note Make sure to free allocated elements in tree 
*/
void kaelTree_free(KaelTree *tree){
	if(NULL_CHECK(tree,"free") || NULL_CHECK(tree->data,"free->data")){return;} 
	free(tree->data); //Free branch or leaf
	memset(tree,0,sizeof(KaelTree)); //set to NULL and 0
}


//---rescaling---


/**
 * @brief Set tree to specific length
 */
uint8_t kaelTree_resize(KaelTree *tree, const size_t length){
	if(NULL_CHECK(tree,"resize")){return KAEL_ERR_NULL;}

	if(length > tree->maxLength){ //Beyond last growth stage
		printf("Too many elements\n");
    	return KAEL_ERR_FULL;
	} 

	size_t newLength = kaelMath_min(length, ELEMS_MAX);
	size_t minAlloc = newLength + ELEMS_MIN;
	size_t newAlloc = minAlloc * tree->width;

	//Shrinking nor 0 size doesn't invoke realloc
	if( newAlloc > tree->capacity ){ 
		if(newAlloc > (UINT16_MAX / GROWTH_NUMER) * GROWTH_DENOM){ //prevent overflow
			newAlloc = UINT16_MAX;
		}else{
			newAlloc = newAlloc * GROWTH_NUMER/GROWTH_DENOM;
		}

		void *newData = realloc(tree->data, newAlloc);
		if( NULL_CHECK(newData) ){ return KAEL_ERR_ALLOC; }

		//Zero newly resized portion if any
		if (newAlloc > tree->capacity) {
			size_t newSize = newAlloc - tree->capacity;
			memset((uint8_t*)newData + tree->capacity, 0, newSize);
		}

		tree->capacity=newAlloc;
		tree->data=newData;
	}

	tree->length=newLength;
	return KAEL_SUCCESS;
}

/**
 * @brief Insert at index, pushing remaining elements to right
 */
KaelTree *kaelTree_insert(KaelTree *tree, size_t index, const void *restrict element){
	if(NULL_CHECK(tree)){return NULL;}
	KAEL_ASSERT(index < tree->length, "kaelTree_insert out of bounds");
	uint8_t code = kaelTree_resize(tree, tree->length+1);

	//from index to old end
	size_t copyAmount = (tree->length - index) * tree->width;

	if(code==KAEL_ERR_FULL){return NULL;}

	//shift everything from index to end by 1 element
	void* dest = kaelTree_get(tree, index);
	if(dest==NULL){return NULL;}
	memmove( (uint8_t *)dest + tree->width, dest, copyAmount );

	if(element==NULL){ //No macro since NULL use is valid
    	memset(dest, 0, tree->width);
	}else{
    	memcpy(dest, element, tree->width);
	}
	return dest;
}

/**
 * @brief Add element to tree. NULL element is initialized as zero
 */
KaelTree *kaelTree_push(KaelTree *tree, const void *restrict element){
	if(NULL_CHECK(tree)){return NULL;}
	
	uint8_t code = kaelTree_resize(tree, tree->length+1);
	if(code==KAEL_ERR_FULL){return NULL;}

	//copy the element after last element
   void *dest = kaelTree_back(tree); 
	if(NULL_CHECK(dest)){return NULL;}
	
	if(element==NULL){ //No macro since NULL use is valid
    	memset(dest, 0, tree->width);
	}else{
    	memcpy(dest, element, tree->width);
	}

	return dest;
}

/**
 * @brief Remove last element
 */
uint8_t kaelTree_pop(KaelTree *tree){
	if(NULL_CHECK(tree) || (tree->length==0)){return KAEL_ERR_NULL;}

	size_t newLength = tree->length -1;

	size_t newAlloc = newLength * tree->width + ELEMS_MIN;
	size_t scaleAlloc = (tree->capacity/GROWTH_NUMER)*GROWTH_DENOM;
	if( newAlloc <= scaleAlloc ){ //shrink if below threshold
		newAlloc = tree->capacity/GROWTH_NUMER*GROWTH_DENOM;
		void *newData = realloc(tree->data, newAlloc);
		if( NULL_CHECK(newData,"popRealloc") ){ return KAEL_ERR_ALLOC; }
		tree->capacity=newAlloc;
		tree->data=newData;
	}

	tree->length=newLength;
	return KAEL_SUCCESS;
}

//---setters---

//set element byte width. Any existing data will be invalidated
void kaelTree_setWidth(KaelTree *tree, const size_t size){
	if(NULL_CHECK(tree,"setSize")){return;}
	tree->width=size;
	size_t length = tree->length;
	tree->maxLength = UINT16_MAX / tree->width - ELEMS_MIN;
	kaelTree_resize(tree,length); //resize with new byte width
}

/**
 * @brief Set element value in a tree by index
 */
void kaelTree_set(KaelTree *tree, const size_t index, const void *restrict element){
	if(NULL_CHECK(tree,"set")){return;}
	void *dest = kaelTree_get(tree, index);
	if(dest==NULL){return;}
	memcpy(dest, element, tree->width);
}

//---getters---

/**
 * @brief Return number of elements in tree 
 */
size_t kaelTree_length(const KaelTree *tree){
	if(NULL_CHECK(tree)){return 0;}
	return tree->length;
}

size_t kaelTree_empty(const KaelTree *tree){
	if(NULL_CHECK(tree)){return -1;}
	return (tree->length==0);
}

size_t kaelTree_getIndex(const KaelTree *tree, const void *restrict element){
	if(NULL_CHECK(tree)){return -1;}
	return ((const uint8_t *)element - (const uint8_t *)tree->data) / tree->width;
}

//---get pointers---

//get by index
void *kaelTree_get(const KaelTree *tree, const size_t index){
	KAEL_ASSERT(!NULL_CHECK(tree,"get"));
	KAEL_ASSERT(!NULL_CHECK(tree->data,"get"));
	KAEL_ASSERT(index < tree->length, "kaelTree_get out of bounds");

   void *elem = (uint8_t *)tree->data + index * tree->width;
	return elem;
}
//get first element
void *kaelTree_begin(const KaelTree *tree){
    void *elem = kaelTree_get(tree,0);
	return elem;
}
//get last element
void *kaelTree_back(const KaelTree *tree){
    void *elem = kaelTree_get( tree, tree->length-1 );
	return elem;
}

/* Iterators in C are kinda of a mess without templates
uint8_t kaelTree_next(const KaelTree *tree, void **current){
	KAEL_ASSERT(current != NULL || tree != NULL);
	void *next = (void *)((uint8_t *)*current + tree->width);
	void *back = kaelTree_back(tree);
	if (next > back){
		*current=NULL;
		return 0;
	}
	*current=next;
	return 1;
}
*/