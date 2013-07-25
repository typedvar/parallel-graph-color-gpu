#ifndef _LUBYCOLOR_H_
#define _LUBYCOLOR_H_

bool lubyColor( const Graph& rGraph,
                cl_command_queue commands,
                cl_context& context, 
                cl_kernel& kernel,
                cl_program& program,
                unsigned int* adjacents,
                size_t mem_size,
                size_t num_vertices );


#endif
