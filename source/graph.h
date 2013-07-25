#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <map>
#include <set>
#include <vector>
#include <string>
#include <ostream>
#include "nonAdjacencyNode.h"

struct NumericLess
{
    bool operator() ( std::string& pLhs, std::string& pRhs )
    {
        int lLhs = atoi( pLhs.c_str() );
        int lRhs = atoi( pRhs.c_str() );

        return ( lLhs < lRhs );
    }
};

class Graph
{
public:
    typedef unsigned char byte_t;
    typedef unsigned int vertexId_t;

    typedef std::set<vertexId_t> idSet_t;
    typedef idSet_t::iterator idSetIter_t;
    typedef idSet_t::const_iterator idSetConstIter_t;
    typedef std::vector<idSet_t> idSetStore_t;

    typedef std::vector<vertexId_t> idVec_t;
    
    typedef std::map<const std::string, vertexId_t> string2IdMap_t;
    typedef std::pair<const std::string, vertexId_t> string2IdPair_t;
    typedef string2IdMap_t::const_iterator string2IdMapConstIter_t;

    typedef std::map<const vertexId_t, const std::string> id2StringMap_t;
    typedef std::pair<const vertexId_t, const std::string> id2StringPair_t;
    typedef id2StringMap_t::const_iterator id2StringConstIter_t;
        
    typedef std::set<std::string> stringSet_t;
    typedef stringSet_t::const_iterator stringSetConstIter_t;

    typedef std::pair<vertexId_t, vertexId_t> idPair_t;
    typedef std::vector<idPair_t> idPairStore_t;
    typedef idPairStore_t::const_iterator idPairStoreConstIter_t;

    static const char COMMENT_CHAR;
    static const int MAX_LINE_SIZE;
    static const int BYTE_SIZE;

    Graph();
    Graph( size_t pNumVertices );

    bool addVertex( const std::string& pVertexName, vertexId_t& rVertexId );

    bool addEdge( vertexId_t& rFirst, vertexId_t& rSecond );

    bool isEdge( const Graph::vertexId_t& rFirst, const Graph::vertexId_t& rSecond ) const
    {
        const idSet_t& rFirstSet = mAdjacencyLists[rFirst];
        return ( rFirstSet.find( rSecond ) != rFirstSet.end() );
    }

    bool isValidId( const vertexId_t& rId ) const
    {
        return ( rId < mVertexName2IdMap.size() );
    }

    bool empty() const
    {
        return mVertexName2IdMap.empty();
    }

    static bool getBit( const Graph::byte_t* pBitMatrix, size_t pBitOffset );

    static void setBit( Graph::byte_t* pBitMatrix, size_t pBitOffset, bool pVal );

    // Helper function: Copies the non adjacency list array data to contiguous memory
    //                  for passing to GPU, and updates the stream idx array with
    //                  appropriate indices
    static void marshallAdjacencyListArray( const NonAdjacencyNode_t* pNonAdjListArray, 
                                            size_t pNumElems, 
                                            vertexId_t*& rNonAdjStream,
                                            size_t& rNonAdjStreamSize,
                                            vertexId_t*& rNonAdjStreamIdxArray );

    bool getId( const std::string& rVertexName, vertexId_t& rId ) const;
    
    bool getName( const vertexId_t& rId, std::string& rVertexName ) const;

    bool getAdjacencyList( const vertexId_t& rId, idSet_t& rList ) const;

    bool getVertexNames( stringSet_t& rNames ) const;

    bool getAdjacencyMatrix( vertexId_t*& rMatrix, size_t& rNumElems ) const;

    bool createAdjacencyBitMatrix( byte_t*& rMatrix, size_t& rNumElems, size_t pNumVertices ) const;

    bool computeAdjacencyBitMatrix( byte_t*& rMatrix, size_t& rNumElems ) const;

    bool getDegree( const vertexId_t& rId, size_t& rDegree ) const
    {
        bool lRet = false;
        static idSet_t sIds;
        rDegree = 0;

        if( getAdjacencyList( rId, sIds ) )
        {
            rDegree = sIds.size();
            lRet = true;
        }

        return lRet;
    }

    // Returns the non adjacency matrix as a single dimension array.
    // the size of the array is num_vertices * ( num_vertices + 1 )
    // The first element of each row contains the number of elements
    // that are non adjacent
    bool getNonAdjacencyMatrix( vertexId_t*& rMatrix, size_t& rNumElems ) const;

    bool getNonAdjacencyListArray( NonAdjacencyNode_t*& rNonAdjacencyListArray ) const;

    void releaseMatrix( vertexId_t*& rMatrix ) const;

    size_t size() const
    {
        return mVertexName2IdMap.size();
    }

    void print( std::ostream& rOutStream ) const;
    void printAdjacencyList( std::ostream& rOutStream ) const;
    void printMatrix( std::ostream& rOutStream, const vertexId_t* pMatrix, size_t pMatrixSize ) const;
    void printBitMatrix( std::ostream& rOutStream, const byte_t* pBitMatrix, size_t pMatrixSize ) const;
    
private:
    idSetStore_t mAdjacencyLists;
    string2IdMap_t mVertexName2IdMap;
    id2StringMap_t mVertexId2NameMap;
    idPairStore_t mEdges;
};

#endif
