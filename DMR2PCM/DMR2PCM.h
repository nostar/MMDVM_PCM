/*
*   Copyright (C) 2016 by Jonathan Naylor G4KLX
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

#if !defined(DMR2PCM_H)
#define DMR2PCM_H

#include "DMRDefines.h"
#include "ModeConv.h"
#include "MMDVMNetwork.h"
#include "DMREmbeddedData.h"
#include "DMRLC.h"
#include "DMRFullLC.h"
#include "DMREMB.h"
#include "DMRLookup.h"
#include "UDPSocket.h"
#include "StopWatch.h"
#include "Sync.h"
#include "Version.h"
#include "Thread.h"
#include "Timer.h"
#include "Utils.h"
#include "Conf.h"
#include "Log.h"
#include "CRC.h"

#include <string>

class CDMR2PCM
{
public:
	CDMR2PCM(const std::string& configFile);
	~CDMR2PCM();

	int run();

private:
	std::string      m_callsign;
	CConf            m_conf;
	CMMDVMNetwork*   m_dmrNetwork;
	CDMRLookup*      m_dmrlookup;
	CModeConv        m_conv;
	unsigned int     m_colorcode;
	unsigned int     m_dstid;
	unsigned int     m_dmrSrc;
	unsigned int     m_dmrDst;
	unsigned char    m_dmrLastDT;
	unsigned char*   m_dmrFrame;
	unsigned int     m_dmrFrames;
	CDMREmbeddedData m_EmbeddedLC;
	FLCO             m_dmrflco;
	bool             m_dmrinfo;
	unsigned char*   m_config;
	unsigned int     m_configLen;

	bool createMMDVM();
	void audio_thread();

};

#endif
