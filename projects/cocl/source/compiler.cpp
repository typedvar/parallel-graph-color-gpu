#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <CL/cl.h>

void usage( const char* pProgramName )
{
    printf( "usage: %s <OpenCL code file>\n", pProgramName );
}

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

bool initOCL( cl_device_id& device_id, cl_context& context, cl_command_queue& commands )
{
    cl_int err;

    // Connect to a GPU compute device
    err = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to create a device group!\n");
        return false;
    }

    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
    if (!context)
    {
        printf("Error: Failed to create a compute context!\n");
        return false;
    }

    commands = clCreateCommandQueue(context, device_id, 0, &err);
    if (!commands)
    {
        printf("Error: Failed to create a command commands!\n");
        return false;
    }
    return true;
}

bool createKernelFromSource( const char* pSourceFile,
                             cl_device_id& device_id, 
                             cl_context& context, 
                             cl_program& program, 
                             cl_command_queue& commands, 
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

    printf( "Success\n" );
    return true;
}

int main(int argc, char **argv)
{
    cl_device_id device_id;
    cl_command_queue commands;
    cl_context context;

    if( argc < 1 )
    {
        usage( argv[0] );
        return 1;
    }

    const char* lKernelFile = argv[1];

    if( !initOCL( device_id, context, commands ) )
    {
        return EXIT_FAILURE;
    }

    // Create program and kernel
    cl_program program;

    if( !createKernelFromSource( lKernelFile,
                                 device_id, 
                                 context, 
                                 program, 
                                 commands,
                                 "" ) )
    {
        return EXIT_FAILURE;
    }

    clReleaseProgram(program);
    clReleaseCommandQueue(commands);
    clReleaseContext(context);

    return 0;
}
