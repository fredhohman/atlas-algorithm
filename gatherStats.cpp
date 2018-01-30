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
#include "properties.h"

char *mmapGraph;

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

void createMemoryMap() {
    int binFile = open(INPUTFILE, O_RDONLY);
    long fileSizeInByte;
    struct stat sizeResults;
    assert(stat(INPUTFILE, &sizeResults) == 0);
    fileSizeInByte = sizeResults.st_size;
    mmapGraph = (char *)mmap(NULL, fileSizeInByte, PROT_READ, MAP_SHARED, binFile, 0);
    if(mmapGraph == MAP_FAILED) {
        std::cout<<"Memory mapping failed";
    }
    close(binFile);
}

// Checks if the boolean array is all false (corresponds to empty array).
bool isEmpty(bool *arr, int n) {
        for(int i = 0; i < n; i++) {
                if(arr[i] == false) {
                        return false;
                }
        }
        return true;
}

bool isEdgeDeleted(int i) {
    if(i != 0)
        return true;
    return false;
}

// Computes the degree of each node in an undirected graph represented by an edge list
void computeDegree(int *edgeLabels, bool *isNodeDeleted, int *degree) {
        int pointer = 0;
        int source, target;
        std::fill_n(degree, NODENUM, 0);
        for(int i = 0; i < EDGENUM; i++) {
                source = htonl(*((int *)(mmapGraph + pointer)));
                pointer += sizeof(int);
                target = htonl(*((int *)(mmapGraph + pointer)));
                pointer += sizeof(int);
                assert(source >= 0 && target >= 0);
                assert(source <= NODENUM && target <= NODENUM);
                // Update degree only if the nodes / edge haven't been deleted
                if(!isNodeDeleted[source] && !isNodeDeleted[target] && !isEdgeDeleted(edgeLabels[i])) {
                        degree[source]++;
                        degree[target]++;
                }
        }
        if(DEBUG) {
                std::cout<<"DEGREES\n";
                for(int i = 0; i < NODENUM; i++) {
                        std::cout<<degree[i]<<" ";
                }
                std::cout<<"\n";
        }
}

// Delete all nodes with degree <= peel
void deleteNodes(int peel, int *degree, bool *isNodeDeleted, int numNodes) {
        for(int i = 0; i < numNodes; i++) {
                if(degree[i] <= peel) {
                        isNodeDeleted[i] = true;
                }
        }
}

// Deletes the largest core
bool labelAndDeleteEdges(int peel, bool *isFinalNode, int *edgeLabels) {
        int pointer = 0;
        int source, target;
        bool isGraphEmpty = true;
        // Label and delete the induced subgraph
        if(DEBUG)
                std::cout<<"EDGES BEING DELETED\n";
        for(int i = 0; i < EDGENUM; i++) {
                source = htonl(*((int *)(mmapGraph + pointer)));
                pointer += sizeof(int);
                target = htonl(*((int *)(mmapGraph + pointer)));
                pointer += sizeof(int);
                if(isFinalNode[source] && isFinalNode[target] && !isEdgeDeleted(edgeLabels[i])) {
                        if(DEBUG)
                                std::cout<<source<<" "<<target<<" "<<peel<<"\n";
                        edgeLabels[i] = peel;
                }
                isGraphEmpty = isGraphEmpty & isEdgeDeleted(edgeLabels[i]);
        }
        return isGraphEmpty;
}

void edgeDecomposition(int *edgeLabels) {
        int peel = 0;
        int *degree = new int[NODENUM];
        // Tracks deleted nodes
        bool *isNodeDeleted = new bool[NODENUM];
        // Used to check if the graph has changed from the previous iteration
        bool *oldNodeDeleted = new bool[NODENUM];
        // Stores nodes in final layer
        bool *isFinalNode = new bool[NODENUM];
        // Set to true if the graph is empty (ie) all edges deleted
        bool isGraphEmpty = false;
        /* Iterate until we run out of edges.
           Note that edges are deleted only when they
           are labeled and removed from the graph.
           Only nodes are deleted in all other cases.
        */
        while(!isGraphEmpty) {
                if(DEBUG)
                        std::cout<<"PEEL : "<<peel<<"\n";
                std::fill_n(isFinalNode, NODENUM, false);
                std::fill_n(oldNodeDeleted, NODENUM, true);
                bool isGraphUnchanged = false;
                while(!isGraphUnchanged) {
                        isGraphUnchanged = true;
                        // Computes degree of each node
                        computeDegree(edgeLabels, isNodeDeleted, degree);
                        std::copy(isNodeDeleted, isNodeDeleted + NODENUM, oldNodeDeleted);
                        // Deletes nodes with degree <= peel
                        deleteNodes(peel, degree, isNodeDeleted, NODENUM);
                        // Finds the nodes in the final layer.
                        if(DEBUG)
                                std::cout<<"FINAL LAYER NODES\n";
                        for(int i = 0; i < NODENUM; i++) {
                                if(oldNodeDeleted[i] != isNodeDeleted[i]) {
                                        isFinalNode[i] = true;
                                        isGraphUnchanged = false;
                                }
                        }
                        if(DEBUG) {
                                for(int i = 0; i < NODENUM; i++) {
                                        std::cout<<isFinalNode[i]<<" ";
                                }
                                std::cout<<"\n";
                        }
                        // Checks if doing this removes all nodes from the graph
                        if(isEmpty(isNodeDeleted, NODENUM)) {
                                // Label these edges and delete them from the graph
                                isGraphEmpty = labelAndDeleteEdges(peel, isFinalNode, edgeLabels);
                                // If graph is empty, finish execution
                                if(isGraphEmpty)
                                        break;
                                peel = 0;
                                std::fill_n(isNodeDeleted, NODENUM, false);
                        }
                }
                peel++;
        }
        delete [] degree;
        delete [] isNodeDeleted;
        delete [] oldNodeDeleted;
        delete [] isFinalNode;
}

void writeToFile(int *edgeLabels) {
        std::ofstream outputFile;
        outputFile.open(OUTPUTFILE);
        for(int i = 0; i < EDGENUM; i++) {
                outputFile<<edgeLabels[i]<<"\n";
        }
        outputFile.close();
}

int main() {
        int *edgeLabels = new int[EDGENUM];
        createMemoryMap();
        reset();
        edgeDecomposition(edgeLabels);
        showTimeElapsed("Time Elapsed");
        writeToFile(edgeLabels);
        delete [] edgeLabels;
        return 0;
}
