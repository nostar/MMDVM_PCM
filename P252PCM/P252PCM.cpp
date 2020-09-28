/*
*   Copyright (C) 2016,2017 by Jonathan Naylor G4KLX
*   Copyright (C) 2018,2019 by Andy Uribe CA6JAU
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

#include "P252PCM.h"

#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <thread>
#include <pwd.h>
#include <gpiod.h>
#include <pulse/simple.h>
#include <pulse/error.h>

const unsigned char REC62[] = {
	0x62U, 0x02U, 0x02U, 0x0CU, 0x0BU, 0x12U, 0x64U, 0x00U, 0x00U, 0x80U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
	0x00U, 0x00U, 0x00U, 0x00U, 0x00U};

const unsigned char REC63[] = {
	0x63U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U};

const unsigned char REC64[] = {
	0x64U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U};

const unsigned char REC65[] = {
	0x65U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U};

const unsigned char REC66[] = {
	0x66U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U};

const unsigned char REC67[] = {
	0x67U, 0xF0U, 0x9DU, 0x6AU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U};

const unsigned char REC68[] = {
	0x68U, 0x19U, 0xD4U, 0x26U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U};

const unsigned char REC69[] = {
	0x69U, 0xE0U, 0xEBU, 0x7BU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U};

const unsigned char REC6A[] = {
	0x6AU, 0x00U, 0x00U, 0x02U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};

const unsigned char REC6B[] = {
	0x6BU, 0x02U, 0x02U, 0x0CU, 0x0BU, 0x12U, 0x64U, 0x00U, 0x00U, 0x80U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
	0x00U, 0x00U, 0x00U, 0x00U, 0x00U};

const unsigned char REC6C[] = {
	0x6CU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U};

const unsigned char REC6D[] = {
	0x6DU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U};

const unsigned char REC6E[] = {
	0x6EU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U};

const unsigned char REC6F[] = {
	0x6FU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U};

const unsigned char REC70[] = {
	0x70U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U};

const unsigned char REC71[] = {
	0x71U, 0xACU, 0xB8U, 0xA4U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U};

const unsigned char REC72[] = {
	0x72U, 0x9BU, 0xDCU, 0x75U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U};

const unsigned char REC73[] = {
	0x73U, 0x00U, 0x00U, 0x02U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};

const unsigned char REC80[] = {
	0x80U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};
	
#define BUFSIZE 1024

#define P25_FRAME_PER       15U
#define PCM_FRAME_PER       10U

const char* DEFAULT_INI_FILE = "/etc/P252PCM.ini";

const char* HEADER1 = "This software is for use on amateur radio networks only,";
const char* HEADER2 = "it is to be used for educational purposes only. Its use on";
const char* HEADER3 = "commercial networks is strictly prohibited.";
const char* HEADER4 = "Copyright(C) 2018,2019 by CA6JAU, G4KLX and others";

#include <functional>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <clocale>
#include <cctype>

int end = 0;

void sig_handler(int signo)
{
	if (signo == SIGTERM) {
		end = 1;
		::fprintf(stdout, "Received SIGTERM\n");
	}
}

int main(int argc, char** argv)
{
	const char* iniFile = DEFAULT_INI_FILE;
	if (argc > 1) {
		for (int currentArg = 1; currentArg < argc; ++currentArg) {
			std::string arg = argv[currentArg];
			if ((arg == "-v") || (arg == "--version")) {
				::fprintf(stdout, "P252PCM version %s\n", VERSION);
				return 0;
			} else if (arg.substr(0, 1) == "-") {
				::fprintf(stderr, "Usage: P252PCM [-v|--version] [filename]\n");
				return 1;
			} else {
				iniFile = argv[currentArg];
			}
		}
	}

	// Capture SIGTERM to finish gracelessly
	if (signal(SIGTERM, sig_handler) == SIG_ERR) 
		::fprintf(stdout, "Can't catch SIGTERM\n");

	CP252PCM* gateway = new CP252PCM(std::string(iniFile));

	int ret = gateway->run();

	delete gateway;

	return ret;
}

CP252PCM::CP252PCM(const std::string& configFile) :
m_callsign(),
m_conf(configFile),
m_conv()
{
	m_p25Frame = new unsigned char[200U];
	::memset(m_p25Frame, 0U, 200U);
}

CP252PCM::~CP252PCM()
{
	delete[] m_p25Frame;
}

void CP252PCM::audio_thread()
{
	static const pa_sample_spec format = {
        .format = PA_SAMPLE_S16LE,
        .rate = 8000,
        .channels = 1
    };
    
    pa_simple *in_pcm = NULL;
    pa_simple *out_pcm = NULL;
    //pa_usec_t in_pcm_delay;
    int error;
    bool tx = false;
    int16_t pcm[160];
    
    if (!(out_pcm = pa_simple_new(NULL, "P252PCM", PA_STREAM_PLAYBACK, NULL, "playback", &format, NULL, NULL, &error))) {
        fprintf(stderr, "pa_simple_new() failed: %s\n", pa_strerror(error));
    }
    
	while(1){
		if(m_conv.getPCM(pcm)){
			if(pa_simple_write(out_pcm, (uint8_t *)pcm, 320, &error) < 0){
				fprintf(stderr, "pa_simple_write() failed: %s\n", pa_strerror(error));
			}
		}
		if(gpiod_ctxless_get_value("gpiochip0", 12, 0, "P252PCM")){
		//if(0){
			if(!tx){
				memset(pcm, 0, sizeof(pcm));
				if (!(in_pcm = pa_simple_new(NULL, "P252PCM", PA_STREAM_RECORD, NULL, "record", &format, NULL, NULL, &error))) {
					fprintf(stderr, "pa_simple_new() failed: %s\n", pa_strerror(error));
				}
				if(pa_simple_read(in_pcm, (uint8_t *)pcm, 320, &error) < 0){
					fprintf(stderr, "pa_simple_read() failed: %s\n", pa_strerror(error));
				}
				m_conv.putPCMHeader(pcm);
				tx = true;
			}
			if(pa_simple_read(in_pcm, (uint8_t *)pcm, 320, &error) < 0){
				fprintf(stderr, "pa_simple_read() failed: %s\n", pa_strerror(error));
			}
			//if(pa_simple_write(out_pcm, (uint8_t *)pcm, 320, &error) < 0){
			//	fprintf(stderr, "pa_simple_write() failed: %s\n", pa_strerror(error));
			//}
			
			m_conv.putPCM(pcm);
			
		}
		else{
			if(tx){
				memset(pcm, 0, sizeof(pcm));
				fprintf(stderr, "sending EOT\n");
				m_conv.putPCMEOT(pcm);
				pa_simple_free(in_pcm);
			}
			tx = false;
		}
	}
}

int CP252PCM::run()
{
	bool ret = m_conf.read();
	if (!ret) {
		::fprintf(stderr, "P252PCM: cannot read the .ini file\n");
		return 1;
	}

	setlocale(LC_ALL, "C");

	unsigned int logDisplayLevel = m_conf.getLogDisplayLevel();

	if(m_conf.getDaemon())
		logDisplayLevel = 0U;

	bool m_daemon = m_conf.getDaemon();
	if (m_daemon) {
		// Create new process
		pid_t pid = ::fork();
		if (pid == -1) {
			::fprintf(stderr, "Couldn't fork() , exiting\n");
			return -1;
		} else if (pid != 0)
			exit(EXIT_SUCCESS);

		// Create new session and process group
		if (::setsid() == -1) {
			::fprintf(stderr, "Couldn't setsid(), exiting\n");
			return -1;
		}

		// Set the working directory to the root directory
		if (::chdir("/") == -1) {
			::fprintf(stderr, "Couldn't cd /, exiting\n");
			return -1;
		}

		//If we are currently root...
		if (getuid() == 0) {
			struct passwd* user = ::getpwnam("mmdvm");
			if (user == NULL) {
				::fprintf(stderr, "Could not get the mmdvm user, exiting\n");
				return -1;
			}

			uid_t mmdvm_uid = user->pw_uid;
			gid_t mmdvm_gid = user->pw_gid;

			//Set user and group ID's to mmdvm:mmdvm
			if (setgid(mmdvm_gid) != 0) {
				::fprintf(stderr, "Could not set mmdvm GID, exiting\n");
				return -1;
			}

			if (setuid(mmdvm_uid) != 0) {
				::fprintf(stderr, "Could not set mmdvm UID, exiting\n");
				return -1;
			}

			//Double check it worked (AKA Paranoia) 
			if (setuid(0) != -1) {
				::fprintf(stderr, "It's possible to regain root - something is wrong!, exiting\n");
				return -1;
			}
		}
	}

	ret = ::LogInitialise(m_conf.getLogFilePath(), m_conf.getLogFileRoot(), m_conf.getLogFileLevel(), logDisplayLevel);
	if (!ret) {
		::fprintf(stderr, "P252PCM: unable to open the log file\n");
		return 1;
	}

	if (m_daemon) {
		::close(STDIN_FILENO);
		::close(STDOUT_FILENO);
		::close(STDERR_FILENO);
	}

	LogInfo(HEADER1);
	LogInfo(HEADER2);
	LogInfo(HEADER3);
	LogInfo(HEADER4);

	m_callsign = m_conf.getCallsign();
	m_p25Src = m_conf.getDMRId();
	m_p25Dst = m_conf.getP25DstId();

	std::string p25_dstAddress   = m_conf.getP25DstAddress();
	unsigned int p25_dstPort     = m_conf.getP25DstPort();
	std::string p25_localAddress = m_conf.getP25LocalAddress();
	unsigned int p25_localPort   = m_conf.getP25LocalPort();
	bool p25_debug               = m_conf.getP25NetworkDebug();

	m_p25Network = new CP25Network(p25_localAddress, p25_localPort, p25_dstAddress, p25_dstPort, m_callsign, p25_debug);
	
	ret = m_p25Network->open();
	if (!ret) {
		::LogError("Cannot open the P25 network port");
		::LogFinalise();
		return 1;
	}

	CTimer pollTimer(1000U, 5U);
	
	CStopWatch stopWatch;
	CStopWatch p25Watch;
	stopWatch.start();
	p25Watch.start();
	pollTimer.start();

	unsigned char p25_cnt = 0;

    std::thread audio_thread(&CP252PCM::audio_thread, this);

	LogMessage("Starting P252PCM-%s", VERSION);
	
	for (; end == 0;) {
		unsigned char buffer[2000U];
		unsigned int srcId = 0U;
		unsigned int dstId = 0U;
		int16_t pcm[160];
		memset(pcm, 0, sizeof(pcm));
		//uint8_t buf[320];
		unsigned int ms = stopWatch.elapsed();

		while (m_p25Network->readData(m_p25Frame, 22U) > 0U) {
			//CUtils::dump(1U, "P25 Data", m_p25Frame, 22U);
			if (m_p25Frame[0U] != 0xF0U && m_p25Frame[0U] != 0xF1U) {
				if (m_p25Frame[0U] == 0x62U && !m_p25info) {
					LogMessage("Received P25 Header");
					m_p25Frames = 0;
					//m_conv.putP25Header();
				} else if (m_p25Frame[0U] == 0x65U && !m_p25info) {
					dstId  = (m_p25Frame[1U] << 16) & 0xFF0000U;
					dstId |= (m_p25Frame[2U] << 8)  & 0x00FF00U;
					dstId |= (m_p25Frame[3U] << 0)  & 0x0000FFU;
				} else if (m_p25Frame[0U] == 0x66U && !m_p25info) {
					srcId  = (m_p25Frame[1U] << 16) & 0xFF0000U;
					srcId |= (m_p25Frame[2U] << 8)  & 0x00FF00U;
					srcId |= (m_p25Frame[3U] << 0)  & 0x0000FFU;
					LogMessage("Received P25 audio: Src: %d Dst: %d", srcId, dstId);
					m_p25info = true;
				} else if (m_p25Frame[0U] == 0x80U) {
					LogMessage("P25 received end of voice transmission, %.1f seconds", float(m_p25Frames) / 50.0F);
					m_p25info = false;
					//m_conv.putP25EOT();
				}
				m_conv.putP25(m_p25Frame);
				m_p25Frames++;
			}
		}
		if (p25Watch.elapsed() > P25_FRAME_PER) {
			unsigned int p25FrameType = m_conv.getP25(m_p25Frame);
			
			if(p25FrameType == TAG_HEADER) {
				p25_cnt = 0U;
				p25Watch.start();
			}
			else if(p25FrameType == TAG_EOT) {
				m_p25Network->writeData(REC80, 17U);
				p25Watch.start();
			}
			else if(p25FrameType == TAG_DATA) {
				unsigned int p25step = p25_cnt % 18U;
				//CUtils::dump(1U, "P25 Data", m_p25Frame, 11U);

				if (p25_cnt > 2U) {
					switch (p25step) {
					case 0x00U:
						::memcpy(buffer, REC62, 22U);
						::memcpy(buffer + 10U, m_p25Frame, 11U);
						m_p25Network->writeData(buffer, 22U);
						break;
					case 0x01U:
						::memcpy(buffer, REC63, 14U);
						::memcpy(buffer + 1U, m_p25Frame, 11U);
						m_p25Network->writeData(buffer, 14U);
						break;
					case 0x02U:
						::memcpy(buffer, REC64, 17U);
						::memcpy(buffer + 5U, m_p25Frame, 11U);
						buffer[1U] = 0x00U;
						m_p25Network->writeData(buffer, 17U);
						break;
					case 0x03U:
						::memcpy(buffer, REC65, 17U);
						::memcpy(buffer + 5U, m_p25Frame, 11U);
						buffer[1U] = (m_p25Dst >> 16) & 0xFFU;
						buffer[2U] = (m_p25Dst >> 8) & 0xFFU;
						buffer[3U] = (m_p25Dst >> 0) & 0xFFU;
						m_p25Network->writeData(buffer, 17U);
						break;
					case 0x04U:
						::memcpy(buffer, REC66, 17U);
						::memcpy(buffer + 5U, m_p25Frame, 11U);
						buffer[1U] = (m_p25Src >> 16) & 0xFFU;
						buffer[2U] = (m_p25Src >> 8) & 0xFFU;
						buffer[3U] = (m_p25Src >> 0) & 0xFFU;
						m_p25Network->writeData(buffer, 17U);
						break;
					case 0x05U:
						::memcpy(buffer, REC67, 17U);
						::memcpy(buffer + 5U, m_p25Frame, 11U);
						m_p25Network->writeData(buffer, 17U);
						break;
					case 0x06U:
						::memcpy(buffer, REC68, 17U);
						::memcpy(buffer + 5U, m_p25Frame, 11U);
						m_p25Network->writeData(buffer, 17U);
						break;
					case 0x07U:
						::memcpy(buffer, REC69, 17U);
						::memcpy(buffer + 5U, m_p25Frame, 11U);
						m_p25Network->writeData(buffer, 17U);
						break;
					case 0x08U:
						::memcpy(buffer, REC6A, 16U);
						::memcpy(buffer + 4U, m_p25Frame, 11U);
						m_p25Network->writeData(buffer, 16U);
						break;
					case 0x09U:
						::memcpy(buffer, REC6B, 22U);
						::memcpy(buffer + 10U, m_p25Frame, 11U);
						m_p25Network->writeData(buffer, 22U);
						break;
					case 0x0AU:
						::memcpy(buffer, REC6C, 14U);
						::memcpy(buffer + 1U, m_p25Frame, 11U);
						m_p25Network->writeData(buffer, 14U);
						break;
					case 0x0BU:
						::memcpy(buffer, REC6D, 17U);
						::memcpy(buffer + 5U, m_p25Frame, 11U);
						m_p25Network->writeData(buffer, 17U);
						break;
					case 0x0CU:
						::memcpy(buffer, REC6E, 17U);
						::memcpy(buffer + 5U, m_p25Frame, 11U);
						m_p25Network->writeData(buffer, 17U);
						break;
					case 0x0DU:
						::memcpy(buffer, REC6F, 17U);
						::memcpy(buffer + 5U, m_p25Frame, 11U);
						m_p25Network->writeData(buffer, 17U);
						break;
					case 0x0EU:
						::memcpy(buffer, REC70, 17U);
						::memcpy(buffer + 5U, m_p25Frame, 11U);
						buffer[1U] = 0x80U;
						m_p25Network->writeData(buffer, 17U);
						break;
					case 0x0FU:
						::memcpy(buffer, REC71, 17U);
						::memcpy(buffer + 5U, m_p25Frame, 11U);
						m_p25Network->writeData(buffer, 17U);
						break;
					case 0x10U:
						::memcpy(buffer, REC72, 17U);
						::memcpy(buffer + 5U, m_p25Frame, 11U);
						m_p25Network->writeData(buffer, 17U);
						break;
					case 0x11U:
						::memcpy(buffer, REC73, 16U);
						::memcpy(buffer + 4U, m_p25Frame, 11U);
						m_p25Network->writeData(buffer, 16U);
						break;
					}
				}

				p25_cnt++;
				p25Watch.start();
			}
		}
		
		stopWatch.start();
		pollTimer.clock(ms);
		
		if (pollTimer.isRunning() && pollTimer.hasExpired()) {
			m_p25Network->writePoll();
			pollTimer.start();
		}

		if (ms < 5U)
			::usleep(5 * 1000);
	}

	m_p25Network->close();

	delete m_p25Network;

	::LogFinalise();

	return 0;
}

