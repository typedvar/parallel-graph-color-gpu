#include "graph.h"
#include "graphGen.h"
#include <iostream>
#include <string>
#include <sstream>

int main( int argc, char* argv[] )
{
    if( argc < 2 )
    {
        std::cout << "usage : " 
                  << argv[0] 
                  << " <number of vertices> [completeness factor (default 0.5)]" 
                  << std::endl;
        return 1;
    }

    bool lCompletenessProvided = ( argc == 3 );

    size_t lNumVertices = atoi( argv[1] );

    float lCompleteness = 0.5;
    if( lCompletenessProvided )
    {
        lCompleteness = atof( argv[2] );
    }

    GraphGen lGenerator( lNumVertices, lCompleteness );

    Graph lGraph;
    lGenerator.generate( lGraph );

    std::stringstream lStrStream;

    lGraph.print( lStrStream );

    std::cout << lStrStream.str();

    return 0;
}


