/*
 *   Copyright (C) 2010,2014,2016 and 2018 by Jonathan Naylor G4KLX
 *   Copyright (C) 2016 Mathias Weyland, HB9FRV
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

#include "ModeConv.h"
#include "Utils.h"
#include "Log.h"
#include <cstdio>
#include <cassert>

const uint8_t  BIT_MASK_TABLE8[]  = { 0x80U, 0x40U, 0x20U, 0x10U, 0x08U, 0x04U, 0x02U, 0x01U };
#define WRITE_BIT8(p,i,b)   p[(i)>>3] = (b) ? (p[(i)>>3] | BIT_MASK_TABLE8[(i)&7]) : (p[(i)>>3] & ~BIT_MASK_TABLE8[(i)&7])
#define READ_BIT8(p,i)     (p[(i)>>3] & BIT_MASK_TABLE8[(i)&7])

CModeConv::CModeConv() :
m_p25N(0U),
m_pcmN(0U),
m_P25(5000U, "PCM2P25"),
m_PCM(50000U, "P252PCM")
{
}

CModeConv::~CModeConv()
{
}

void CModeConv::putP25(unsigned char* data)
{
	assert(data != NULL);

	int16_t pcm[160U];
	uint8_t imbe[11U];
	::memset(pcm, 0, sizeof(pcm));
	
	switch (data[0U]) {
	case 0x62U:
		::memcpy(imbe, data + 10U, 11U);
		break;
	case 0x63U:
		::memcpy(imbe, data + 1U, 11U);
		break;
	case 0x64U:
		::memcpy(imbe, data + 5U, 11U);
		break;
	case 0x65U:
		::memcpy(imbe, data + 5U, 11U);
		break;
	case 0x66U:
		::memcpy(imbe, data + 5U, 11U);
		break;
	case 0x67U:
		::memcpy(imbe, data + 5U, 11U);
		break;
	case 0x68U:
		::memcpy(imbe, data + 5U, 11U);
		break;
	case 0x69U:
		::memcpy(imbe, data + 5U, 11U);
		break;
	case 0x6AU:
		::memcpy(imbe, data + 4U, 11U);
		break;
	case 0x6BU:
		::memcpy(imbe, data + 10U, 11U);
		break;
	case 0x6CU:
		::memcpy(imbe, data + 1U, 11U);
		break;
	case 0x6DU:
		::memcpy(imbe, data + 5U, 11U);
		break;
	case 0x6EU:
		::memcpy(imbe, data + 5U, 11U);
		break;
	case 0x6FU:
		::memcpy(imbe, data + 5U, 11U);
		break;
	case 0x70U:
		::memcpy(imbe, data + 5U, 11U);
		break;
	case 0x71U:
		::memcpy(imbe, data + 5U, 11U);
		break;
	case 0x72U:
		::memcpy(imbe, data + 5U, 11U);
		break;
	case 0x73U:
		::memcpy(imbe, data + 4U, 11U);
		break;
	default:
		break;
	}
	decode_4400(pcm, imbe);
	m_PCM.addData(pcm, 160U);
	m_pcmN += 1;
}

unsigned int CModeConv::getP25(unsigned char* data)
{
	unsigned char tag[1U];

	tag[0U] = TAG_NODATA;

	if (m_p25N >= 1U) {
		m_P25.peek(tag, 1U);

		if (tag[0U] != TAG_DATA) {
			m_P25.getData(tag, 1U);
			m_P25.getData(data, 11U);
			m_p25N -= 1U;
			return tag[0U];
		}
	}

	if (m_p25N >= 1U) {
		m_P25.getData(tag, 1U);
		m_P25.getData(data, 11U);
		m_p25N -= 1U;

		return TAG_DATA;
	}
	else
		return TAG_NODATA;
}

void CModeConv::putPCMHeader(int16_t *pcm)
{
	uint8_t imbe[11U];
	encode_4400(pcm, imbe);
	m_P25.addData(&TAG_HEADER, 1U);
	m_P25.addData(imbe, 11U);
	m_p25N += 1U;
}

void CModeConv::putPCM(int16_t *pcm)
{
	uint8_t imbe[11U];
	encode_4400(pcm, imbe);
	m_P25.addData(&TAG_DATA, 1U);
	m_P25.addData(imbe, 11U);
	m_p25N += 1U;
}

void CModeConv::putPCMEOT(int16_t *pcm)
{
	uint8_t imbe[11U];
	encode_4400(pcm, imbe);
	m_P25.addData(&TAG_EOT, 1U);
	m_P25.addData(imbe, 11U);
	m_p25N += 1U;
}

bool CModeConv::getPCM(int16_t *pcm)
{
	//fprintf(stderr, "CModeConv::getPCM N == %d\n", m_pcmN);
	if(m_pcmN){
		--m_pcmN;
		return m_PCM.getData(pcm, 160U);
	}
	else{
		return false;
	}
}

void CModeConv::decode_4400(int16_t *pcm, uint8_t *imbe)
{
	int16_t frame[8U] = {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};
	unsigned int offset = 0U;

	int16_t mask = 0x0800;
	for (unsigned int i = 0U; i < 12U; i++, mask >>= 1, offset++)
		frame[0U] |= READ_BIT8(imbe, offset) != 0x00U ? mask : 0x0000;

	mask = 0x0800;
	for (unsigned int i = 0U; i < 12U; i++, mask >>= 1, offset++)
		frame[1U] |= READ_BIT8(imbe, offset) != 0x00U ? mask : 0x0000;

	mask = 0x0800;
	for (unsigned int i = 0U; i < 12U; i++, mask >>= 1, offset++)
		frame[2U] |= READ_BIT8(imbe, offset) != 0x00U ? mask : 0x0000;

	mask = 0x0800;
	for (unsigned int i = 0U; i < 12U; i++, mask >>= 1, offset++)
		frame[3U] |= READ_BIT8(imbe, offset) != 0x00U ? mask : 0x0000;

	mask = 0x0400;
	for (unsigned int i = 0U; i < 11U; i++, mask >>= 1, offset++)
		frame[4U] |= READ_BIT8(imbe, offset) != 0x00U ? mask : 0x0000;

	mask = 0x0400;
	for (unsigned int i = 0U; i < 11U; i++, mask >>= 1, offset++)
		frame[5U] |= READ_BIT8(imbe, offset) != 0x00U ? mask : 0x0000;

	mask = 0x0400;
	for (unsigned int i = 0U; i < 11U; i++, mask >>= 1, offset++)
		frame[6U] |= READ_BIT8(imbe, offset) != 0x00U ? mask : 0x0000;

	mask = 0x0040;
	for (unsigned int i = 0U; i < 7U; i++, mask >>= 1, offset++)
		frame[7U] |= READ_BIT8(imbe, offset) != 0x00U ? mask : 0x0000;

	vocoder.imbe_decode(frame, pcm);
}

void CModeConv::encode_4400(int16_t *pcm, uint8_t *imbe)
{
	int16_t frame_vector[8];
	memset(imbe, 0, 9);

	vocoder.imbe_encode(frame_vector, pcm);
	//vocoder.set_gain_adjust(1.0);
	uint32_t offset = 0U;
	int16_t mask = 0x0800;
	
	for (uint32_t i = 0U; i < 12U; i++, mask >>= 1, offset++)
		WRITE_BIT8(imbe, offset, (frame_vector[0U] & mask) != 0);

	mask = 0x0800;
	for (uint32_t i = 0U; i < 12U; i++, mask >>= 1, offset++)
		WRITE_BIT8(imbe, offset, (frame_vector[1U] & mask) != 0);

	mask = 0x0800;
	for (uint32_t i = 0U; i < 12U; i++, mask >>= 1, offset++)
		WRITE_BIT8(imbe, offset, (frame_vector[2U] & mask) != 0);

	mask = 0x0800;
	for (uint32_t i = 0U; i < 12U; i++, mask >>= 1, offset++)
		WRITE_BIT8(imbe, offset, (frame_vector[3U] & mask) != 0);

	mask = 0x0400;
	for (uint32_t i = 0U; i < 11U; i++, mask >>= 1, offset++)
		WRITE_BIT8(imbe, offset, (frame_vector[4U] & mask) != 0);

	mask = 0x0400;
	for (uint32_t i = 0U; i < 11U; i++, mask >>= 1, offset++)
		WRITE_BIT8(imbe, offset, (frame_vector[5U] & mask) != 0);

	mask = 0x0400;
	for (uint32_t i = 0U; i < 11U; i++, mask >>= 1, offset++)
		WRITE_BIT8(imbe, offset, (frame_vector[6U] & mask) != 0);

	mask = 0x0040;
	for (uint32_t i = 0U; i < 7U; i++, mask >>= 1, offset++)
		WRITE_BIT8(imbe, offset, (frame_vector[7U] & mask) != 0);

}

