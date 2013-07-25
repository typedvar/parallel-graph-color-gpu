void getCandidateSet( constant unsigned int* adj, 
                      __global unsigned int* d, 
                      __global unsigned int* can, 
                      int rows, 
                      __global unsigned int* is )
{
    unsigned int v = get_global_id(0);
    unsigned int cmp = 1;
    unsigned int isolated = 0;
    unsigned int val = 0;
    unsigned short is_highest_degree = 0;
    unsigned int offset = v * rows;

    if (can[v] == 1)
    {
        //For vertex with degree zero
        isolated = 1;
        for(int i = 0; i < rows; i++)
        {
            if ((adj[i + offset] == 1) && (can[i] == 1))
            {
                isolated = 0;
            }
        }

        if (isolated == 1)
        {
            is[v] = 1;
            can[v] = 0;
            return;
        }

        //for loop iterates only for those vertices that are yet a part of candidate set
        for(int i = 0; i < rows; i++)
        {
            if ((adj[i + offset] == 1)  && (can[i] == 1)) 
            {
                if (d[v] < d[i])
                {
                    is_highest_degree = 0;
                }
                else
                {
                    is_highest_degree = 1;
                }
            }
        }

        if (is_highest_degree == 1) 
        {
            is[v] = 1;
            can[v] = 0;

            for(int i = 0; i < rows; i++)
            {
                if (adj[i + offset] == 1)
                {
                    can[i] = 0;
                }
            }
        }
    }
}

__kernel void getISSet( constant unsigned int* adj, 
                        __global unsigned int* d, 
                        __global unsigned int* can, 
                        int rows, 
                        __global unsigned int* is )
{
    unsigned int num = rows;
    
    while(num > 0)
    {
        getCandidateSet( adj, d, can, rows, is );
        num = 0;
        for( int i = 0; i < rows; i++ )
        {
            if( can[i] == 1 )
            {
                ++num;
            }
        }    
    }
}
