#ifndef _GRAPHCOLOR_UTILS_H_
#define _GRAPHCOLOR_UTILS_H_

#include "CL/cl.h"

#include "defines.h"

void profile_event( cl_event pEvent, const char* pEventTitle, float& rTotalTime );
char* loadProgSource( const char* cFilename, size_t* szFinalLength );

bool initOCL( cl_device_id& device_id, cl_context& context, cl_command_queue& commands );
bool createKernelFromSource( const char* pSourceFile,
                             cl_device_id& device_id, 
                             cl_context& context, 
                             cl_program& program, 
                             cl_command_queue& commands, 
                             cl_kernel& kernel, 
                             const char* kernelName );

#ifdef _GRAFCOLOR_ENABLE_OCL_PROFILING_

#define START_PROFILING float grafcolor_ocl_profile_total_time = 0
#define PROFILE_EVENT( x, y ) profile_event( x, y, grafcolor_ocl_profile_total_time )
#define END_PROFILING printf( "\nTotal OCL device execution time %f millisecs\n", grafcolor_ocl_profile_total_time )

#else

#define START_PROFILING
#define PROFILE_EVENT( x, y )
#define END_PROFILING

#endif // _GRAFCOLOR_ENABLE_OCL_PROFILING_

#ifdef _GRAFCOLOR_STATIC_DATA_

#define PRINT_VERT( X ) \
    { \
        printf( "%d ", X ); \
    }

#else

#define PRINT_VERT( graph, vertex ) \
    { \
        std::string lVertexName; \
        if( graph.getName( vertex, lVertexName ) ) \
        { \
            printf( "%s ", lVertexName.c_str() ); \
        } \
    }

#endif


#endif
