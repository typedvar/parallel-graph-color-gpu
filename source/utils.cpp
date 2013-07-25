#include "utils.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

char* loadProgSource(const char* cFilename, size_t* szFinalLength)
{
    // locals 
    FILE* pFileStream = NULL;
    size_t szSourceLength;

    // open the OpenCL source code file
#ifdef _WIN32   // Windows version
    if(fopen_s(&pFileStream, cFilename, "rb") != 0) 
    {       
        return NULL;
    }
#else           // Linux version
    pFileStream = fopen(cFilename, "rb");
    if(pFileStream == 0) 
    {       
        return NULL;
    }
#endif

    // get the length of the source code
    fseek(pFileStream, 0, SEEK_END); 
    szSourceLength = ftell(pFileStream);
    fseek(pFileStream, 0, SEEK_SET); 

    // allocate a buffer for the source code string and read it in
    char* cSourceString = (char *)malloc(szSourceLength + 1 ); 
    if ( fread( cSourceString, szSourceLength, 1, pFileStream ) != 1 )
    {
        fclose(pFileStream);
        free(cSourceString);
        return 0;
    }

    // close the file and return the total length of the combined (preamble + source) string
    fclose(pFileStream);
    if(szFinalLength != 0)
    {
        *szFinalLength = szSourceLength;
    }
    cSourceString[szSourceLength] = '\0';

    return cSourceString;
}

void profile_event( cl_event pEvent, const char* pEventTitle, float& pTotalTime )
{
    cl_ulong start = 0;
    cl_ulong end = 0;
    clGetEventProfilingInfo( pEvent, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL );
    clGetEventProfilingInfo( pEvent, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL );
    float executionTimeInMilliseconds = ( end - start ) * 1.0e-6f;
    pTotalTime += executionTimeInMilliseconds;

#ifdef _DEBUG
    printf( "\n> Time %s: %f millisecs\n", pEventTitle, executionTimeInMilliseconds );
#endif
}

bool initOCL( cl_device_id& device_id, cl_context& context, cl_command_queue& commands )
{
    cl_int err;

    cl_device_type lDeviceType = CL_DEVICE_TYPE_DEFAULT;

#ifdef _GRAFCOLOR_DEBUG_KERNEL_
    lDeviceType = CL_DEVICE_TYPE_CPU;
#else
    lDeviceType = CL_DEVICE_TYPE_GPU;
#endif

    // Connect to a GPU compute device
    err = clGetDeviceIDs(NULL, lDeviceType, 1, &device_id, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to create a device group!\n");
        return false;
    }

    printf("Connected to GPU compute device\n");
    // Create a compute context
    //
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
    if (!context)
    {
        printf("Error: Failed to create a compute context!\n");
        return false;
    }

    printf("Created compute context\n");
    // Create a command queue
    //
    commands = clCreateCommandQueue(context, device_id, 0, &err);
    if (!commands)
    {
        printf("Error: Failed to create a command commands!\n");
        return false;
    }
    printf("Created command queue\n");

#ifdef _GRAFCOLOR_ENABLE_OCL_PROFILING_
    clSetCommandQueueProperty( commands, CL_QUEUE_PROFILING_ENABLE, CL_TRUE, NULL );
#endif

    return true;
}

bool createKernelFromSource( const char* pSourceFile,
                             cl_device_id& device_id, 
                             cl_context& context, 
                             cl_program& program, 
                             cl_command_queue& commands, 
                             cl_kernel& kernel, 
                             const char* kernelName )
{
    size_t lSrcLen = 0;
    char* lSource = loadProgSource( pSourceFile, &lSrcLen );

    if( !lSource )
    {
        return false;
    }

    cl_int err;

    // Create the program
    program = clCreateProgramWithSource( context, 1, (const char **)&lSource, &lSrcLen, &err );
    if (err != CL_SUCCESS)
    {
        printf( "Error creating program from %s\n", pSourceFile );
        return false;
    }

    printf( "Building OpenCL program from %s... ", pSourceFile );

    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        size_t len;
        char buffer[8192];

        clGetProgramBuildInfo(
            program,             // the program object being queried
            device_id,            // the device for which the OpenCL code was built
            CL_PROGRAM_BUILD_LOG, // specifies that we want the build log
            sizeof(buffer),       // the size of the buffer
            buffer,               // on return, holds the build log
            &len);                // on return, the actual size in bytes of the

        printf( "Error building program %s: %s\n", pSourceFile, buffer );
        return false;
    }

    printf( "done\n" );
    // Create the kernel
    kernel = clCreateKernel( program, kernelName, &err );

    if( err != CL_SUCCESS )
    {
        printf( "Error creating kernel %s from %s\n", kernelName, pSourceFile );
        return false;
    }

    return true;
}

//void readAdj(unsigned int adj[], unsigned int num_vertices)
//{
//    printf("Enter the adjacency matrix\n");
//    for (unsigned int i = 0; i < num_vertices; i++)
//    {
//        for (unsigned int j = 0; j < num_vertices; j++)
//        {   
//            adj[j + num_vertices * i] = 0;
//            printf("adj[%d][%d] = ", i, j);
//            sscanf("%u", &adj[j + num_vertices * i]);        
//        }
//    }
//}

// end of file
