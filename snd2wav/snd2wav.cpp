//
//  main.cpp
//  snd2wav
//
//  Created by Uli Kusterer on 15/05/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

/*
	Useful docs:
		http://soundfile.sapp.org/doc/WaveFormat/
		https://developer.apple.com/legacy/library/documentation/mac/pdf/Sound/Sound_Manager.pdf
		http://mirror.informatimago.com/next/developer.apple.com/documentation/mac/Sound/Sound-75.html#HEADING75-0
		http://mirror.informatimago.com/next/developer.apple.com/documentation/mac/Sound/Sound-68.html
		http://mirror.informatimago.com/next/developer.apple.com/documentation/mac/Sound/Sound-76.html#MARKER-9-702
		http://mirror.informatimago.com/next/developer.apple.com/documentation/mac/Sound/Sound-66.html#HEADING66-0
		http://mirror.informatimago.com/next/developer.apple.com/documentation/mac/Sound/Sound-60.html#MARKER-9-400
		http://www.instructables.com/id/How-to-Read-aiff-Files-using-C/?ALLSTEPS
*/

#include <iostream>
#include <fstream>
#include <math.h>


#define FLIP_16(value) \
        (((((u_int16_t)(value))<<8) & 0xFF00)   | \
         ((((u_int16_t)(value))>>8) & 0x00FF))

#define FLIP_32(value) \
        (((((u_int32_t)(value))<<24) & 0xFF000000)  | \
         ((((u_int32_t)(value))<< 8) & 0x00FF0000)  | \
         ((((u_int32_t)(value))>> 8) & 0x0000FF00)  | \
         ((((u_int32_t)(value))>>24) & 0x000000FF))

#define FLIP_64(value) \
		(((((u_int64_t)(value))<<56) & 0xFF00000000000000)  | \
         ((((u_int64_t)(value))<<40) & 0x00FF000000000000)  | \
         ((((u_int64_t)(value))<<24) & 0x0000FF0000000000)  | \
         ((((u_int64_t)(value))<< 8) & 0x000000FF00000000)  | \
         ((((u_int64_t)(value))>> 8) & 0x00000000FF000000)  | \
         ((((u_int64_t)(value))>>24) & 0x0000000000FF0000)  | \
         ((((u_int64_t)(value))>>40) & 0x000000000000FF00)  | \
         ((((u_int64_t)(value))>>56) & 0x00000000000000FF))


using namespace std;

class snd2wav
{
public:
	snd2wav( const string& inSndFile, const string& inWavFile )
	{
		sndFile.open( inSndFile.c_str() );
		wavPath = inWavFile;
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
	
	double	ReadExtended80()
	{
		double		num = 0;
		char		f[10] = {};
		sndFile.read( f, 10 );
		
		bool	sign = false;
		if( f[0] & (1 << 7) )
			sign = true;
		
		int		exponent = 0;
		f[0] &= ~(1<<7);	// Take rest of number without sign bit.
		exponent = (f[0] << 8) | (0x00ff & f[1]);
		exponent -= 16383;	// subtract bias.
		
		for( int i = 2; i < 10; i++ )
		{
			for( int j = 7; j >= 0; j-- )
			{
				if( (exponent >= 0) && (f[i] & (1 << j)) )
					num += pow( 2, exponent );
				else if( (exponent < 0) && (f[i] & (1 << j)) )
					num += 1.0 / (pow( 2, abs(exponent) ));
				exponent--;
			}
		}
		
		if( sign )
			num = -sign;
		
		return num;
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
		initChanLeft		= 0x0002,		// left stereo channel
		initChanRight		= 0x0003,		// right stereo channel
		waveInitChannel0	= 0x0004,		// wave-table channel 0
		waveInitChannel1	= 0x0005,		// wave-table channel 1
		waveInitChanne12	= 0x0006,		// wave-table channel 2
		waveInitChannel3	= 0x0007,		// wave-table channel 3
		initMono			= 0x0080,		// monophonic channel
		initStereo			= 0x00C0,		// stereo channel
		initMACE3			= 0x0300,		// 3:1 compression
		initMACE6			= 0x0400,		// 6:1 compression
		initNoInterp		= 0x0004,		// no linear interpolation
		initNoDrop			= 0x0008		// no drop-sample conversion
	};
	
	enum {
		stdSH = 0x00,	// standard sound header
		extSH = 0xff,	// extended sound header
		cmpSH = 0xfe	// compressed sound header
	};
	
	int	convert()
	{
		uint16_t	channels = 1;
		int16_t		sndFormat = ReadSint16();
		if( sndFormat != 1 && sndFormat != 2 )
		{
			cerr << "bad snd format " << sndFormat << "\n";
			return 1;
		}
		uint32_t	sndHeaderOffset = 20;
		int32_t		opts = 0;
		if( sndFormat == 2 )
		{
			ReadSint16();	// "reference count" for app use.
			sndHeaderOffset = 14;
		}
		else
		{
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
			if( opts != initMono && opts != 0xa0 && opts != initStereo )
			{
				cerr << "unhandled opts " << opts << "\n";
				return 4;
			}
		}
		if( ReadSint16() != 1 )
		{
			cerr << "too many commands\n";
			return 5;
		}
		uint16_t	sndCommand = ReadUint16();
		if( sndCommand != 0x8051 && sndCommand != 0x8050 )
		{
			cerr << "not a bufferCmd or sndCmd" << sndCommand << "\n";
			return 6;
		}
		if( ReadSint16() != 0 )
		{
			cerr << "bad param1\n";
			return 7;
		}
		if( ReadSint32() != sndHeaderOffset )
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

		uint8_t		sampleEncoding = ReadUint8();
		if( sampleEncoding != 0 && sampleEncoding != extSH )
		{
			cerr << "not standard sample encoding " << (int)sampleEncoding << "\n";
			return 10;
		}
		if( sampleEncoding == extSH )
		{
			channels = numbytes;
			numbytes = 0;
		}
		else if( opts == initStereo )
			channels = 2;
		uint8_t		baseFrequency = ReadUint8();
		float		factor = 1.0;
		if( baseFrequency != 60 )
		{
			factor = (baseFrequency / 127.0);
			factor = 1.0 +(factor -0.5);
			factor = 1 / factor;
			cerr << "weird baseFrequency " << (int) baseFrequency << "expected 60 out of 0...127. WAV will play " << factor << "x.\n";
			factor = 0.8;
		}
		uint32_t	data_size = numbytes;
		uint32_t	rate = samplerate;
		uint32_t	bytes_sample = 1;
		if( sampleEncoding == extSH )
		{
			uint32_t		numFrames = ReadUint32();
			double	aiffSampleRate = ReadExtended80();
			/*uint32_t markerChunk =*/ ReadUint32();
			/*uint32_t instrumentChunks =*/ ReadUint32();
			/*uint32_t aesRecording =*/ ReadUint32();
			uint16_t sampleSize = ReadUint16();
			bytes_sample = sampleSize / 8;
			/*uint16_t futureUse1 =*/ ReadUint16();
			/*uint32_t futureUse2 =*/ ReadUint32();
			/*uint32_t futureUse3 =*/ ReadUint32();
			/*uint32_t futureUse4 =*/ ReadUint32();
			data_size = numFrames * bytes_sample * channels;
			numbytes = data_size;
			rate = aiffSampleRate;
		}
		
		// we're up to sound data now, let's write some WAV!
		// Thanks to: https://ccrma.stanford.edu/courses/422/projects/WaveFormat/
		// Thanks also to Jeremiah Morris in the past, who wrote the following for
		//  a different project, which he copied for this.
		
		wavFile.open( wavPath.c_str() );
		
		cout << "size: " << data_size << "\nrate: " << rate << "\n channels: " << channels << "\n";
		
		print( "RIFF" );			// ChunkID
		packV( 36 + data_size );	// ChunkSize
		print( "WAVE" );			// Format
		
		print("fmt ");				// Subchunk1ID
		packV(16);					// Subchunk1Size
		packv(1);					// AudioFormat
		packv(channels);			// NumChannels
		packV(rate * factor);		// SampleRate
		packV(rate * channels * bytes_sample);	// ByteRate
		packv( channels * bytes_sample );		// BlockAlign
		packv( bytes_sample * 8 );				// BitsPerSample
		
		print("data");				// Subchunk2ID
		packV( data_size );			// Subchunk2Size
		if( bytes_sample == 2 )
		{
			for( uint32_t i = 0; i < (numbytes /2); i++ )
			{
				uint16_t	theByte = 0;
				sndFile.read( (char*) &theByte, sizeof(theByte) );
				theByte = FLIP_16(theByte);
				wavFile.write( (char*) &theByte, sizeof(theByte) );
			}
		}
		else
		{
			for( uint32_t i = 0; i < numbytes; i++ )
				print( ReadUint8() );
		}
		
		return 0;
	}

protected:
	ifstream	sndFile;
	ofstream	wavFile;
	string		wavPath;
};



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
