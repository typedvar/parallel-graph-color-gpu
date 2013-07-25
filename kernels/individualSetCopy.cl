typedef unsigned char byte_t;
const size_t BYTE_SIZE = 8;

bool getBit( constant byte_t* bit_matrix, unsigned int offset )
{
    unsigned int lByteNum = offset / BYTE_SIZE;
    unsigned int lBitPos = offset % BYTE_SIZE;
    byte_t lVertexByte = bit_matrix[lByteNum];
    return ( 0 != ( lVertexByte & ( 0x1 << ( BYTE_SIZE - ( lBitPos + 1 ) ) ) ) );
}

bool isConflicting( constant byte_t* adjacents,
                    unsigned int pNumVertices,
                    __global unsigned int* group,
                    unsigned int curr_vertex,
                    unsigned int num_filled,
                    unsigned int non_neighbor )
{
    unsigned int offset =  curr_vertex * pNumVertices;

    for( int i = 0; i < pNumVertices; ++i )
    {
        unsigned int already_added;
        if( 1 == group[ offset + i ] )
            already_added = i;
        if( getBit( adjacents, ( already_added + pNumVertices * non_neighbor ) ) )
        {
            return true;
        }
    }

    return false;
}

__kernel void kernelColor( constant byte_t* adjacents,
                           constant unsigned int* non_adjacents,
                           constant unsigned int* non_adj_offset_array,
                           int non_adjacents_num_elems,
                           int pNumVertices, 
                           __global unsigned int* group,
                           __global int* colors )
{
    unsigned int curr_vertex = get_global_id( 0 );

    unsigned int offset =  curr_vertex * pNumVertices;
    unsigned int offset_non = non_adj_offset_array[curr_vertex];

    unsigned int num_items = 0;
    
    if( curr_vertex < ( pNumVertices - 1 ) )
    {
        num_items = non_adj_offset_array[curr_vertex + 1] - offset_non;
    }
    else
    {
        num_items = non_adjacents_num_elems - offset_non;
    }

    unsigned int num_filled = 1;

    // self should always be a part of the IVS
    group[offset + curr_vertex] = 1;

    // the non_adjacents[ 0 ] element is the number of elements in the non_adjacents location of v;
    for( int i = 0; i < num_items; ++i )
    {
        unsigned int non_neighbor = non_adjacents[ offset_non + i ];

        if( curr_vertex != non_neighbor )
        {
            if( isConflicting( adjacents, pNumVertices, group, curr_vertex, num_filled, non_neighbor ) )
            {
                continue;
            }
            else
            {
                //while( 1 == atom_cmpxchg( &group[ offset + non_neighbor ], 0, 1 ) );
                group[ offset + non_neighbor ] = 1;
                ++num_filled;
            }
        }
    }
}
