#include <iostream>
#include <vector>
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
#define DEBUG 0

long long currentTimeMilliS = 0;

long long currentTimeStamp() {
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds
    return milliseconds;
}

void reset() {
        currentTimeMilliS = currentTimeStamp();
}

long long getTimeElapsed() {
    long long newTime = currentTimeStamp();
    long long timeElapsed = newTime - currentTimeMilliS;
    currentTimeMilliS = newTime;
    return timeElapsed;
}

void showTimeElapsed(const char * comment) {
    long long newTime = currentTimeStamp();
    long long timeElapsed = newTime - currentTimeMilliS;
    currentTimeMilliS = newTime;
    std::cout << comment << ": " << timeElapsed << " milliseconds.\n";
}

// A struct to represent an edge in the edge list
struct edge {
    int src;
    int tgt;
};

// Memory mapped edge list
edge *edgeList;

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
        if(a.src < b.src)
                return true;
        if(a.src == b.src) {
                if(a.tgt < b.tgt)
                        return true;
        }
        return false;
}

// Compares indices according to their corresponding edges
int compareByEdges(const void * a, const void * b) {
        if ((edgeList + *(int *)a)->src < (edgeList + *(int *)b)->src)
                return -1;
        if ((edgeList + *(int *)a)->src == (edgeList + *(int *)b)->src){
                if ((edgeList + *(int *)a)->tgt < (edgeList + *(int *)b)->tgt)
                        return -1;
                if ((edgeList + *(int *)a)->tgt == (edgeList + *(int *)b)->tgt)
                        return 0;
                if ((edgeList + *(int *)a)->tgt > (edgeList + *(int *)b)->tgt)
                        return 1;
        }
        if ((edgeList + *(int *)a)->src > (edgeList + *(int *)b)->src)
                return 1;
}

// Formats the graph by sorting it and tracing original indices in the graph
void formatGraph(int EDGENUM, int *originalIndices) {
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

// Finds the start and end indices  of each node in the graph
void findStartAndEndIndices(int *start, int *end, int EDGENUM) {
        int i;
        int old = edgeList->src;
        start[old] = 0;
        for(i = 0; i < EDGENUM; i++) {
                if((edgeList + i)->src != old) {
                        end[old] = i - 1;
                        old = (edgeList + i)->src;
                        start[old] = i;
                }
        }
        end[old] = i;
}

// Computes the degree of each node in the graph
void findDegree(int *edgeLabels, int EDGENUM, int NODENUM, int *degree) {
        std::fill_n(degree, NODENUM + 1, 0);
        for(int i = 0; i < EDGENUM; i++) {
                // If edge hasn't been deleted yet. An edge is considered deleted
                // when it has been labeled.
                if(edgeLabels[i] == -1) {
                        degree[(edgeList + i)->src]++;
                        degree[(edgeList + i)->tgt]++;
                }
        }
        for(int i = 0; i < NODENUM + 1; i++) {
                degree[i] /= 2;
        }
}

void core(int *start_indices, int *end_indices, int NODENUM, int EDGENUM, int *edgeLabels, int *deg) {
        int * vert = new int[NODENUM + 1];
        int * pos = new int[NODENUM + 1];
        std::fill_n(vert, NODENUM + 1, 0);
        std::fill_n(pos, NODENUM + 1, 0);
        int md = *std::max_element(deg, deg + NODENUM + 1);
        int * bins = new int[md + 1];
        std::fill_n(bins, md + 1, 0);
        for(int v = 1; v <= NODENUM; v++)
                bins[deg[v]]++;
        int start = 1;
        for(int d = 0; d <= md; d++) {
                int num = bins[d];
                bins[d] = start;
                start += num;
        }
        for(int v = 1; v <= NODENUM; v++) {
                pos[v] = bins[deg[v]];
                vert[pos[v]] = v;
                bins[deg[v]]++;
        }
        for(int d = md; d > 0; d--) {
                bins[d] = bins[d - 1];
        }
        bins[0] = 1;
        for(int i = 1; i <= NODENUM; i++) {
                int v = vert[i];
                // Do nothing if node doesn't exist in the graph
                if(start_indices[v] == 0 && end_indices[v] == 0) {
                        ;
                }
                else {
                        for(int i = start_indices[v]; i <= end_indices[v]; i++) {
                            if(edgeLabels[i] == -1) {
                                int u = (edgeList + i)->tgt;
                                if(deg[u] > deg[v]) {
                                        int du = deg[u];
                                        int pu = pos[u];
                                        int pw = bins[du];
                                        int w = vert[pw];
                                        if(u != w) {
                                                pos[u] = pw;
                                                pos[w] = pu;
                                                vert[pu] = w;
                                                vert[pw] = u;
                                        }
                                        bins[du]++;
                                        deg[u]--;
                                }
                            }
                        }
                }
        }
        delete [] vert;
        delete [] pos;
        delete [] bins;
}

void labelAndDeleteEdges(bool *isFinalNode, int peel, int EDGENUM, int *edgeLabels) {
        for(int i = 0; i < EDGENUM; i++) {
                if(isFinalNode[(edgeList + i)->src] && isFinalNode[(edgeList + i)->tgt] && edgeLabels[i] == -1) {
                        edgeLabels[i] = peel;
                }
        }
}

bool isGraphEmpty(int *edgeLabels, int EDGENUM) {
        for(int i = 0; i < EDGENUM; i++) {
                if(edgeLabels[i] == -1)
                        return false;
        }
        return true;
}

void writeToFile(int *edgeLabels, int EDGENUM, char *fileName) {
        std::ofstream outputFile;
        outputFile.open(fileName);
        for(int i = 0; i < EDGENUM; i++) {
                outputFile<<edgeLabels[i]<<"\n";
        }
        outputFile.close();
}

void doubleAndReverseGraph(char *inputFile, char *outputFile, int EDGENUM) {
        std::ifstream is;
        is.open(inputFile, std::ios::in | std::ios::binary);
        std::ofstream os;
        os.open(outputFile, std::ios::out | std::ios::binary | std::ios::app);
        int src, tgt;
        for(int i = 0; i < EDGENUM; i++) {
                is.read((char *)(&src), sizeof(int));
                is.read((char *)(&tgt), sizeof(int));
                src = htonl(src);
                tgt = htonl(tgt);
                // Removes self loops
                if(src != tgt) {
                        os.write((char *)&src, sizeof(int));
                        os.write((char *)&tgt, sizeof(int));
                }
        }
        is.seekg(0, std::ios::beg);
        for(int i = 0; i < EDGENUM; i++) {
                is.read((char *)(&src), sizeof(int));
                is.read((char *)(&tgt), sizeof(int));
                src = htonl(src);
                tgt = htonl(tgt);
                // Removes self loops
                if(src != tgt) {
                os.write((char *)(&tgt), sizeof(int));
                os.write((char *)(&src), sizeof(int));
                }
        }
        is.close();
        os.close();
}

void labelAndDeletePeelOneEdges(int *degree, int EDGENUM, int *edgeLabels) {
        for(int i = 0; i < EDGENUM; i++) {
                if((edgeLabels[i] == -1) && (degree[(edgeList + i)->src] == 1 || degree[(edgeList + i)->tgt] == 1)) {
                        edgeLabels[i] = 1;
                }
        }
}

int main(int argc, char *argv[]) {
        char *tmpFile = "tmp.bin";
        remove(tmpFile);
        int EDGENUM = atoi(argv[2]);
        int NODENUM = atoi(argv[3]);
        reset();
        doubleAndReverseGraph(argv[1], tmpFile, EDGENUM);
        if(DEBUG)
                std::cout<<"DOUBLED AND REVERSED GRAPH\n";
        EDGENUM *= 2;
        int *originalIndices = new int[EDGENUM];
        int *edgeLabels = new int[EDGENUM];
        std::fill_n(edgeLabels, EDGENUM, -1);
        createMemoryMap(tmpFile);
        if(DEBUG)
                std::cout<<"CREATED MEMORY MAP\n";
        formatGraph(EDGENUM, originalIndices);
        if(DEBUG)
                std::cout<<"FORMATTED GRAPH\n";
        int *start = new int[NODENUM + 1];
        int *end = new int[NODENUM + 1];
        std::fill_n(start, NODENUM + 1, 0);
        std::fill_n(end, NODENUM + 1, 0);
        findStartAndEndIndices(start, end, EDGENUM);
        if(DEBUG)
                std::cout<<"START AND END INDICES COMPUTED\n";
        while(!isGraphEmpty(edgeLabels, EDGENUM)) {
                int *degree = new int[NODENUM + 1];
                findDegree(edgeLabels, EDGENUM, NODENUM, degree);
                //labelAndDeletePeelOneEdges(degree, EDGENUM, edgeLabels);
                //findDegree(edgeLabels, EDGENUM, NODENUM, degree);
                core(start, end, NODENUM, EDGENUM, edgeLabels, degree);
                int mc = *std::max_element(degree, degree + NODENUM + 1);
                if(DEBUG)
                        std::cout<<"CURRENT MAXIMUM CORE : "<<mc<<"\n";
                bool *isFinalNode = new bool[NODENUM];
                std::fill_n(isFinalNode, NODENUM, false);
                for(int i = 0; i <= NODENUM; i++) {
                        if(degree[i] == mc) {
                                isFinalNode[i] = true;
                        }
                }
                labelAndDeleteEdges(isFinalNode, mc, EDGENUM, edgeLabels);
                delete [] degree;
                delete [] isFinalNode;
        }
        EDGENUM /= 2;
        int *originalLabels = new int[EDGENUM];
        if(DEBUG)
                std::cout<<"RECONSTRUCTING ORIGINAL LABELS\n";
        for(int i = 0; i < EDGENUM; i++) {
                originalLabels[i] = edgeLabels[originalIndices[i]];
        }
        showTimeElapsed("Time Elapsed ");
        writeToFile(originalLabels, EDGENUM, argv[4]);
        remove(tmpFile);
        delete [] originalLabels;
        delete [] edgeLabels;
        delete [] start;
        delete [] end;
        delete [] originalIndices;
        return 0;
}
