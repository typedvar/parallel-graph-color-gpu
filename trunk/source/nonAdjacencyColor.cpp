#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <list>
#include <CL/cl.h>
#include "utils.h"
#include "defines.h"
#include "graph.h"

bool nonAdjacencyColor( const Graph& rGraph,
                        cl_command_queue pCommandQueue,
                        cl_context& rContext, 
                        cl_kernel& rKernel,
                        cl_program& rProgram,
                        Graph::byte_t* pAdjBitMatrix,
                        size_t pAdjSize,
                        Graph::vertexId_t* pNonAdjArray,
                        size_t pNonAdjNumElems,
                        Graph::vertexId_t* pNonAdjOffsetArray,
                        size_t pNumVertices )
{
    int allDone = pNumVertices;
    bool lRet = true;

    size_t* work_item_counts = 0;
    int* entry_counts = 0;

    int color_size = sizeof( int ) * pNumVertices;
    int* color = ( int* ) malloc( color_size );
    std::fill( color, color + pNumVertices, -1 );

    Graph::byte_t *h_groups;
    size_t num_group_elems = 0;
    rGraph.createAdjacencyBitMatrix( h_groups, num_group_elems, pNumVertices );
    size_t groups_size = ( size_t )std::ceil( ( ( double )num_group_elems ) / Graph::BYTE_SIZE );

    unsigned int h_serial_coloring = 0;

    printf("Created all the host array\n");

    int num = pNumVertices;

    // Create the input buffer on the device for adjacency matrix
    cl_mem d_adj = clCreateBuffer( rContext, 
                                   CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
                                   ( pAdjSize * sizeof( Graph::byte_t ) ), 
                                   pAdjBitMatrix, 
                                   NULL );
    if (!d_adj)
    {
        printf("Error: Failed to allocate input databuffer on device!\n");
        lRet = false;
    }

    // Create the input buffer on the device for adjacency matrix
    cl_mem d_non_adj = clCreateBuffer( rContext, 
                                       CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                                       ( pNonAdjNumElems * sizeof( Graph::vertexId_t ) ), 
                                       pNonAdjArray, 
                                       NULL );
    if (!d_non_adj)
    {
        printf("Error: Failed to allocate input databuffer on device!\n");
        lRet = false;
    }

    // Create the input buffer on the device for adjacency matrix
    cl_mem d_non_adj_offset_array = clCreateBuffer( rContext, 
                                                    CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                                                    ( pNumVertices * sizeof( Graph::vertexId_t ) ), 
                                                    pNonAdjOffsetArray, 
                                                    NULL );
    if (!d_non_adj)
    {
        printf("Error: Failed to allocate input databuffer on device!\n");
        lRet = false;
    }

    cl_mem d_groups = clCreateBuffer( rContext, 
                                      CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
                                      groups_size, 
                                      h_groups, 
                                      NULL );
    if ( !d_groups )
    {
        printf("Error: Failed to allocate input databuffer on device!\n");
        lRet = false;
    }

    //cl_mem d_colors = clCreateBuffer( rContext, 
    //                                  CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
    //                                  color_size, 
    //                                  color, 
    //                                  NULL );
    //if ( !d_colors )
    //{
    //    printf("Error: Failed to allocate input databuffer on device!\n");
    //    lRet = false;
    //}

    unsigned int div_factor = pNumVertices/500;

    for (unsigned int i = 0; i < div_factor; i++)
    {
    if( lRet )
    {
        START_PROFILING;
        cl_int err;

        //Set the rKernel argument
        err  = clSetKernelArg(rKernel, 0, sizeof(cl_mem), (void *) &d_adj );
        err |= clSetKernelArg(rKernel, 1, sizeof(cl_mem), (void *) &d_non_adj );
        err |= clSetKernelArg(rKernel, 2, sizeof(cl_mem), (void *) &d_non_adj_offset_array );
        err |= clSetKernelArg(rKernel, 3, sizeof(unsigned int), &pNonAdjNumElems );
        err |= clSetKernelArg(rKernel, 4, sizeof(unsigned int), &pNumVertices );
        err |= clSetKernelArg(rKernel, 5, sizeof(cl_mem), (void *) &d_groups );
        //err |= clSetKernelArg(rKernel, 6, sizeof(cl_mem), (void *) &d_colors );

        if( CL_SUCCESS == err )
        {
#ifdef _DEBUG
            printf("Populated Kernel arguments\n");
#endif
            size_t WorkSize[] = { div_factor }; // one dimensional Range

            cl_event lEventNDRangeKernel;

            printf( "Invoking GPU...\n" );

            // Preping GPU
            err = clFinish( pCommandQueue );
            if( err != CL_SUCCESS )
            {
                printf("Error: Failed to finish!\n");
                getchar();
                lRet = false;
                return lRet;
            }


            err = clEnqueueNDRangeKernel( pCommandQueue, 
                                          rKernel, 
                                          1, 
                                          0, 
                                          WorkSize, 
                                          NULL, 
                                          0, 
                                          NULL, 
                                          &lEventNDRangeKernel );

            if (err != CL_SUCCESS)
            {
                printf("Error: Failed to execute rKernel!\n");
                getchar();
                lRet = false;
            }

            err = clFinish( pCommandQueue );
            if( err != CL_SUCCESS )
            {
                printf("Error: Failed to finish!\n");
                getchar();
                lRet = false;
            }

            PROFILE_EVENT( lEventNDRangeKernel, "NDRangeKernel" );


            if( lRet )
            {
                cl_event lEventReadBufferGroups;
                err = clEnqueueReadBuffer( pCommandQueue, 
                                           d_groups, 
                                           CL_TRUE, 
                                           0, 
                                           groups_size, 
                                           h_groups, 
                                           0, 
                                           NULL, 
                                           &lEventReadBufferGroups );
                if( err )
                {
                    printf("Error: Failed to read back groups from the device!\n");
                    lRet = false;
                }

                err = ::clWaitForEvents( 1, &lEventReadBufferGroups );
                err = clFinish( pCommandQueue );
                if( err != CL_SUCCESS )
                {
                    printf("Error: Failed to finish!\n");
                    getchar();
                    lRet = false;
                    return lRet;
                }

                PROFILE_EVENT( lEventReadBufferGroups, "GPU2HOSTRead Groups" );
                
                 //Printing the VIS of the vertices along with the color of the vertices
                for (i = 0; i < div_factor; i++)
                {
                    unsigned int offset = i * pNumVertices;
                    printf( "VIS " );
                    PRINT_VERT(rGraph, i );
                    printf( ": " );

                    for( size_t j = 0; j < pNumVertices; ++j )
                    {
                        if( Graph::getBit( h_groups, offset + j ) )
                        {
                            PRINT_VERT(rGraph, j );
                            
                        }
                    }
                    printf( "\n" );
                }//end for
            }//for loop for dividing data
                size_t i = 0;
                std::list<int> listColor;
                std::list <int>::iterator Iter;

                
               for(i = 0; i < pNumVertices; i++)
                   listColor.push_back(i);
          
                   
                //Variables needed for colouring the vertices
                unsigned int assignColor = 0; // holds the next color to be assigned
                unsigned int isAssignColor = 0; //true if the vertexColor[i] is assigned color in the loop
                unsigned int duplicateNode = 0; //set to 1 if the given VIS is impure

                //NOTE: If we can get the size of each VIS, then the internel for loops can be reduced

                
                     
                unsigned int loopCount = 0;// A variable to see the loop performance
                
                Iter = listColor.begin();

                bool isRemoved = false;
             

                while(!listColor.empty()) // looping for all the VIS
                {

                    unsigned int offset = listColor.front() * pNumVertices;

                    isAssignColor = 0;
                    duplicateNode = 0;

                    for( size_t j = 0; j < pNumVertices; ++j )
                    {
                        if( Graph::getBit( h_groups, offset + j) )
                        {
                            if(color[j] != -1)
                            {
                                duplicateNode = 1;
                                break;
                            }
                        }
                    }

                    if (duplicateNode == 0)
                    {
                        for( size_t j = 0; j < pNumVertices; ++j )
                        {
                            if(Graph::getBit( h_groups, offset + j) )
                            {
                                color[j] = assignColor;
                                isAssignColor = 1;
                                listColor.remove(j);
                            }
                        }
                        ++assignColor;
                    }

                    if( !isAssignColor )
                    {
                        listColor.pop_front();
                    }
                }//end while

                //Coloring the vertices that have a conflict colouring state as their VIS are impure
                unsigned int maxDegree = 0;
                size_t degree = 0;

                for (unsigned int i = 0; i < pNumVertices; i++)
                {
                    if( rGraph.getDegree( i, degree ) )
                    {
                        if ( degree > maxDegree )
                            maxDegree = degree;
                    }
                }

#ifdef _DEBUG
                printf("Max Degree is %d\n", maxDegree);
               for ( Iter = listColor.begin( ); Iter != listColor.end( ); Iter++ )
               {
                  PRINT_VERT( *Iter );
                  printf("\n");
               }
               
#endif
               maxDegree = maxDegree + 1;

                unsigned int* tempColorSlot = (unsigned int *) malloc (maxDegree * sizeof(unsigned int));

                for (unsigned int i = 0; i < pNumVertices; i++)
                {   
                    if (color[i] == -1)
                    {
                        for (unsigned int j = 0; j < maxDegree; j++)
                        {
                            tempColorSlot[j] = 0;
                        }

                        printf("Working for vertex ");
                        PRINT_VERT( rGraph,i);
                        printf("\n");

                        for (unsigned int j = 0; j < pNumVertices; j++)
                        {
                            if ((Graph::getBit( pAdjBitMatrix, (j + i * pNumVertices)) && (color[j] != -1)))
                            {
                                //printf("vertexColor[%d] is %d\n", j, vertexColor[j]);                 
                                tempColorSlot[color[j]] = 1;
                                //printf("%d color match with %d\n", i, j);
                            }
                        }

                        //The first unused color slot is used to assign color to i
                        for (unsigned int j = 0; j < maxDegree; j++)
                        {
                            if (tempColorSlot[j] == 0)
                            {
                                color[i] = j;
                                break;
                            }
                        }
                    }//end if
                }//end for

                //Printing the VIS of the vertices along with the color of the vertices
                for (i = 0; i < pNumVertices; i++)
                {
                    unsigned int offset = i * pNumVertices;
                    printf( "VIS " );
                    PRINT_VERT(rGraph, i );
                    printf(" -> %d", color[i]);
                    printf( ": " );

                    for( size_t j = 0; j < pNumVertices; ++j )
                    {
                        if( Graph::getBit( h_groups, offset + j ) )
                        {
                            PRINT_VERT(rGraph, j );
                            //printf( "%d : %d", j, vertexColor[j] );
                        }
                    }
                    printf( "\n" );
                }//end for
        
        END_PROFILING;
    }

//end if
    clReleaseMemObject(d_adj);
    clReleaseMemObject(d_non_adj);
    clReleaseMemObject(d_groups);

    free(h_groups);

    return lRet;
}
}
}