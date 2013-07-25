#include "graph.h"
#include <cmath>
#include <ctime>
#include <string>
#include <sstream>

class GraphGen
{
public:
    GraphGen( size_t pNumVertices, float pCompleteness = 0.5 ) 
            : mNumVertices( pNumVertices )
            , mCompleteness( pCompleteness )
            , mNumEdges( 0 )
    {
        time_t lNow = time( NULL );
        ::srand( lNow );
    }

    bool generate( Graph& rGraph )
    {
        mNumEdges = mNumVertices * ( mNumVertices - 1 );
        mNumEdges = mNumEdges >> 1;
        mNumEdges *= mCompleteness;

        for( size_t j = 0; j < mNumVertices; ++j )
        {
            std::stringstream lStrStream;
            lStrStream << j;
            Graph::vertexId_t lVertex;
            rGraph.addVertex( lStrStream.str(), lVertex );
        }

        for( size_t i = 0; i < mNumEdges; ++i )
        {
            generateEdge( rGraph );
        }

        return true;
    }

private:
    void getRandomVertexPair( Graph::idPair_t& rVertexPair )
    {
        Graph::vertexId_t lVertex1 = 0;
        Graph::vertexId_t lVertex2 = 0;

        lVertex1 = getRandom();
        lVertex2 = getRandom();

        while( lVertex1 == lVertex2 )
        {
            lVertex2 = getRandom();
        }

        rVertexPair = Graph::idPair_t( lVertex1, lVertex2 );
    }

    bool generateEdge( Graph& rGraph )
    {
        Graph::idPair_t vertexPair( 0, 0 );
        getRandomVertexPair( vertexPair );

        while( rGraph.isEdge( vertexPair.first, vertexPair.second ) )
        {
            getRandomVertexPair( vertexPair );
        }

        rGraph.addEdge( vertexPair.first, vertexPair.second );
        return true;
    }

private:
    unsigned int getRandom()
    {
        return ::rand() % mNumVertices;
    }

    size_t mNumVertices;
    float mCompleteness;
    size_t mNumEdges;
};

// end of file
