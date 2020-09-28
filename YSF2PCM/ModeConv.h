/*
 *   Copyright (C) 2010,2014,2016 by Jonathan Naylor G4KLX
 *   Copyright (C) 2018 by Andy Uribe CA6JAU
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "Defines.h"
#include "YSFDefines.h"
#include "RingBuffer.h"

#if !defined(MODECONV_H)
#define MODECONV_H

class CModeConv {
public:
	CModeConv();
	~CModeConv();

	void putYSF(unsigned char* bytes);
	unsigned int getYSF(unsigned char* bytes);
	
	void putPCMHeader(int16_t *pcm);
	void putPCM(int16_t* pcm);
	void putPCMEOT(int16_t *pcm);
	bool getPCM(int16_t* pcm);

private:
	void encodeYSF(int16_t *pcm, uint8_t *vch);
	unsigned int m_ysfN;
	unsigned int m_pcmN;
	CRingBuffer<unsigned char> m_YSF;
	CRingBuffer<int16_t> m_PCM;
};

#endif
