#include "raaVector.h"
#include <raaSystem/raaSystem.h>


void initVector(raaVector* vec, int initial_capacity) {
    vec->size = 0;
    vec->capacity = initial_capacity;
    vec->data = (void**)malloc(vec->capacity * sizeof(void*));  
    if (vec->data == NULL) {
        exit(1);  
    }
}

void resizeVector(raaVector* vec) {
    vec->capacity *= 2;
    vec->data = (void**)realloc(vec->data, vec->capacity * sizeof(void*)); 

    if (vec->data == NULL) {
        exit(1);  
    }
}

void addElementToVector(raaVector* vec, void* value) {
    if (vec->size == vec->capacity) {
        resizeVector(vec);
    }
    vec->data[vec->size] = value;
    vec->size++;
}

void* getElementFromVector(raaVector* vec, int index) {
    if (index >= vec->size) {
        exit(1);  
    }
    return vec->data[index];
}

void freeVector(raaVector* vec, void (*freeElement)(void*)) {
    for (size_t i = 0; i < vec->size; i++) {
        freeElement(vec->data[i]); 
    }

    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;
}

void freeVector(raaVector* vec) {

    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;
}

void linkedListToVector(raaLinkedList* pLinkedList, raaVector* pNodes) {
    raaLinkedListElement* element = pLinkedList->m_pHead;

    if (element) {
        raaNode* pNode = (raaNode*)element->m_pData;
        addElementToVector(pNodes, pNode);

        while (element->m_pNext) {
            element = element->m_pNext;
            raaNode* pNode = (raaNode*)element->m_pData;
            addElementToVector(pNodes, pNode);
        }
    }

}


void bubbleSortVector(raaVector *pNodes, int(*swapCondition)(void*, void*)) {
    bool hasSwapped = true;
    while (hasSwapped) {
        hasSwapped = false;
        for (int i = 0; i < pNodes->size - 1; ++i) {
            void* node0 = (void*)pNodes->data[i];
            void* node1 = (void*)pNodes->data[i+1];

            if (swapCondition(node0, node1)) {
                pNodes->data[i] = node1;
                pNodes->data[i + 1] = node0;
                hasSwapped = true;
            }
        }
    }

}
