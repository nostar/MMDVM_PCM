/*
 *   Copyright (C) 2015-2019 by Jonathan Naylor G4KLX
 *   Copyright (C) 2018,2019 by Andy Uribe CA6JAU
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

#if !defined(CONF_H)
#define	CONF_H

#include <string>
#include <vector>

class CConf
{
public:
  CConf(const std::string& file);
  ~CConf();

  bool read();

  // The Info section
  std::string  getCallsign() const;
  unsigned int getDMRId() const;

 // The P25 Network section
  bool         getDaemon() const;
  unsigned int getP25DstId() const;
  std::string  getP25DstAddress() const;
  unsigned int getP25DstPort() const;
  std::string  getP25LocalAddress() const;
  unsigned int getP25LocalPort() const;
  std::string  getP25TGListFile() const;
  bool         getP25NetworkDebug() const;

  // The Log section
  unsigned int getLogDisplayLevel() const;
  unsigned int getLogFileLevel() const;
  std::string  getLogFilePath() const;
  std::string  getLogFileRoot() const;

private:
  std::string  m_file;
  std::string  m_callsign;
  unsigned int m_p25SrcId;
  unsigned int m_p25DstId;
  std::string  m_p25DstAddress;
  unsigned int m_p25DstPort;
  std::string  m_p25LocalAddress;
  unsigned int m_p25LocalPort;
  bool         m_daemon;
  bool		   m_p25NetworkDebug;

  unsigned int m_logDisplayLevel;
  unsigned int m_logFileLevel;
  std::string  m_logFilePath;
  std::string  m_logFileRoot;
};

#endif
