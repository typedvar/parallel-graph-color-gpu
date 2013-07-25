#ifndef _NON_ADJACENCY_COLOR_H_
#define _NON_ADJACENCY_COLOR_H_

bool nonAdjacencyColor( const Graph& rGraph,
                        cl_command_queue commands,
                        cl_context& context, 
                        cl_kernel& kernel,
                        cl_program& program,
                        Graph::byte_t* adjacents,
                        size_t adj_size,
                        Graph::vertexId_t* non_adjacents,
                        size_t non_adj_size,
                        Graph::vertexId_t* non_adj_offset_array,
                        size_t num_vertices );

#endif
