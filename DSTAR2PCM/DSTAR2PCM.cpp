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

#include "DSTAR2PCM.h"

#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <thread>
#include <pwd.h>
#include <gpiod.h>
#include <pulse/simple.h>
#include <pulse/error.h>

#define BUFSIZE 1024

#define DSTAR_FRAME_PER     18U

const char* DEFAULT_INI_FILE = "/etc/DSTAR2PCM.ini";

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
				::fprintf(stdout, "DSTAR2PCM version %s\n", VERSION);
				return 0;
			} else if (arg.substr(0, 1) == "-") {
				::fprintf(stderr, "Usage: DSTAR2PCM [-v|--version] [filename]\n");
				return 1;
			} else {
				iniFile = argv[currentArg];
			}
		}
	}

	// Capture SIGTERM to finish gracelessly
	if (signal(SIGTERM, sig_handler) == SIG_ERR) 
		::fprintf(stdout, "Can't catch SIGTERM\n");

	CDSTAR2PCM* gateway = new CDSTAR2PCM(std::string(iniFile));

	int ret = gateway->run();

	delete gateway;

	return ret;
}

CDSTAR2PCM::CDSTAR2PCM(const std::string& configFile) :
m_conf(configFile),
m_conv("/dev/ttyUSB0")
{
	m_dstarFrame = new unsigned char[200U];
	::memset(m_dstarFrame, 0U, 200U);
}

CDSTAR2PCM::~CDSTAR2PCM()
{
	delete[] m_dstarFrame;
}

void CDSTAR2PCM::audio_thread()
{
	static const pa_sample_spec format = {
        .format = PA_SAMPLE_S16BE,
        .rate = 8000,
        .channels = 1
    };
    
    static const pa_buffer_attr bufattr = {
        .maxlength = (uint32_t)-1,
        .tlength = (uint32_t)-1,
        .prebuf = (uint32_t)-1,
        .minreq = (uint32_t)-1
    };
    
    pa_simple *in_pcm = NULL;
    pa_simple *out_pcm = NULL;
    
    int error;
    bool tx = false;
    int16_t pcm[160];
    
    
    if (!(out_pcm = pa_simple_new(NULL, "DSTAR2PCM", PA_STREAM_PLAYBACK, NULL, "playback", &format, NULL, &bufattr, &error))) {
        fprintf(stderr, "pa_simple_new() failed: %s\n", pa_strerror(error));
    }
    
	while(1){
		//::usleep(15000);
		if(m_conv.getPCM(pcm)){
			/*
			fprintf(stderr, "PCM: ");
			for(int i = 0; i < 160; ++i){
				fprintf(stderr, "%04x ", (uint16_t)pcm[i]);
			}
			fprintf(stderr, "\n");
			fflush(stderr);
			*/
			pa_usec_t latency;
			
			if ((latency = pa_simple_get_latency(out_pcm, &error)) == (pa_usec_t) -1) {
				fprintf(stderr, __FILE__": pa_simple_get_latency() failed: %s\n", pa_strerror(error));
			}
			
			fprintf(stderr, "Playback latency: %0.0f usec    \n", (float)latency);
			
			if(pa_simple_write(out_pcm, (uint8_t *)pcm, 320, &error) < 0){
				fprintf(stderr, "pa_simple_write() failed: %s\n", pa_strerror(error));
			}
		}
		if(gpiod_ctxless_get_value("gpiochip0", 12, 0, "DSTAR2PCM")){
		//if(0){
			if(!tx){
				memset(pcm, 0, sizeof(pcm));
				if (!(in_pcm = pa_simple_new(NULL, "DSTAR2PCM", PA_STREAM_RECORD, NULL, "record", &format, NULL, &bufattr, &error))) {
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

int CDSTAR2PCM::run()
{
	bool ret = m_conf.read();
	if (!ret) {
		::fprintf(stderr, "DSTAR2PCM: cannot read the .ini file\n");
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
		::fprintf(stderr, "DSTAR2PCM: unable to open the log file\n");
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

	std::string mycall = m_conf.getMycall();
	std::string urcall = m_conf.getUrcall();
	std::string rptr1 = m_conf.getRptr1();
	std::string rptr2 = m_conf.getRptr2();
	std::string suffix = m_conf.getSuffix();
	
	char usertxt[20];
	::memset(usertxt, 0x20, 20);
	::memcpy(usertxt, m_conf.getUserTxt().c_str(), (m_conf.getUserTxt().size() <= 20) ? m_conf.getUserTxt().size() : 20);

	std::string dstar_dstAddress   = m_conf.getDSTARDstAddress();
	unsigned int dstar_dstPort     = m_conf.getDSTARDstPort();
	std::string dstar_localAddress = m_conf.getDSTARLocalAddress();
	unsigned int dstar_localPort   = m_conf.getDSTARLocalPort();
	bool dstar_debug               = m_conf.getDSTARNetworkDebug();

	m_dstarNetwork = new CDSTARNetwork(dstar_localAddress, dstar_localPort, dstar_dstAddress, dstar_dstPort, mycall, dstar_debug);
	
	ret = m_dstarNetwork->open();
	if (!ret) {
		::LogError("Cannot open the DSTAR network port");
		::LogFinalise();
		return 1;
	}

	CTimer pollTimer(1000U, 5U);
	
	CStopWatch stopWatch;
	CStopWatch dstarWatch;
	stopWatch.start();
	dstarWatch.start();
	pollTimer.start();

	unsigned char dstar_cnt = 0;

    std::thread audio_thread(&CDSTAR2PCM::audio_thread, this);

	LogMessage("Starting DSTAR2PCM-%s", VERSION);
	
	for (; end == 0;) {
		unsigned char data[41U];
		int16_t pcm[160];
		memset(pcm, 0, sizeof(pcm));
		unsigned int ms = stopWatch.elapsed();

		while (m_dstarNetwork->readData(m_dstarFrame, 49U) > 0U) {
			if(::memcmp("DSRP!", m_dstarFrame, 5) == 0){
				m_conv.putDSTAR(m_dstarFrame + 9);
			}
			//CUtils::dump(1U, "DSTAR Data", m_dstarFrame, 49U);
		}
		
		if (dstarWatch.elapsed() > DSTAR_FRAME_PER) {
			unsigned int dstarFrameType = m_conv.getDSTAR(m_dstarFrame);
			
			if(dstarFrameType == TAG_HEADER) {
				data[0] = 0;
				data[1] = 0;
				data[2] = 0;
				
				::memcpy(data+3, rptr2.c_str(), rptr2.size());
				::memset(data+3+rptr2.size(), 0x20, 8-rptr2.size());
				
				::memcpy(data+11, rptr1.c_str(), rptr1.size());
				::memset(data+11+rptr1.size(), 0x20, 8-rptr1.size());
				
				::memcpy(data+19, urcall.c_str(), urcall.size());
				::memset(data+19+urcall.size(), 0x20, 8-urcall.size());
				
				::memcpy(data+27, mycall.c_str(), mycall.size());
				::memset(data+27+mycall.size(), 0x20, 8-mycall.size());
				
				::memcpy(data+35, suffix.c_str(), 4);
				
				data[39] = 0xea;
				data[40] = 0x7e;
				m_dstarNetwork->writeHeader(data, 41U);
				//m_dstarNetwork->writeHeader(data, 41U);
				dstar_cnt = 0U;
				dstarWatch.start();
				
			}
			else if(dstarFrameType == TAG_EOT) {
				::memcpy(data, m_dstarFrame, 9);
				if(dstar_cnt % 0x15 == 0){
					data[9] = 0x55;
					data[10] = 0x2d;
					data[11] = 0x16;
				}
				else{
					data[9] = 0;
					data[10] = 0;
					data[11] = 0;
				}
				m_dstarNetwork->writeData(data, 12U, true);
				dstarWatch.start();
			}
			else if(dstarFrameType == TAG_DATA) {
				::memcpy(data, m_dstarFrame, 9);
				switch(dstar_cnt){
				case 0:
					data[9]  = 0x55;
					data[10] = 0x2d;
					data[11] = 0x16;
					break;
				case 1:
					data[9]  = 0x40 ^ 0x70;
					data[10] = usertxt[0] ^ 0x4f;
					data[11] = usertxt[1] ^ 0x93;
					break;
				case 2:
					data[9]  = usertxt[2] ^ 0x70;
					data[10] = usertxt[3] ^ 0x4f;
					data[11] = usertxt[4] ^ 0x93;
					break;
				case 3:
					data[9]  = 0x41 ^ 0x70;
					data[10] = usertxt[5] ^ 0x4f;
					data[11] = usertxt[6] ^ 0x93;
					break;
				case 4:
					data[9]  = usertxt[7] ^ 0x70;
					data[10] = usertxt[8] ^ 0x4f;
					data[11] = usertxt[9] ^ 0x93;
					break;
				case 5:
					data[9]  = 0x42 ^ 0x70;
					data[10] = usertxt[10] ^ 0x4f;
					data[11] = usertxt[11] ^ 0x93;
					break;
				case 6:
					data[9]  = usertxt[12] ^ 0x70;
					data[10] = usertxt[13] ^ 0x4f;
					data[11] = usertxt[14] ^ 0x93;
					break;
				case 7:
					data[9]  = 0x43 ^ 0x70;
					data[10] = usertxt[15] ^ 0x4f;
					data[11] = usertxt[16] ^ 0x93;
					break;
				case 8:
					data[9]  = usertxt[17] ^ 0x70;
					data[10] = usertxt[18] ^ 0x4f;
					data[11] = usertxt[19] ^ 0x93;
					break;
				default:
					data[9]  = 0x16;
					data[10] = 0x29;
					data[11] = 0xf5;
					break;
				}
				
				m_dstarNetwork->writeData(data, 12U, false);
				//CUtils::dump(1U, "P25 Data", m_p25Frame, 11U);
				(dstar_cnt >= 0x14) ? dstar_cnt = 0 : ++dstar_cnt;
				dstarWatch.start();
			}
		}
		
		stopWatch.start();
		pollTimer.clock(ms);
		
		if (pollTimer.isRunning() && pollTimer.hasExpired()) {
			//m_dstarNetwork->writePoll();
			pollTimer.start();
		}

		//if (ms < 5U)
		//	::usleep(5 * 1000);
	}

	m_dstarNetwork->close();

	delete m_dstarNetwork;

	::LogFinalise();

	return 0;
}
