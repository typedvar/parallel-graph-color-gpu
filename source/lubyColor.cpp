#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <CL/cl.h>
#include "utils.h"
#include "defines.h"
#include "graph.h"

bool lubyColor( const Graph& rGraph,
                cl_command_queue commands,
                cl_context& context, 
                cl_kernel& kernel,
                cl_program& program,
                unsigned int* adjacents,
                size_t mem_size,
                size_t num_vertices )
{
    int allDone = num_vertices;
    unsigned int* h_d;
    unsigned int* h_can;
    unsigned int* h_is;
    unsigned int* color;
    bool lRet = true;

    size_t* work_item_counts = 0;
    int* entry_counts = 0;

    h_d = (unsigned int*) malloc (sizeof(unsigned int) * num_vertices);

    size_t i = 0;
    for( i = 0; i < num_vertices; ++i )
    {
        h_d[i] = i;
    }

    h_can = ( unsigned int* ) malloc( sizeof( unsigned int ) * num_vertices );
    color = ( unsigned int* ) malloc( sizeof( unsigned int ) * num_vertices );

    for (i = 0; i < num_vertices; i++)
    {
        h_can[i] = 1;
        color[i] = -1;
    }

    h_is = (unsigned int*) malloc (sizeof(unsigned int) * num_vertices);

    printf("Created all the host array\n");

    size_t WorkSize[] = { num_vertices }; // one dimensional Range
    int num = num_vertices;

    cl_mem d_adj;
    cl_mem d_d;
    cl_mem d_is;
    cl_mem d_can;

    // Create the input buffer on the device for adjacency matrix
    d_adj = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, mem_size, adjacents, NULL);
    if (!d_adj)
    {
        printf("Error: Failed to allocate input databuffer on device!\n");
        lRet = false;
    }

    if( lRet )
    {
        printf("Created the adj buffer\n");
        // Create the input buffer on the device for probabilty matrix

        d_d = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int) * num_vertices, h_d, NULL);

        if (!d_d)
        {
            printf("Error: Failed to allocate input data buffer on device!\n");
            lRet = false;
        }
    }

    if( lRet )
    {
        START_PROFILING;

        printf("Created the probabilty buffer\n");
        cl_int err;

        //Set the kernel argument
        err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *) &d_adj);
        err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *) &d_d);
        err |= clSetKernelArg(kernel, 3, sizeof(unsigned int), &num_vertices );

        printf("Populated kernel arguments\n");
        
        while( allDone > 0 )
        {
            // Create the candidate buffer on the device
            //
            d_can = clCreateBuffer( context, 
                                    CL_MEM_READ_WRITE| CL_MEM_COPY_HOST_PTR,
                                    sizeof( unsigned int ) * num_vertices, 
                                    h_can,
                                    NULL );

            if (!d_can)
            {
                printf("Error: Failed to allocate candidate buffer on device!\n");
                lRet = false;
                break;
            }

            memset( h_is, 0x0, ( sizeof( unsigned int ) * num_vertices ) );

            // Create the IS buffer on the device
            //
            d_is = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(unsigned int) * num_vertices, h_is, NULL);

            if (!d_is)
            {
                printf("Error: Failed to allocate IS buffer on device!\n");
                lRet = false;
                break;
            }

            err = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *) &d_can);
            err |= clSetKernelArg(kernel, 4, sizeof(cl_mem), (void *) &d_is);

            //Launch kernel
            cl_event lEventNDRangeKernel;
            err = clEnqueueNDRangeKernel(commands, kernel, 1, 0, WorkSize, NULL, 0, NULL, &lEventNDRangeKernel);

            if (err != CL_SUCCESS)
            {
                printf("Error: Failed to execute kernel!\n");
                getchar();
                lRet = false;
                break;
            }

            PROFILE_EVENT( lEventNDRangeKernel, "NDRangeKernel" );

            // Read back the Independent set that was computed on the device
            //
            cl_event lEventReadBufferIS;
            err = clEnqueueReadBuffer(commands, d_is, CL_TRUE, 0, sizeof(unsigned int) * num_vertices, h_is, 0, NULL, &lEventReadBufferIS );
            if (err)
            {
                printf("Error: Failed to read back independent set from the device!\n");
                lRet = false;
                break;
            }

            PROFILE_EVENT( lEventReadBufferIS, "GPU2HOSTRead IS" );

            printf("IS : ");
            for (i = 0; i < num_vertices; i++)
            {
                if (color[i] == -1)
                    h_can[i] = 1;
                if (h_is[i] == 1)
                {
                    PRINT_VERT( rGraph, i );
                    h_can[i] = 0;
                    --allDone;
                    color[i] = 1;
                }
            }
            printf( "\n" );
        }//allDone Loop

        END_PROFILING;
    }

    clReleaseMemObject(d_adj);
    clReleaseMemObject(d_d);
    clReleaseMemObject(d_can);
    clReleaseMemObject(d_is);

    free(h_d);
    free(h_can);
    free(h_is);

    return lRet;
}
