#include <algorithm>
#include <iostream>
#include <ostream>
#include <iterator>

#include "graphloader.h"
#include "graph.h"

int main(int argc, char **argv)
{
    // load input graph
    if( argc < 2 )
    {
        std::cout << "Usage: " << argv[0] << " <graph file>" << std::endl;
        return 1;
    }

    Graph lGraph;
    GraphLoader lLoader;
    lLoader.loadInput( argv[1], lGraph );

    if( !lGraph.empty() )
    {
        lGraph.printAdjacencyList( std::cout );

        Graph::vertexId_t* lMatrix = NULL;
        size_t lNumElems = 0;

        if( lGraph.getAdjacencyMatrix( lMatrix, lNumElems ) )
        {
            lGraph.printMatrix( std::cout, lMatrix, lNumElems );
        }
    }

    return 0;
}
