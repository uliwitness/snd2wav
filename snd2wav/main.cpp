//
//  main.cpp
//  snd2wav
//
//  Created by Uli Kusterer on 18/06/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#include "snd2wav.h"
#include <iostream>


using namespace std;


int main( int argc, const char * argv[] )
{
	if( argc > 3 || argc < 2 )
	{
		cerr << "Syntax: " << argv[0] << " <sndFile> [<wavFile>]\n";
		return 100;
	}
	
	std::string	dstPath;
	if( argc > 2 )
		dstPath = argv[2];
	else
	{
		dstPath = argv[1];
		dstPath.append(".wav");
	}
	
	snd2wav		converter( argv[1], dstPath );

	return converter.convert();
}
