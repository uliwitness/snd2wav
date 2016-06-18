//
//  snd2wav.h
//  snd2wav
//
//  Created by Uli Kusterer on 18/06/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#ifndef snd2wav_h
#define snd2wav_h

#include <fstream>
#include <string>

class snd2wav
{
public:
	snd2wav( const std::string& inSndFile, const std::string& inWavFile );

	int	convert();

protected:
	uint8_t		ReadUint8();
	int16_t		ReadSint16();
	uint16_t	ReadUint16();
	int32_t		ReadSint32();
	uint32_t	ReadUint32();
	
	double		ReadExtended80();

	double		ReadFixed();
	double		ReadUFixed();
	
	void		print( const char* str );
	
	void		packV( uint32_t inNum );
	void		packv( uint16_t inNum );
	void		print( uint8_t inNum );
	
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
	
protected:
	std::ifstream	sndFile;
	std::ofstream	wavFile;
	std::string		wavPath;
};

#endif /* snd2wav_h */
