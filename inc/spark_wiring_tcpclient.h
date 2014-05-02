/**
 ******************************************************************************
 * @file    spark_wiring_tcpclient.h
 * @author  Satish Nair
 * @version V1.0.0
 * @date    13-March-2013
 * @brief   Header for spark_wiring_tcpclient.cpp module
 ******************************************************************************
  Copyright (c) 2013 Spark Labs, Inc.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */

#ifndef __SPARK_WIRING_TCPCLIENT_H
#define __SPARK_WIRING_TCPCLIENT_H

#include "spark_wiring.h"

#define TCPCLIENT_BUF_MAX_SIZE	512

class TCPClient : public Stream {

  public:
    TCPClient();
    TCPClient(uint8_t sock);
    virtual ~TCPClient() {};

    uint8_t status();
    virtual int connect(IPAddress ip, uint16_t port);
    virtual int connect(const char *host, uint16_t port);
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t *buffer, size_t size);
    virtual int available();
    virtual int read();
    virtual int read(uint8_t *buffer, size_t size);
    virtual int peek();
    virtual void flush();
    virtual void stop();
    virtual bool connected();
    virtual operator bool();
    virtual bool equals(TCPClient &tcpClient);
    void resetIp() {
      ip[0]=0;
      ip[1]=0;
      ip[2]=0;
      ip[3]=0;
      ip[4]=0;
    }
    void setIp (IPAddress &address, uint8_t port){
      ip[0]=address[0];
      ip[1]=address[1];
      ip[2]=address[2];
      ip[3]=address[3];
      ip[4]=port;

    }
    void getIP(String &str) {
      str+=ip[0];
      str+=".";
      str+=ip[1];
      str+=".";
      str+=ip[2];
      str+=".";
      str+=ip[3];
      str+=".";
      str+=ip[4];
    }
    friend class TCPServer;

    using Print::write;

  private:
    uint16_t _srcport;
    unsigned short ip[5];

    long _sock;
    uint8_t _buffer[TCPCLIENT_BUF_MAX_SIZE];
    uint16_t _offset;
    uint16_t _total;
    inline int bufferCount();
    inline int isWanReady();
};

#endif
