bool isConflicting( constant unsigned int* adjacents,
                    unsigned int rows,
                    __global unsigned int* group,
                    unsigned int curr_vertex,
                    unsigned int num_filled,
                    unsigned int non_neighbor )
{
    unsigned int offset =  curr_vertex * rows;

    for( int i = 0; i < rows; ++i )
    {
        unsigned int already_added;
        if( 1 == group[ offset + i ] )
            already_added = i;
        if( 1 == adjacents[ already_added + rows * non_neighbor ] )
        {
            return true;
        }
    }

    return false;
}

void colorGraph( constant unsigned int* adjacents,
                 unsigned int curr_vertex,
                 unsigned int rows, 
                 __global unsigned int* group,
                 __global int* color )
{                
    //Variables needed for colouring the vertices
    unsigned int assignColor = 0;
    unsigned int offset =  curr_vertex * rows;

    bool isColorAssigned = false;
    bool duplicateNode = false;

    for( size_t j = 0; j < rows; ++j )
    {
        if( 1 == group[offset + j] )
        {
            if( color[j] != -1 )
            {
                duplicateNode = true;
                break;
            }
        }
    }

    if( !duplicateNode )
    {
        for( size_t j = 0; j < rows; ++j )
        {
            if( 1 == group[offset + j] )
            {
                color[j] = assignColor;
                isColorAssigned = true;
            }
        }
    }

    if( isColorAssigned )
    {
        ++assignColor;
    }

    assignColor = 0;

    if( color[curr_vertex] == -1 )
    {
        for( unsigned int j = 0; j < rows; j++ )
        {
            if( ( adjacents[j + offset] == 1 ) && ( j != curr_vertex ) )
            {
                if (color[j] == assignColor)
                    ++assignColor;
            }
        }
        color[curr_vertex] = assignColor;
    }
}

__kernel void kernelColor( constant unsigned int* adjacents,
                           constant unsigned int* non_adjacents,
                           int rows, 
                           __global unsigned int* group,
                           __global int* colors,
                           unsigned int serial_coloring,
                           __global unsigned int* assignedColor )
{
    unsigned int curr_vertex = get_global_id( 0 );
    unsigned int offset =  curr_vertex * rows;
    unsigned int offset_non =  offset + curr_vertex + 1;
    unsigned int num_items = non_adjacents[offset_non - 1];
    unsigned int num_filled = 1;

    // self should always be a part of the IVS
    group[offset + curr_vertex] = 1;

    // the non_adjacents[ 0 ] element is the number of elements in the non_adjacents location of v;
    for( int i = 0; i < num_items; ++i )
    {
        unsigned int non_neighbor = non_adjacents[ offset_non + i ];

        if( curr_vertex != non_neighbor )
        {
            if( isConflicting( adjacents, rows, group, curr_vertex, num_filled, non_neighbor ) )
            {
                continue;
            }
            else
            {
                while( 1 == atom_cmpxchg( &group[ offset + non_neighbor ], 0, 1 ) );
                ++num_filled;
            }
        }
    }

    if( !serial_coloring )
    {
        colorGraph( adjacents, curr_vertex, rows, group, colors );
    }
}
