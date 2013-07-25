#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <functional>

#include "graphLoader.h"
#include "graph.h"

#define MAX_LINE_SIZE 256

bool GraphLoader::loadInput( const char* pFilename, Graph& rGraph )
{
    bool lRet = false;

    std::ifstream lInput;
    lInput.open( pFilename );
    if( lInput.is_open() )
    {
        char lLine[MAX_LINE_SIZE] = { 0 };

        while( lInput.getline( lLine, MAX_LINE_SIZE ) )
        {
            // remove newline at end
            std::string lStr = lLine;
            lStr.erase( std::remove_if( lStr.begin(), 
                                        lStr.end(), 
                                        std::bind2nd( std::equal_to<char>(), '\n' ) ),
                        lStr.end() );

            lStr.erase( std::remove_if( lStr.begin(), 
                                        lStr.end(), 
                                        std::bind2nd( std::equal_to<char>(), ' ' ) ),
                        lStr.end() );

            if( lStr.empty() )
            {
                continue;
            }

            if( Graph::COMMENT_CHAR == lStr[0] )
            {
                continue;
            }                
            
            std::string lVertex1;
            std::string lVertex2;
            Graph::vertexId_t lId1;
            Graph::vertexId_t lId2;

            std::string::size_type lCommaPos = lStr.find( ',' );

            if( lCommaPos != std::string::npos )
            {
                lVertex1 = std::string( lStr, 0, lCommaPos );
                lVertex2 = std::string( lStr, lCommaPos + 1 );
            }
            else
            {
                lVertex1 = lStr;
            }

            if( !lVertex1.empty() )
            {
                rGraph.addVertex( lVertex1, lId1 );
                lRet = true;
            }

            if( !lVertex2.empty() )
            {
                rGraph.addVertex( lVertex2, lId2 );
                lRet = true;
            }

            if( !lVertex1.empty() && !lVertex2.empty() )
            {
                rGraph.addEdge( lId1, lId2 );
            }
        }

        lInput.close();
    }

    return lRet;
}
