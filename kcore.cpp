#include <iostream>
#include <fstream>
#include <algorithm>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

// A struct to represent an edge in the edge list
struct edge {
    int src;
    int tgt;
};

// Memory mapped edge list
edge *edgeList;

// Memory maps input file
void createMemoryMap(char *fileName) {
    int binFile = open(fileName, O_RDWR);
    long fileSizeInByte;
    struct stat sizeResults;
    assert(stat(fileName, &sizeResults) == 0);
    fileSizeInByte = sizeResults.st_size;
    edgeList = (edge *)mmap(NULL, fileSizeInByte, PROT_READ | PROT_WRITE, MAP_SHARED, binFile, 0);
    close(binFile);
}

bool compareEdges(const edge& a, const edge& b) {
        return a.src < b.src;
}

// Compares indices according to their corresponding edges
int compareByEdges(const void * a, const void * b) {
        if ((edgeList + *(int *)a)->src < (edgeList + *(int *)b)->src) return -1;
        if ((edgeList + *(int *)a)->src == (edgeList + *(int *)b)->src) return 0;
        if ((edgeList + *(int *)a)->src > (edgeList + *(int *)b)->src) return 1;
}

// Utility function to print the entire memory mapped graph
void printGraph(int EDGENUM) {
        for(int i = 0; i < EDGENUM; i++) {
                std::cout<<(edgeList + i)->src<<" "<<(edgeList + i)->tgt<<"\n";
        }
}

// Utility function to print a given array
template <class T>
void printArray(T *arr, int n) {
        for(int i = 0; i < n; i++) {
        std::cout<<arr[i]<<" ";
        }
        std::cout<<"\n";
}

// Formats the graph by sorting it and tracing original indices in the graph
void formatGraph(const int EDGENUM, int *originalIndices) {
        int *indices = new int[EDGENUM];
        for(int i = 0; i < EDGENUM; i++) {
                indices[i] = i;
        }
        qsort(indices, EDGENUM, sizeof(int), compareByEdges);
        for(int i = 0; i < EDGENUM; i++) {
                originalIndices[indices[i]] = i;
        }
        std::sort(edgeList, edgeList + EDGENUM, compareEdges);
        delete [] indices;
}

int main(int argc, char *argv[]) {
        const int EDGENUM = atoi(argv[2]);
        const int NODENUM = atoi(argv[3]);
        int *originalIndices = new int[EDGENUM];
        createMemoryMap(argv[1]);
        formatGraph(EDGENUM, originalIndices);
        delete[] originalIndices;
        return 0;
}
