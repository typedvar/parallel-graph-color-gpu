// graphColorSeq.cpp : main project file.

#include <stdio.h>
#include <stdlib.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>

#include "graph.h"
#include "graphLoader.h"


#define ROWS 5
#define FILENAME "C:\\shilpi\\openCL\\projects\\parallelGraphColor\\data\\tree36.txt"

void assignColor(Graph& lGraph, const unsigned int num_vertices)
{
    printf("the number of vertices are %d\n", num_vertices);

    Graph::vertexId_t* lMatrix;
    size_t lNumElems;

    if( lGraph.getAdjacencyMatrix( lMatrix, lNumElems ) )
    {
        unsigned int maxDegree = 0;
        size_t degree = 0;

        for (unsigned int i = 0; i < num_vertices; i++)
        {
            if( lGraph.getDegree( i, degree ) )
            {
                if ( degree > maxDegree )
                    maxDegree = degree;
            }
        }

        printf("Max Degree is %d\n", maxDegree);
        unsigned int* vertexColor = (unsigned int *) malloc (num_vertices * sizeof(unsigned int));



        unsigned int* tempColorSlot = (unsigned int *) malloc (maxDegree * sizeof(unsigned int));

        for (unsigned int i = 1; i < num_vertices; i++)
        {
            vertexColor[i] = -1;
        }

        vertexColor[0] = 0;

        for (unsigned int i = 1; i < num_vertices; i++)
        {   
            for (unsigned int j = 0; j < maxDegree; j++)
            {
                tempColorSlot[j] = 0;
            }

            printf("Working for vertex %d\n", i);
            for (unsigned int j = 0; j < num_vertices; j++)
            {
                if ((lMatrix [ i * num_vertices + j] == 1) && (vertexColor[j] != -1))
                {
                    printf("vertexColor[%d] is %d\n", j, vertexColor[j]);                 
                    tempColorSlot[vertexColor[j]] = 1;
                    //printf("%d color match with %d\n", i, j);
                }
            }

            //The first unused color slot is used to assign color to i
            for (unsigned int j = 0; j < maxDegree; j++)
            {
                if (tempColorSlot[j] == 0)
                {
                    vertexColor[i] = j;
                    break;
                }
            }
        }

        for (unsigned int j = 0; j < num_vertices; j++)
        {
            std::string lVertexName;
            lGraph.getName( j, lVertexName );
            printf( "colour of vertex %s is %d\n", 
                    lVertexName.c_str(), 
                    vertexColor[j]);
        }
    }

    getchar();
}

int main(int argc, char **argv)
{
    const char* lGraphData = FILENAME;
    unsigned int* h_adj = NULL;

    Graph lGraph;
    GraphLoader lGraphLoader;

    if( !lGraphLoader.loadInput( lGraphData, lGraph ) )
    {
        printf( "Unable to load graph data from %d\n", lGraphData );
        return 2;
    }

    size_t lNumElems = 0;
    if( !lGraph.getAdjacencyMatrix( ( Graph::vertexId_t*& )h_adj, lNumElems ) )
    {
        printf( "Unable to load adjacency matrix\n" );
        return 3;
    }

#ifdef _DEBUG
    std::cout << "Adjacency Matrix" << std::endl;
    lGraph.printMatrix( std::cout, ( const Graph::vertexId_t* )h_adj, lNumElems );
#endif

    const unsigned int num_vertices = lGraph.size();

    /*	a = (unsigned int *) malloc (mem_size);

        for (int i = 0; i < ROWS; i++)
        {
        for (int j = 0; j < ROWS; j++)
        {
        a[i * ROWS + j] = 0;
        }
        }

        a[1] = 1;
        a[4] = 1;
        a[5] = 1;
        a[7] = 1;
        a[8] = 1;
        a[9] = 1;
        a[11] = 1;
        a[13] = 1;
        a[16] = 1;
        a[17] = 1;
        a[19] = 1;
        a[20] = 1;
        a[21] = 1;
        a[23] = 1;
        */

    assignColor(lGraph, num_vertices);

    getchar();
    return 0;
}

