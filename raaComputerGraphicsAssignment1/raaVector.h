#include <stddef.h>  // For size_t
#include <stdlib.h>  // For malloc, realloc, free
#include <raaSystem/raaSystem.h>

#ifndef RAA_VECTOR_H  // Check if this symbol is already defined
#define RAA_VECTOR_H 

// Define the vector structure
typedef struct _raaVector{
    void** data;      // Array of void pointers
    size_t size;      // Current number of elements in the vector
    size_t capacity;  // Maximum capacity before resizing
} raaVector;

void initVector(raaVector* vec, int initial_capacity);
void resizeVector(raaVector* vec);
void addElementToVector(raaVector* vec, void* value);
void* getElementFromVector(raaVector* vec, int index);
void freeVector(raaVector* vec, void (*freeElement)(void*));
void freeVector(raaVector* vec);
void bubbleSortVector(raaVector* pNodes, int(*swapCondition)(void*, void*));
void linkedListToVector(raaLinkedList* pLinkedList, raaVector* pNodes);

#endif // RAA_VECTOR_H