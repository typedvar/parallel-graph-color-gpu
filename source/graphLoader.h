#ifndef _GRAPHLOADER_H_
#define _GRAPHLOADER_H_

class Graph;

class GraphLoader
{
public:
    bool loadInput( const char* pFilename, Graph& rGraph );
};

#endif
