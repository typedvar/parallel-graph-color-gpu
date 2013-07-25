#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <cmath>

#include <CL/cl.h>

#include "defines.h"
#include "graph.h"
#include "graphLoader.h"
#include "utils.h"
#include "lubyColor.h"
#include "nonAdjacencyColor.h"
#include "nonAdjacencyNode.h"

void usage( const char* pProgramName )
{
    printf( "usage: %s <vis|luby> [<OpenCL code file> <Kernel Name>] <Graph data file>\n", pProgramName );
}

#define DEFAULT_VIS_KERNEL_NAME "kernelColor"
#define DEFAULT_VIS_KERNEL_FILE "..\\kernels\\individualSet.cl"

#define DEFAULT_LUBY_KERNEL_NAME "getISSet"
#define DEFAULT_LUBY_KERNEL_FILE "..\\kernels\\lubyColor.cl"

// *********************************************************************
// Main function
// *********************************************************************
int main(int argc, char **argv)
{
    const char* lAlgorithm = NULL;
    const char* lKernelFile = NULL;
    const char* lKernelName = NULL;
    const char* lGraphData = NULL;
    bool lDoLuby = false;
    
    if( argc < 3 )
    {
        usage( argv[0] );
        return 1;
    }
    else 
    {
        lAlgorithm = argv[1];
        if( lAlgorithm[0] == 'l' || lAlgorithm[0] == 'L' )
        {
            lDoLuby = true;
        }

        if( argc == 3 )
        {
            lKernelFile = lDoLuby ? DEFAULT_LUBY_KERNEL_FILE : DEFAULT_VIS_KERNEL_FILE;
            lKernelName = lDoLuby ? DEFAULT_LUBY_KERNEL_NAME : DEFAULT_VIS_KERNEL_NAME;
            lGraphData = argv[2];
        }
        else if( argc == 5 )
        {
            lKernelFile = argv[2];
            lKernelName = argv[3];
            lGraphData = argv[4];
        }
        else
        {
            usage( argv[0] );
            return 1;
        }
    }
    
    Graph lGraph;
    GraphLoader lGraphLoader;
    
    if( !lGraphLoader.loadInput( lGraphData, lGraph ) )
    {
        printf( "Unable to load graph data from %d\n", lGraphData );
        return 2;
    }

    size_t lNumElems = 0;
    Graph::vertexId_t* h_adj = NULL;
    Graph::byte_t* h_bit_adj = NULL;

    if( !lGraph.computeAdjacencyBitMatrix( h_bit_adj, lNumElems ) )
    {
        printf( "Unable to load adjacency bit matrix\n" );
        return 3;
    }

    // size of memory required to store the matrix
    const size_t adj_size = std::ceil( ( ( double )lNumElems ) / Graph::BYTE_SIZE );

#ifdef _DEBUG
    std::cout << "Adjacency Bit Matrix" << std::endl;
    lGraph.printBitMatrix( std::cout, h_bit_adj, lNumElems );
#endif // _DEBUG

    const unsigned int lNumVertices = lGraph.size();

    cl_device_id device_id;
    cl_command_queue commands;
    cl_context context;

    if( !initOCL( device_id, context, commands ) )
    {
        return EXIT_FAILURE;
    }

    // Create program and kernel
    cl_kernel kernel;
    cl_program program;

    if( !createKernelFromSource( lKernelFile,
                                 device_id, 
                                 context, 
                                 program, 
                                 commands,
                                 kernel, 
                                 lKernelName ) )
    {
        return EXIT_FAILURE;
    }

    if( lDoLuby )
    {
        lubyColor( lGraph,
            commands, 
            context, 
            kernel, 
            program, 
            h_adj, 
            adj_size, 
            lNumVertices );
    }
    else
    {
        NonAdjacencyNode_t* lNonAdjListArray = NULL;
        Graph::vertexId_t* lNonAdjArray = NULL;
        Graph::vertexId_t* lNonAdjOffsetArray = NULL;

        if( !lGraph.getNonAdjacencyListArray( lNonAdjListArray ) )
        {
            printf( "Unable to compute non adjacency matrix\n" );
            return 4;
        }

        size_t lNumNonAdjArrayElems = 0;
        Graph::marshallAdjacencyListArray( lNonAdjListArray, 
                                           lNumVertices, 
                                           lNonAdjArray,
                                           lNumNonAdjArrayElems, 
                                           lNonAdjOffsetArray );
#ifdef _DEBUG
        std::cout << "G' adjacency list" << std::endl;

        size_t lLastIdx = 0;
        for( size_t i = 0; i < lNumVertices; ++i )
        {
            size_t lCurrIdx = lNonAdjOffsetArray[i];
            if( lCurrIdx != 0 )
            {
                PRINT_VERT( lGraph, ( i - 1 ) );
                std::cout << " : ";
                for( size_t j = lLastIdx; j < lCurrIdx; ++j )
                {
                    PRINT_VERT( lGraph, lNonAdjArray[j] );
                    std::cout << " ";
                }
                std::cout << std::endl;
            }
            lLastIdx = lCurrIdx;
        }
#endif // _DEBUG

        nonAdjacencyColor( lGraph,
            commands,
            context, 
            kernel,
            program,
            h_bit_adj,
            adj_size,
            lNonAdjArray,
            lNumNonAdjArrayElems,
            lNonAdjOffsetArray,
            lNumVertices );

        lGraph.releaseMatrix( ( Graph::vertexId_t*& )lNonAdjArray );
    }

    ::clFinish( commands );

    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(commands);
    clReleaseContext(context);

    lGraph.releaseMatrix( ( Graph::vertexId_t*& )h_adj );

#ifdef _DEBUG
    getchar();
#endif

    return 0;
}
