//
//  main.cpp
//  snd2wav
//
//  Created by Uli Kusterer on 15/05/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#include <iostream>
#include <fstream>


#define FLIP_16(value) \
        (((((u_int16_t)(value))<<8) & 0xFF00)   | \
         ((((u_int16_t)(value))>>8) & 0x00FF))

#define FLIP_32(value) \
        (((((u_int32_t)(value))<<24) & 0xFF000000)  | \
         ((((u_int32_t)(value))<< 8) & 0x00FF0000)  | \
         ((((u_int32_t)(value))>> 8) & 0x0000FF00)  | \
         ((((u_int32_t)(value))>>24) & 0x000000FF))



using namespace std;

class snd2wav
{
public:
	snd2wav( const string& inSndFile, const string& inWavFile )
	{
		sndFile.open( inSndFile.c_str() );
		wavFile.open( inWavFile.c_str() );
	}

	uint8_t	ReadUint8()
	{
		uint8_t		num = 0;
		sndFile.read( (char*) &num, sizeof(num) );
		
		return num;
	}

	int16_t	ReadSint16()
	{
		int16_t		num = 0;
		sndFile.read( (char*) &num, sizeof(num) );
		
		return FLIP_16(num);
	}

	uint16_t	ReadUint16()
	{
		uint16_t		num = 0;
		sndFile.read( (char*) &num, sizeof(num) );
		
		return FLIP_16(num);
	}

	int32_t	ReadSint32()
	{
		int32_t		num = 0;
		sndFile.read( (char*) &num, sizeof(num) );
		
		return FLIP_32(num);
	}
	
	uint32_t	ReadUint32()
	{
		int32_t		num = 0;
		sndFile.read( (char*) &num, sizeof(num) );
		
		return FLIP_32(num);
	}

	double	ReadFixed()
	{
		int32_t	num = 0;
		sndFile.read( (char*) &num, sizeof(num) );
		
		return double(FLIP_32(num)) / 65536.0;
	}

	double	ReadUFixed()
	{
		uint32_t	num = 0;
		sndFile.read( (char*) &num, sizeof(num) );
		
		return double(FLIP_32(num)) / 65536.0;
	}
	
	void	print( const char* str )
	{
		wavFile.write( str, strlen(str) );
	}
	
	void	packV( uint32_t inNum )
	{
		wavFile.write( (char*) &inNum, sizeof(inNum) );
	}

	void	packv( uint16_t inNum )
	{
		wavFile.write( (char*) &inNum, sizeof(inNum) );
	}
	
	void	print( uint8_t inNum )
	{
		wavFile.write( (char*) &inNum, sizeof(inNum) );
	}
	
	// Flags for the opts field:
	enum {
		initChanLeft		= 0x0002,		// left stereo channel}
		initChanRight		= 0x0003,		// right stereo channel}
		waveInitChannel0	= 0x0004,		// wave-table channel 0}
		waveInitChannel1	= 0x0005,		// wave-table channel 1}
		waveInitChanne12	= 0x0006,		// wave-table channel 2}
		waveInitChannel3	= 0x0007,		// wave-table channel 3}
		initMono			= 0x0080,		// monophonic channel
		initStereo			= 0x00C0,		// stereo channel
		initMACE3			= 0x0300,		// 3:1 compression
		initMACE6			= 0x0400,		// 6:1 compression
		initNoInterp		= 0x0004,		// no linear interpolation
		initNoDrop			= 0x0008		// no drop-sample conversion
	};
	
	int	convert()
	{
		if( ReadSint16() != 1 )
		{
			cerr << "bad snd format\n";
			return 1;
		}
		if( ReadSint16() != 1 )
		{
			cerr << "too many data types\n";
			return 2;
		}
		if( ReadSint16() != 5 )
		{
			cerr << "not sampled sound\n";
			return 3;
		}
		int32_t	opts = ReadSint32();
		if( opts != 0x80 && opts != 0xa0 )
		{
			cerr << "unhandled opts " << opts << "\n";
			return 4;
		}
		if( ReadSint16() != 1 )
		{
			cerr << "too many commands\n";
			return 5;
		}
		if( ReadUint16() != 0x8051 )
		{
			cerr << "not a bufferCmd\n";
			return 6;
		}
		if( ReadSint16() != 0 )
		{
			cerr << "bad param1\n";
			return 7;
		}
		if( ReadSint32() != 20 )
		{
			cerr << "bad param2\n";
			return 8;
		}
		if( ReadSint32() != 0 )
		{
			cerr << "bad data pointer\n";
			return 9;
		}
		uint32_t	numbytes = ReadUint32();
		double		samplerate = ReadUFixed();
		
		/*uint32_t	loopstart =*/ ReadUint32();
		/*uint32_t	samplerate =*/ ReadUint32();

		if( ReadUint8() != 0 )
		{
			cerr << "not standard sample encoding\n";
			return 10;
		}
		if( ReadUint8() != 0x3c )
		{
			cerr << "weird baseFrequency\n";
			return 11;
		}
		
		// we're up to sound data now, let's write some WAV!
		// Thanks to: https://ccrma.stanford.edu/courses/422/projects/WaveFormat/
		// Thanks also to Jeremiah Morris in the past, who wrote the following for
		//  a different project, which he copied for this.
		
		uint32_t	data_size = numbytes;
		uint32_t	rate = samplerate;
		uint32_t	bytes_sample = 1;
		uint16_t	channels = 1;
		
		print( "RIFF" );			// ChunkID
		packV( 36 + data_size );	// ChunkSize
		print( "WAVE" );			// Format
		
		print("fmt ");				// Subchunk1ID
		packV(16);					// Subchunk1Size
		packv(1);					// AudioFormat
		packv(channels);			// NumChannels
		packV(rate);				// SampleRate
		packV(rate * channels * bytes_sample);	// ByteRate
		packv( channels * bytes_sample );		// BlockAlign
		packv( bytes_sample * 8 );				// BitsPerSample
		
		print("data");				// Subchunk2ID
		packV( data_size );			// Subchunk2Size
		for( uint32_t i = 0; i < numbytes; i++ )
			print( ReadUint8() );
		
		return 0;
	}

protected:
	ifstream	sndFile;
	ofstream	wavFile;
};



int main( int argc, const char * argv[] )
{
	if( argc != 3 )
	{
		cerr << "Syntax: " << argv[0] << " <sndFile> <wavFile>\n";
		return 100;
	}
	
	snd2wav		converter( argv[1], argv[2] );

	return converter.convert();
}
