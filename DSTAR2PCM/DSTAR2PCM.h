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

#if !defined(DSTAR2PCM_H)
#define DSTAR2PCM_H

#include "DSTARNetwork.h"
#include "ModeConv.h"
#include "UDPSocket.h"
#include "StopWatch.h"
#include "Version.h"
#include "Timer.h"
#include "Utils.h"
#include "Conf.h"
#include "Log.h"

#include <string>

class CDSTAR2PCM
{
public:
	CDSTAR2PCM(const std::string& configFile);
	~CDSTAR2PCM();

	int run();

private:
	CConf            m_conf;
	CDSTARNetwork*	 m_dstarNetwork;
	CModeConv        m_conv;
	unsigned char*   m_dstarFrame;
	unsigned int     m_dstarFrames;
	
	void audio_thread();
};

#endif
