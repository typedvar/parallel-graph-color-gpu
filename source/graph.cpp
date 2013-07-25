#include <string>
#include <algorithm>
#include <ostream>
#include <cmath>

#ifdef _DEBUG
#include <iostream>
#endif

#include "graph.h"
#include "defines.h"
#include "nonAdjacencyNode.h"

struct VertexIdGenerator
{
    VertexIdGenerator() : mCurrId( 0 )
    {}

    Graph::vertexId_t operator() ()
    {
        return mCurrId++;
    }
    
    Graph::vertexId_t mCurrId;
};

struct GraphPrinter
{
    GraphPrinter( const Graph& pGraph, std::ostream& pOutStream ) 
                : mGraph( pGraph ), mOutStream( pOutStream )
    {}

    void operator() ( const std::string& pNodeName )
    {
        mOutStream << pNodeName << " " << std::endl;

        Graph::vertexId_t lId = 0;
        if( mGraph.getId( pNodeName, lId ) )
        {
            Graph::idSet_t lList;
            if( mGraph.getAdjacencyList( lId, lList ) )
            {
                std::copy( lList.begin(), 
                           lList.end(),  
                           std::ostream_iterator<Graph::vertexId_t>( mOutStream, " | " ) );
                mOutStream << std::endl;
            }
        }
    }
    const Graph& mGraph;
    std::ostream& mOutStream;
};

struct CreateAdjacencyMatrix
{
    CreateAdjacencyMatrix( Graph::vertexId_t* pVertexMatrix, const Graph& pGraph )
        : mMatrix( pVertexMatrix )
        , mGraph( pGraph )
        , mCurrIdx( 0 )
        , mIsBitMatrix( false )
    {
        mNumVertices = mGraph.size();
    }

    // BitMatrix overloaded ctor
    CreateAdjacencyMatrix( Graph::byte_t* pBitMatrix, const Graph& pGraph )
        : mBitMatrix( pBitMatrix )
        , mGraph( pGraph )
        , mCurrIdx( 0 )
        , mIsBitMatrix( true )
    {
        mNumVertices = mGraph.size();
    }

    void operator() ( const Graph::idSet_t& pList )
    {
        size_t lOffset = mCurrIdx * mNumVertices;
        for( Graph::idSetConstIter_t lIter = pList.begin();
            lIter != pList.end();
            ++lIter )
        {
            Graph::vertexId_t lVertexId = *lIter;
            size_t lIdx = lOffset + lVertexId;
            if( mIsBitMatrix )
            {
                Graph::setBit( mBitMatrix, lIdx, true );
            }
            else
            {
                mMatrix[lIdx] = 1;
            }
        }
        ++mCurrIdx;
    }

private:
    const Graph& mGraph;
    size_t mNumVertices;
    
    Graph::vertexId_t* mMatrix;
    Graph::byte_t* mBitMatrix;

    size_t mCurrIdx;
    bool mIsBitMatrix;
};

struct LesserDegree
{
    LesserDegree( const Graph& rGraph ) 
                : mGraph( rGraph ) 
                , mDegrees( rGraph.size() )
    {
        loadDegrees();
    }

    void loadDegrees()
    {
        for( Graph::vertexId_t v = 0; v < mGraph.size(); ++v )
        {
            mGraph.getDegree( v, mDegrees[v] );
        }
    }

    bool operator() ( Graph::vertexId_t& rVertex1, Graph::vertexId_t& rVertex2 )
    {
        return ( mDegrees[rVertex1] < mDegrees[rVertex2] );
    }

private:
    const Graph& mGraph;
    std::vector<size_t> mDegrees;
};

struct CreateNonAdjacencyListArray
{
    CreateNonAdjacencyListArray( const Graph::idVec_t& pVertices, 
                                 NonAdjacencyNode_t* pAdjListArray, 
                                 const Graph& pGraph )
        : mVertices( pVertices ) 
        , mAdjListArray( pAdjListArray )
        , mGraph( pGraph )
        , mCurrIdx( 0 )
        , mLesserDegreeSorter( pGraph )
    {}

    void operator() ( const Graph::idSet_t& pList )
    {
        // compute the non adjacent nodes
        Graph::idVec_t lDifference( mGraph.size() );
        Graph::idVec_t::iterator lResult = std::set_difference( mVertices.begin(), 
                                                                mVertices.end(), 
                                                                pList.begin(), 
                                                                pList.end(), 
                                                                lDifference.begin() );

        size_t lNumElems = lResult - lDifference.begin();

        if( lNumElems )
        {
            std::sort( lDifference.begin(), lResult, mLesserDegreeSorter );

            NonAdjacencyNode_t* lNode = &mAdjListArray[mCurrIdx];
            lNode->mNumElems = lNumElems;
            lNode->mNodes = new Graph::vertexId_t[lNumElems];
            std::copy( lDifference.begin(), lResult, lNode->mNodes );
        }
        ++mCurrIdx;
    }

private:
    const Graph& mGraph;
    size_t mNumVertices;
    NonAdjacencyNode_t* mAdjListArray;
    size_t mCurrIdx;
    const Graph::idVec_t& mVertices;
    LesserDegree mLesserDegreeSorter;
};

struct CreateNonAdjacencyMatrix
{
    CreateNonAdjacencyMatrix( Graph::vertexId_t* pVertexMatrix, const Graph& pGraph )
        : mMatrix( pVertexMatrix ), mGraph( pGraph ), mCurrIdx( 0 )
    {
        mNumVertices = mGraph.size();
        mVertices.reserve( mNumVertices );

        for( Graph::vertexId_t v = 0; v < mNumVertices; ++v )
        {
            mVertices.push_back( v );
        }
    }

    void operator() ( const Graph::idSet_t& pList )
    {
        size_t lStart = mCurrIdx * ( mNumVertices + 1 );
        size_t lEnd = lStart + mNumVertices + 1;

        // compute the non adjacent nodes
        Graph::idVec_t lDifference( mNumVertices );
        Graph::idVec_t::iterator lResult = std::set_difference( mVertices.begin(), 
                                                                mVertices.end(), 
                                                                pList.begin(), 
                                                                pList.end(), 
                                                                lDifference.begin() );

        size_t lNumElems = lResult - lDifference.begin();

        if( lNumElems )
        {
            std::sort( lDifference.begin(), lResult, LesserDegree( mGraph ) );
            std::fill( mMatrix + lStart, mMatrix + lEnd, 0 );
            mMatrix[lStart] = lNumElems;
            std::copy( lDifference.begin(), lResult, mMatrix + lStart + 1 );
        }
        ++mCurrIdx;
    }

private:
    const Graph& mGraph;
    size_t mNumVertices;
    Graph::vertexId_t* mMatrix;
    size_t mCurrIdx;
    Graph::idVec_t mVertices;
};

struct CopyKey
{
    CopyKey( Graph::stringSet_t& rDest ) : mDest( rDest ) {}

    void operator () ( const Graph::string2IdPair_t& rSource )
    {
        mDest.insert( rSource.first );
    }

    Graph::stringSet_t& mDest;
};

const char Graph::COMMENT_CHAR = '#';
const int Graph::MAX_LINE_SIZE = 256;
const int Graph::BYTE_SIZE = 8;

Graph::Graph()
{
}

Graph::Graph( size_t pNumVertices )
{
}

bool Graph::getVertexNames( Graph::stringSet_t& rNames ) const
{
    bool lRet = false;

    if( !empty() )
    {
        std::for_each( mVertexName2IdMap.begin(),
                       mVertexName2IdMap.end(),
                       CopyKey( rNames ) );
        lRet = true;
    }

    return lRet;
}

bool Graph::getId( const std::string& pVertexName, Graph::vertexId_t& rVertexId ) const
{
    bool lRet = false;
    rVertexId = 0;

    if( !mVertexName2IdMap.empty() )
    {
        string2IdMapConstIter_t lIter = mVertexName2IdMap.find( pVertexName );
        if( lIter != mVertexName2IdMap.end() )
        {
            rVertexId = lIter->second;
            lRet = true;
        }
    }
    return lRet;
}

bool Graph::getName( const Graph::vertexId_t& rId, std::string& rVertexName ) const
{
    bool lRet = false;

    if( isValidId( rId ) && !mVertexId2NameMap.empty() )
    {
        id2StringConstIter_t lIter = mVertexId2NameMap.find( rId );

        if( lIter != mVertexId2NameMap.end() )
        {
            rVertexName = lIter->second;
            lRet = true;
        }
    }
    return lRet;
}


bool Graph::getAdjacencyList( const Graph::vertexId_t& pIdx, Graph::idSet_t& rAdjList ) const
{
    bool lRet = false;

    if( !mAdjacencyLists.empty() && isValidId( pIdx ) )
    {
        rAdjList = mAdjacencyLists[pIdx];
        lRet = true;
    }
    return lRet;
}

bool Graph::addVertex( const std::string& pVertexName, vertexId_t& rVertexId )
{
    bool lRet = false;

    rVertexId = 0;

    if( !getId( pVertexName, rVertexId ) )
    {
        rVertexId = mVertexName2IdMap.size();
        
        mVertexName2IdMap.insert( string2IdPair_t( pVertexName, rVertexId ) );
        mVertexId2NameMap.insert( id2StringPair_t( rVertexId, pVertexName ) );

        idSet_t lAdjSet;
        mAdjacencyLists.push_back( lAdjSet );
        lRet = true;
    }
    
    return lRet;
}

bool Graph::addEdge( Graph::vertexId_t& rFirst, Graph::vertexId_t& rSecond )
{
    bool lRet = false;

    if( isValidId( rFirst ) && isValidId( rSecond ) )
    {
        idSet_t& rFirstSet = mAdjacencyLists[rFirst];
        idSet_t& rSecondSet = mAdjacencyLists[rSecond];

        size_t lInitSize = rFirstSet.size();
        bool lIsAdded = false;

        rFirstSet.insert( rSecond );
        lIsAdded = ( ( rFirstSet.size() - lInitSize ) > 0 );

        rSecondSet.insert( rFirst );

        if( lIsAdded )
        {
            mEdges.push_back( idPair_t( rFirst, rSecond ) );
        }

        lRet = true;
    }
    return lRet;
}

bool Graph::getAdjacencyMatrix( Graph::vertexId_t*& rMatrix, size_t& rNumElems ) const
{
    bool lRet = false;

    if( !mVertexName2IdMap.empty() )
    {
        size_t lNumVertices = mVertexName2IdMap.size();
        rNumElems = lNumVertices * lNumVertices;
        rMatrix = new vertexId_t[rNumElems];
        
        std::fill( rMatrix, rMatrix + rNumElems, 0 );

        std::for_each( mAdjacencyLists.begin(),
                       mAdjacencyLists.end(),
                       CreateAdjacencyMatrix( rMatrix, *this ) );
        lRet = true;
    }
    return lRet;
}

bool Graph::createAdjacencyBitMatrix( byte_t*& rMatrix, size_t& rNumElems, size_t pNumVertices ) const
{
    bool lRet = false;
    rNumElems = pNumVertices * pNumVertices;
    size_t lNumBytes = std::ceil( ( ( double )rNumElems ) / Graph::BYTE_SIZE );
    rMatrix = new byte_t[lNumBytes];
    std::fill( rMatrix, rMatrix + lNumBytes, 0 );

    if( rMatrix )
    {
        lRet = true;
    }
    return lRet;
}

bool Graph::computeAdjacencyBitMatrix( Graph::byte_t*& rMatrix, size_t& rNumElems ) const
{
    bool lRet = false;

    if( !mVertexName2IdMap.empty() && createAdjacencyBitMatrix( rMatrix, rNumElems, size() ) )
    {
        std::for_each( mAdjacencyLists.begin(),
                       mAdjacencyLists.end(),
                       CreateAdjacencyMatrix( rMatrix, *this ) );
        lRet = true;
    }
    return lRet;
}

bool Graph::getNonAdjacencyMatrix( vertexId_t*& rMatrix, size_t& rNumElems ) const
{
    bool lRet = false;
    if( !mVertexName2IdMap.empty() )
    {
        size_t lNumVertices = mVertexName2IdMap.size();
        rNumElems = lNumVertices * ( lNumVertices + 1 );
        rMatrix = new vertexId_t[rNumElems];
        
        std::fill( rMatrix, rMatrix + rNumElems, 0 );

        std::for_each( mAdjacencyLists.begin(),
                       mAdjacencyLists.end(),
                       CreateNonAdjacencyMatrix( rMatrix, *this ) );
        lRet = true;
    }
    return lRet;
}

bool Graph::getNonAdjacencyListArray( NonAdjacencyNode_t*& rNonAdjacencyListArray ) const
{
    bool lRet = false;
    if( !mVertexName2IdMap.empty() )
    {
        size_t lNumVertices = mVertexName2IdMap.size();
        
        rNonAdjacencyListArray = new NonAdjacencyNode_t[lNumVertices];
        
        Graph::idVec_t cVertices( lNumVertices );

        std::generate( cVertices.begin(), cVertices.end(), VertexIdGenerator() );

        std::for_each( mAdjacencyLists.begin(),
                       mAdjacencyLists.end(),
                       CreateNonAdjacencyListArray( cVertices, 
                                                    rNonAdjacencyListArray,
                                                    *this ) );
        lRet = true;
    }
    return lRet;
}

void Graph::releaseMatrix( Graph::vertexId_t*& rMatrix ) const
{
    if( rMatrix )
    {
        delete [] rMatrix;
        rMatrix = NULL;
    }
}

void Graph::printAdjacencyList( std::ostream& rOutStream ) const
{
    Graph::stringSet_t lNames;
    if( getVertexNames( lNames ) )
    {
        std::for_each( lNames.begin(), lNames.end(), GraphPrinter( *this, rOutStream ) );
    }
}

void Graph::print( std::ostream& rOutStream ) const
{
    for( idPairStoreConstIter_t lIter = mEdges.begin();
         lIter != mEdges.end();
         ++lIter )
    {
        rOutStream << lIter->first << ", " << lIter->second << std::endl;
    }
}

bool Graph::getBit( const Graph::byte_t* pBitMatrix, size_t pBitOffset )
{
    size_t lByteNum = pBitOffset / Graph::BYTE_SIZE;
    int lBitPos = pBitOffset % Graph::BYTE_SIZE;
    const Graph::byte_t& lVertexByte = pBitMatrix[lByteNum];
    return ( 0 != ( lVertexByte & ( 0x1 << ( Graph::BYTE_SIZE - ( lBitPos + 1 ) ) ) ) );
}

void Graph::setBit( Graph::byte_t* pBitMatrix, size_t pBitOffset, bool pVal )
{
    size_t lByteNum = pBitOffset / Graph::BYTE_SIZE;
    int lBitPos = pBitOffset % Graph::BYTE_SIZE;
    Graph::byte_t& lVertexByte = pBitMatrix[lByteNum];

    if( pVal )
    {
        lVertexByte |= ( 0x1 << ( Graph::BYTE_SIZE - ( lBitPos + 1 ) ) );
    }
    else
    {
        lVertexByte &= ~( 0x1 << ( Graph::BYTE_SIZE - ( lBitPos + 1 ) ) );
    }
}

void Graph::printBitMatrix( std::ostream& rOutStream, const byte_t* pBitMatrix, size_t pMatrixSize ) const
{
    Graph::stringSet_t lNames;
    if( getVertexNames( lNames ) )
    {
        rOutStream << "  ";
        std::copy( lNames.begin(), lNames.end(), std::ostream_iterator<std::string>( rOutStream, " " ) );
        rOutStream << std::endl;
 
        size_t lNumVertices = size();
        for( Graph::stringSetConstIter_t lIter = lNames.begin();
            lIter != lNames.end();
            ++lIter )
        {
            Graph::vertexId_t i;
            std::string lVertName = *lIter;
            if( getId( lVertName, i ) )
            {
                rOutStream << lVertName << " ";

                for( Graph::stringSetConstIter_t lInnerIter = lNames.begin();
                    lInnerIter != lNames.end();
                    ++lInnerIter )
                {
                    std::string lInnerVert = *lInnerIter;
                    Graph::vertexId_t j;

                    if( getId( lInnerVert, j ) )
                    {
                        int lVal = Graph::getBit( pBitMatrix, ( i * lNumVertices + j ) ) ? 1 : 0;
                        rOutStream << lVal << " ";
                    }
                }

                rOutStream << std::endl;
            }
        }
    }
}

void Graph::printMatrix( std::ostream& rOutStream, const vertexId_t* pMatrix, size_t pMatrixSize ) const
{
    Graph::stringSet_t lNames;
    if( getVertexNames( lNames ) )
    {
        rOutStream << "  ";
        std::copy( lNames.begin(), lNames.end(), std::ostream_iterator<std::string>( rOutStream, " " ) );
        rOutStream << std::endl;
 
        size_t lNumVertices = size();
        for( Graph::stringSetConstIter_t lIter = lNames.begin();
            lIter != lNames.end();
            ++lIter )
        {
            Graph::vertexId_t i;
            std::string lVertName = *lIter;
            if( getId( lVertName, i ) )
            {
                rOutStream << lVertName << " ";

                for( Graph::stringSetConstIter_t lInnerIter = lNames.begin();
                    lInnerIter != lNames.end();
                    ++lInnerIter )
                {
                    std::string lInnerVert = *lInnerIter;
                    Graph::vertexId_t j;

                    if( getId( lInnerVert, j ) )
                    {
                        rOutStream << pMatrix[i * lNumVertices + j] << " ";
                    }
                }

                rOutStream << std::endl;
            }
        }
    }
}

void Graph::marshallAdjacencyListArray( const NonAdjacencyNode_t* pNonAdjListArray, 
                                        size_t pNumElems, 
                                        vertexId_t*& rNonAdjStream,
                                        size_t& rNonAdjStreamSize,
                                        vertexId_t*& rNonAdjStreamIdxArray )
{
    size_t lMarshallSize = 0;

    rNonAdjStreamIdxArray = new vertexId_t[pNumElems];

    for( size_t lIdx = 0; lIdx < pNumElems; ++lIdx )
    {
        lMarshallSize += pNonAdjListArray[lIdx].mNumElems;

        if( 0 == lIdx )
        {
            rNonAdjStreamIdxArray[lIdx] = 0;
        }
        else
        {
            rNonAdjStreamIdxArray[lIdx] = rNonAdjStreamIdxArray[lIdx - 1] + pNonAdjListArray[lIdx - 1].mNumElems;
        }
    }

    rNonAdjStream = new vertexId_t[lMarshallSize];
    rNonAdjStreamSize = lMarshallSize;

    for( size_t lIdx = 0; lIdx < pNumElems; ++lIdx )
    {
        memcpy( rNonAdjStream + rNonAdjStreamIdxArray[lIdx], pNonAdjListArray[lIdx].mNodes, sizeof( vertexId_t ) * pNonAdjListArray[lIdx].mNumElems );
    }
}

// end of file
