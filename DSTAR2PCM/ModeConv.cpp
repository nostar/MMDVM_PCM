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
#include <queue>


const uint8_t  BIT_MASK_TABLE8[]  = { 0x80U, 0x40U, 0x20U, 0x10U, 0x08U, 0x04U, 0x02U, 0x01U };
#define WRITE_BIT8(p,i,b)   p[(i)>>3] = (b) ? (p[(i)>>3] | BIT_MASK_TABLE8[(i)&7]) : (p[(i)>>3] & ~BIT_MASK_TABLE8[(i)&7])
#define READ_BIT8(p,i)     (p[(i)>>3] & BIT_MASK_TABLE8[(i)&7])

CModeConv::CModeConv(std::string device) :
m_dstarN(0U),
m_pcmN(0U),
m_DSTAR(5000U, "PCM2DSTAR"),
m_PCM(500000U, "DSTAR2PCM")
{
	uint8_t buf[512];
	::memset(buf, 0, sizeof(buf));
	const uint8_t AMBE2000_2400_1200[17] = {0x61, 0x00, 0x0d, 0x00, 0x0a, 0x01U, 0x30U, 0x07U, 0x63U, 0x40U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x48U};
	fprintf(stderr, "Creating vocoder on %s\n", device.c_str());
	vocoder = new CSerialController(device, SERIAL_460800);
	fprintf(stderr, "Opening vocoder on %s\n", device.c_str());
	vocoder->open();
	fprintf(stderr, "Opened vocoder on %s\n", device.c_str());
	int lw = vocoder->write(AMBE2000_2400_1200, sizeof(AMBE2000_2400_1200));
	int lr = vocoder->read(buf, 6);
	
	fprintf(stderr, "AMBEHW:%d:%d:   ", lw, lr);
	for(int i = 0; i < lr; ++i){
		fprintf(stderr, "%02x ", buf[i]);
	}
	fprintf(stderr, "\n");
	fflush(stderr);
	//vocoder_thread = new std::thread(&CModeConv::vocoder_thread_fn, this);
}

CModeConv::~CModeConv()
{
}

void CModeConv::vocoder_thread_fn()
{
	uint8_t dvsi_rx[512];
	int16_t pcm[160U];
	uint8_t pcm_bytes[320U];
	
	std::queue<uint8_t> d;
	::memset(dvsi_rx, 0, 512);
	::memset(pcm, 0, sizeof(pcm));
	fprintf(stderr, "CModeConv::vocoder_thread_fn() started\n");
	while(1){
		int lr = vocoder->read(dvsi_rx, sizeof(dvsi_rx));
		for(int i = 0; i < lr; ++i){
			d.push(dvsi_rx[i]);
		}
		if(lr != 326){
			fprintf(stderr, "vocoder returned %d\n", lr);
			//continue;
		}
		if(d.size() > 325){
			d.pop();
			d.pop();
			d.pop();
			d.pop();
			d.pop();
			d.pop();
			for(int i = 0; i < 320; ++i){
				pcm_bytes[i] = d.front();
				d.pop();
			}
			::memcpy(pcm, pcm_bytes, 320);
			m_PCM.addData(pcm, 160U);
		m_pcmN += 1;
		}
	}
}

void CModeConv::putDSTAR(unsigned char* ambe)
{
	assert(ambe != NULL);

	int16_t pcm[160U];
	::memset(pcm, 0, sizeof(pcm));
	
	decode_2400(pcm, ambe);
	m_PCM.addData(pcm, 160U);
	m_pcmN += 1;
}

unsigned int CModeConv::getDSTAR(unsigned char* data)
{
	unsigned char tag[1U];

	tag[0U] = TAG_NODATA;

	if (m_dstarN >= 1U) {
		m_DSTAR.getData(tag, 1U);
		m_DSTAR.getData(data, 9U);
		m_dstarN -= 1U;

		return tag[0U];
	}
	else
		return TAG_NODATA;
}

void CModeConv::putPCMHeader(int16_t *pcm)
{
	uint8_t ambe[9U];
	encode_2400(pcm, ambe);
	m_DSTAR.addData(&TAG_HEADER, 1U);
	m_DSTAR.addData(ambe, 9U);
	m_dstarN += 1U;
}

void CModeConv::putPCM(int16_t *pcm)
{
	uint8_t ambe[9U];
	encode_2400(pcm, ambe);
	m_DSTAR.addData(&TAG_DATA, 1U);
	m_DSTAR.addData(ambe, 9U);
	m_dstarN += 1U;
}

void CModeConv::putPCMEOT(int16_t *pcm)
{
	uint8_t ambe[9U];
	encode_2400(pcm, ambe);
	m_DSTAR.addData(&TAG_EOT, 1U);
	m_DSTAR.addData(ambe, 9U);
	m_dstarN += 1U;
}

bool CModeConv::getPCM(int16_t *pcm)
{
	//fprintf(stderr, "CModeConv::getPCM N == %d\n", m_pcmN);
	if(m_pcmN ){
		m_pcmN -= 1;
		return m_PCM.getData(pcm, 160U);
	}
	else{
		return false;
	}
}

void CModeConv::decode_2400(int16_t *pcm, uint8_t *ambe)
{
	uint8_t dvsi_tx[15] = {0x61, 0x00, 0x0b, 0x01, 0x01, 0x48};
	uint8_t dvsi_rx[326];
	::memcpy(dvsi_tx + 6, ambe, 9);
	
	int lw = vocoder->write(dvsi_tx, 15);
	int lr = vocoder->read(dvsi_rx, sizeof(dvsi_rx));
	
	if(lr != 326){
		fprintf(stderr, "vocoder returned %d:%d\n", lr, lw);
		return;
	}
	
	//uint8_t *p_rx = dvsi_rx + 6;
	//for(int i = 0; i < 160; ++i){
	//	pcm[i] = ((p_rx[(i*2)+1] << 8) & 0xff00) | (p_rx[i*2] & 0xff);
	//}
	::memcpy(pcm, dvsi_rx + 6, 320);
	/*
	fprintf(stderr, "AMBEHW:%d:%d:  ", lw, lr);
	for(int i = 0; i < lr; ++i){
		fprintf(stderr, "%02x ", dvsi_rx[i]);
	}
	fprintf(stderr, "\n");
	fflush(stderr);
	*/
}

void CModeConv::encode_2400(int16_t *pcm, uint8_t *ambe)
{
	uint8_t dvsi_tx[326] = {0x61, 0x01, 0x42, 0x02, 0x00, 0xa0};
	uint8_t dvsi_rx[15];
	::memcpy(dvsi_tx + 6, pcm, 320);
	
	int lw = vocoder->write(dvsi_tx, sizeof(dvsi_tx));
	int lr = vocoder->read(dvsi_rx, sizeof(dvsi_rx));
	
	::memcpy(ambe, dvsi_rx + 6, 9);
	
	fprintf(stderr, "AMBEHW:%d:%d:  ", lw, lr);
	for(int i = 0; i < lr; ++i){
		fprintf(stderr, "%02x ", dvsi_rx[i]);
	}
	fprintf(stderr, "\n");
	fflush(stderr);
}
