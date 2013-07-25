typedef unsigned char byte_t;
const size_t BYTE_SIZE = 8;

bool getBit( __global byte_t* pBitMatrix, unsigned int pBitOffset )
{
    unsigned int lByteNum = pBitOffset / BYTE_SIZE;
    unsigned int lBitPos = pBitOffset % BYTE_SIZE;
    return ( 0 != ( pBitMatrix[lByteNum] & ( 0x1 << ( BYTE_SIZE - ( lBitPos + 1 ) ) ) ) );
}

void setBit( __global byte_t* pBitMatrix, unsigned int pBitOffset, bool pVal )
{
    size_t lByteNum = pBitOffset / BYTE_SIZE;
    int lBitPos = pBitOffset % BYTE_SIZE;

    if( pVal )
    {
        pBitMatrix[lByteNum] = pBitMatrix[lByteNum] | ( 0x1 << ( BYTE_SIZE - ( lBitPos + 1 ) ) );
    }
    else
    {
        pBitMatrix[lByteNum] = pBitMatrix[lByteNum] & ~( 0x1 << ( BYTE_SIZE - ( lBitPos + 1 ) ) );
    }
}

bool isConflicting( __global byte_t* adjacents,
                    unsigned int pNumVertices,
                    __global byte_t* group,
                    unsigned int curr_vertex,
                    unsigned int non_neighbor )
{
    unsigned int offset = curr_vertex * pNumVertices;
    for( int i = 0; i < pNumVertices; ++i )
    {
        if( getBit( group, ( offset + i ) ) )
        {
            if( getBit( adjacents, ( i + pNumVertices * non_neighbor ) ) )
            {
                return true;
            }
        }
    }
    return false;
}

__kernel void kernelColor( __global byte_t* adjacents,
                           constant unsigned int* non_adjacents,
                           constant unsigned int* non_adj_offset_array,
                           int non_adjacents_num_elems,
                           int pNumVertices, 
                           __global byte_t* group )
{
    unsigned int curr_vertex = get_global_id( 0 );

    unsigned int offset = curr_vertex * pNumVertices;
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
    setBit( group, ( offset + curr_vertex ), 1 );

    // the non_adjacents[ 0 ] element is the number of elements in the non_adjacents location of v;
    for( int i = 0; i < num_items; ++i )
    {
        unsigned int non_neighbor = non_adjacents[ offset_non + i ];

        if( curr_vertex != non_neighbor )
        {
            if( isConflicting( adjacents, 
                               pNumVertices, 
                               group, 
                               curr_vertex, 
                               non_neighbor ) )
            {
                continue;
            }
            else
            {
                //while( 1 == atom_cmpxchg( &group[ offset + non_neighbor ], 0, 1 ) );
                setBit( group, ( offset + non_neighbor ), 1 );
                ++num_filled;
            }
        }
    }
}

// end of file
