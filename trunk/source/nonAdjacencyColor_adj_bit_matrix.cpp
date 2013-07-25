#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <CL/cl.h>
#include "utils.h"
#include "defines.h"
#include "graph.h"

bool nonAdjacencyColor( const Graph& rGraph,
                        cl_command_queue commands,
                        cl_context& context, 
                        cl_kernel& kernel,
                        cl_program& program,
                        //unsigned int* adjacents,
                        Graph::byte_t* adjacents,
                        size_t adj_size,
                        unsigned int* non_adjacents,
                        size_t non_adj_size,
                        size_t num_vertices )
{
    int allDone = num_vertices;
    bool lRet = true;

    size_t* work_item_counts = 0;
    int* entry_counts = 0;

    size_t color_size = sizeof( unsigned int ) * num_vertices;
    int* color = ( int* ) malloc( color_size );
    std::fill( color, color + num_vertices, -1 );

    unsigned int *h_groups;
    size_t num_group_elems = num_vertices * num_vertices;
    size_t groups_size = sizeof( unsigned int ) * num_group_elems;

    h_groups = ( unsigned int* ) malloc( groups_size );
    std::fill( h_groups, h_groups + num_group_elems, 0 );

    unsigned int h_serial_coloring = 0;
    unsigned int h_assigned_color = 0;

#ifdef _GRAFCOLOR_SERIAL_
    h_serial_coloring = 1;
#endif

    printf("Created all the host array\n");

    size_t WorkSize[] = { num_vertices }; // one dimensional Range
    int num = num_vertices;

    // Create the input buffer on the device for adjacency matrix
    cl_mem d_adj = clCreateBuffer( context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, adj_size, adjacents, NULL );
    if (!d_adj)
    {
        printf("Error: Failed to allocate input databuffer on device!\n");
        lRet = false;
    }

    // Create the input buffer on the device for adjacency matrix
    cl_mem d_non_adj = clCreateBuffer( context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, non_adj_size, non_adjacents, NULL );
    if (!d_non_adj)
    {
        printf("Error: Failed to allocate input databuffer on device!\n");
        lRet = false;
    }

    cl_mem d_groups = clCreateBuffer( context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, groups_size, h_groups, NULL );
    if ( !d_groups )
    {
        printf("Error: Failed to allocate input databuffer on device!\n");
        lRet = false;
    }

    cl_mem d_colors = clCreateBuffer( context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, color_size, color, NULL );
    if ( !d_colors )
    {
        printf("Error: Failed to allocate input databuffer on device!\n");
        lRet = false;
    }

    cl_mem d_assignedColor = clCreateBuffer( context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof( unsigned int ), &h_assigned_color, NULL );
    if ( !d_assignedColor )
    {
        printf("Error: Failed to allocate input databuffer on device!\n");
        lRet = false;
    }

    if( lRet )
    {
        START_PROFILING;
        cl_int err;

        //Set the kernel argument
        err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *) &d_adj );
        err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *) &d_non_adj );
        err |= clSetKernelArg(kernel, 2, sizeof(unsigned int), &num_vertices );
        err |= clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *) &d_groups );
        err |= clSetKernelArg(kernel, 4, sizeof(cl_mem), (void *) &d_colors );
        err |= clSetKernelArg(kernel, 5, sizeof(unsigned int), (void *) &h_serial_coloring );
        err |= clSetKernelArg(kernel, 6, sizeof(unsigned int), (void *) &d_assignedColor );

        if( CL_SUCCESS == err )
        {
            printf("Populated kernel arguments\n");
            cl_event lEventNDRangeKernel;
            err = clEnqueueNDRangeKernel( commands, kernel, 1, 0, WorkSize, NULL, 0, NULL, &lEventNDRangeKernel );

            if (err != CL_SUCCESS)
            {
                printf("Error: Failed to execute kernel!\n");
                getchar();
                lRet = false;
            }

            PROFILE_EVENT( lEventNDRangeKernel, "NDRangeKernel" );

            if( lRet )
            {
                if( h_serial_coloring )
                {
                    printf("\nPerforming serial coloring ...\n");
                    cl_event lEventReadBufferGroups;
                    err = clEnqueueReadBuffer(commands, d_groups, CL_TRUE, 0, groups_size, h_groups, 0, NULL, &lEventReadBufferGroups );
                    if( err )
                    {
                        printf("Error: Failed to read back groups from the device!\n");
                        lRet = false;
                    }

                    PROFILE_EVENT( lEventReadBufferGroups, "GPU2HOSTRead Groups" );


                    size_t i = 0;
                    //Variables needed for colouring the vertices

                    unsigned int assignColor = 0;
                    unsigned int isAssignColor = 0;
                    unsigned int duplicateNode = 0;

                    for (i = 0; i < num_vertices; i++)
                    {
                        unsigned int offset = i * num_vertices;
                        isAssignColor = 0;
                        duplicateNode = 0;

                        for( size_t j = 0; j < num_vertices; ++j )
                        {
                            if( 1 == h_groups[offset + j] )
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
                            for( size_t j = 0; j < num_vertices; ++j )
                            {
                                if( 1 == h_groups[offset + j] )
                                {
                                    color[j] = assignColor;
                                    isAssignColor = 1;
                                }
                            }
                        }

                        if (isAssignColor == 1)
                        {
                            ++assignColor;
                        }
                    }

                    printf( "Unique assignment done using %d colors.\n", assignColor - 1  );

                    for (i = 0; i < num_vertices; i++)
                    {
                        assignColor = 0;

                        if (color[i] == -1)
                        {
                            for (unsigned int j = 0; j < num_vertices; j++)
                            {
                                if( Graph::getBit( adjacents, ( j + i * num_vertices ) ) && ( j != i ) )
                                {
                                    if (color[j] == assignColor)
                                        ++assignColor;
                                }
                            }
                            color[i] = assignColor;
                        }
                    }

                    printf( "Non-unique assignment done using %d colors.\n", assignColor );

                    //Printing the VIS of the vertices along with the color of the vertices
                    for (i = 0; i < num_vertices; i++)
                    {
                        unsigned int offset = i * num_vertices;
                        printf( "VIS " );
                        PRINT_VERT( i );
                        printf("/%d", color[i]);
                        printf( ": " );

                        for( size_t j = 0; j < num_vertices; ++j )
                        {
                            if( 1 == h_groups[offset + j] )
                            {
                                printf( "%d ", j);
                                //printf( "%d : %d", j, color[j] );
                            }
                        }
                        printf( "\n" );
                    }
                }
                else
                {
                    printf("\nPerforming parallel coloring on GPU...\n");
                    cl_event lEventReadBufferColors;
                    err = clEnqueueReadBuffer(commands, d_colors, CL_TRUE, 0, color_size, color, 0, NULL, &lEventReadBufferColors );
                    if (err)
                    {
                        printf("Error: Failed to read back colors from the device!\n");
                        lRet = false;
                    }

                    PROFILE_EVENT( lEventReadBufferColors, "GPU2HOSTRead Colors" );

                    printf("\nPrinting the colors ...\n");

                    for( size_t j = 0; j < num_vertices; ++j )
                    {
                        PRINT_VERT( j );
                        printf( " : " );
                        printf( "%d\n", color[j]);
                    }
                }
            }

            END_PROFILING;
        }
    }

    clReleaseMemObject(d_adj);
    clReleaseMemObject(d_non_adj);
    clReleaseMemObject(d_groups);

    free(h_groups);

    return lRet;
}
