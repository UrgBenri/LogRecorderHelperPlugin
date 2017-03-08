/*
	This file is part of the UrgBenri application.

	Copyright (c) 2016 Mehrez Kristou.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Please contact kristou@hokuyo-aut.jp for more details.

*/

#ifndef TCPDEVICE_H
#define TCPDEVICE_H

#include <QtGlobal>
#include <QFile>
#include "Connection.h"
#include "RingBuffer.h"

///////////////////////To change
#include <sys/types.h>
#if defined(Q_OS_WIN)
#include <windows.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
/////////////////////////////////////////////////

#include <string>

using namespace std;

namespace qrk
{

class TcpDevice : public Connection
{
public:
    TcpDevice();
    virtual ~TcpDevice(void);

    const char* what() const;

    bool connect(const char* host, long port);
    void disconnect(void);
    bool setBaudrate(long b);
    long baudrate(void) const;
    bool isConnected(void) const;
    int send(const char* data, size_t count);
    int receive(char* data, size_t count, int timeout);
    size_t size() const;
    void flush(void);
    void clear(void);
    void ungetc(const char ch);

    ConnectionType connectionType() { return ETHERNET_TYPE;}

    QString getDevice();

    int getRecieveBufferSize() const;
    void setRecieveBufferSize(int value);

private:
    long m_baudrate;
    bool is_connected;
    string error_message;
    RingBuffer<char> ring_buffer_;

    QString m_device;

    ///////////////////////To change
    enum {
        Invalid_desc = -1,
    };
    struct sockaddr_in server_addr;
    int sock_desc;
    int recieveBufferSize;


    void set_block_mode();
    //////////////////////////////////////////////
    int rawReceive(char *data, size_t count, int timeout);
    unsigned long rawLength();
    void updateRingBuffer();
};

}

#endif // TCPDEVICE_H

