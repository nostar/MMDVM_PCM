/*
*   Copyright (C) 2016 by Jonathan Naylor G4KLX
*   Copyright (C) 2018 by Andy Uribe CA6JAU
*   Copyright (C) 2018 by Manuel Sanchez EA7EE
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

#if !defined(P252PCM_H)
#define P252PCM_H

#include "P25Network.h"
#include "ModeConv.h"
#include "UDPSocket.h"
#include "StopWatch.h"
#include "Version.h"
#include "Timer.h"
#include "Utils.h"
#include "Conf.h"
#include "Log.h"

#include <string>

class CP252PCM
{
public:
	CP252PCM(const std::string& configFile);
	~CP252PCM();

	int run();

private:
	std::string      m_callsign;
	CConf            m_conf;
	CP25Network*	 m_p25Network;
	CModeConv        m_conv;
	unsigned int     m_p25Src;
	unsigned int     m_p25Dst;
	unsigned char*   m_p25Frame;
	unsigned int     m_p25Frames;
	bool			 m_p25info;
	
	void audio_thread();
};

#endif
