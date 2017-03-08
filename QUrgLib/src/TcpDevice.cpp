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

#include "TcpDevice.h"

#if !defined(Q_OS_WIN)
#include <cstring>
#include <fcntl.h>
#include <cerrno>
#include <sys/ioctl.h>
#include <unistd.h>
#endif
#include <stdio.h>

#include <iostream>

using namespace std;
using namespace qrk;

TcpDevice::TcpDevice()
    : is_connected(false)
    , error_message("No errors.")
    , sock_desc(Invalid_desc)
    , recieveBufferSize(65535)
    , m_baudrate(10940)
{
}

TcpDevice::~TcpDevice()
{
    disconnect();
}

const char* TcpDevice::what() const
{
    return error_message.c_str();
}

bool TcpDevice::connect(const char* host, long port)
{
    if (is_connected) {
        return true;
    }
    /////////////////// To change
    enum { Connect_timeout_sec = 0,
           Connect_timeout_usec = 200 * 1000
         };
    struct timeval tv = { Connect_timeout_sec, Connect_timeout_usec };
    fd_set rmask, wmask;
    //    struct timeval tv = { Connect_timeout_sec, Connect_timeout_usec };
#if defined(Q_OS_WIN)
    u_long flag;
#else
    int flag;
    int sock_optval = -1;
    int sock_optval_size = sizeof(sock_optval);
#endif

    disconnect();

    sock_desc = Invalid_desc;

#if defined(Q_OS_WIN)
    {
        static int is_initialized = 0;

        if (!is_initialized) {
            WORD wVersionRequested = 0x0202;
            WSADATA WSAData;

            if (WSAStartup(wVersionRequested, &WSAData) != 0) {
                error_message = "Connection initiation failed.";
                return false;
            }

            //Did we get the right Winsock version?
            if (WSAData.wVersion != 0x0202) {
#ifdef DEBUG
                cerr << "ERROR: Setting Socket to version 2 failed" << endl;
#endif
            }

            is_initialized = 1;
        }
    }
#endif

#ifdef DEBUG
    cout << "Socket start up: " << timer.elapsed() << "ms" << endl;
#endif

    sock_desc = (int)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_desc < 0) {
        error_message = "Socket creation failed.";
        return false;
    }
#ifdef DEBUG
    cout << "Socket creation: " << timer.elapsed() << "ms" << endl;
#endif

    memset((char*) & (server_addr), 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (!strcmp(host, "localhost")) {
        host = "127.0.0.1";
    }

    // bind is not required, and port number is dynamic
    if ((server_addr.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE) {
        error_message = "No correct host address.";
        return false;
    }

#if defined(Q_OS_WIN)
    flag = 1;
    ::ioctlsocket(sock_desc, FIONBIO, &flag);
    //    //TODO: check this
    //    ::setsockopt(sock_desc, SOL_SOCKET, SO_REUSEADDR, NULL, 1);


    if (::connect(sock_desc, (const struct sockaddr*) & (server_addr),
                  sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
        int error_number = WSAGetLastError();
        if (error_number != WSAEWOULDBLOCK) {
            disconnect();
            error_message = "Connection to socket failed.";
            return false;
        }

        FD_ZERO(&rmask);
        FD_SET((SOCKET)sock_desc, &rmask);
        wmask = rmask;

        if (::select((int)sock_desc + 1, &rmask, &wmask, NULL, &tv) == 0) {
            disconnect();
            error_message = "Connection to socket failed.";
            return false;
        }
    }
    set_block_mode();

    ::setsockopt(sock_desc, SOL_SOCKET, SO_RCVBUF,
                 (const char*)&recieveBufferSize, sizeof(int));

#ifdef DEBUG
    cout << "Socket connect: " << timer.elapsed() << "ms" << endl;
#endif

#else
    flag = fcntl(sock_desc, F_GETFL, 0);
    fcntl(sock_desc, F_SETFL, flag | O_NONBLOCK);
    //    //TODO: check this
    //    setsockopt(sock_desc, SOL_SOCKET, SO_REUSEADDR, NULL, 1);

    if (::connect(sock_desc, (const struct sockaddr*) & (server_addr),
                  sizeof(struct sockaddr_in)) < 0) {
        if (errno != EINPROGRESS) {
            disconnect();
            error_message = "Connection to socket failed.";
            return false;
        }

        FD_ZERO(&rmask);
        FD_SET(sock_desc, &rmask);
        wmask = rmask;

        if (select(sock_desc + 1, &rmask, &wmask, NULL, &tv) <= 0) {
            disconnect();
            error_message = "Connection to socket failed.";
            return false;
        }

        if (getsockopt(sock_desc, SOL_SOCKET, SO_ERROR, (int*)&sock_optval,
                       (socklen_t*)&sock_optval_size) != 0) {
            disconnect();
            error_message = "Connection to socket failed.";
            return false;
        }

        if (sock_optval != 0) {
            disconnect();
            error_message = "Connection to socket failed.";
            return false;
        }

        set_block_mode();
    }
#endif

    is_connected = true;
    m_device = QString::fromLatin1(host);
    m_baudrate = port;
    clear();
    return true;
    ////////////////////////////////////////////////

}

void TcpDevice::disconnect()
{
    ////////////////To change
    if (sock_desc != Invalid_desc) {
#if defined(Q_OS_WIN)
        ::closesocket(sock_desc);
        //WSACleanup();
#else
        close(sock_desc);
#endif
        sock_desc = Invalid_desc;
        is_connected = false;
    }
    //////////////////////////////////////

    clear();
}

bool TcpDevice::setBaudrate(long b)
{
//    m_baudrate = b;
    return true;
}

long TcpDevice::baudrate() const
{
    return m_baudrate;
}

bool TcpDevice::isConnected() const
{
    return is_connected;
}

int TcpDevice::send(const char* data, size_t count)
{
    /////////////////// To change
    // blocking if data size is larger than system's buffer.
    int sent = ::send(sock_desc, data, count, 0);  //4th arg 0: no flag
    /////////////////////////////////////////

    dataSent(QByteArray(data, sent));

    return sent;
}

int TcpDevice::receive(char* data, size_t count, int timeout)
{
    if (! isConnected()) {
        return -1;
    }
    if (count == 0) {
        return 0;
    }

    size_t filled = 0;

    size_t ring_filled = ring_buffer_.size();
    if (ring_filled < count) {
        updateRingBuffer();
    }

    // バッファにデータがある場合、バッファからデータを格納する
    size_t read_size = qMin(count, ring_buffer_.size());
    ring_buffer_.get(data, read_size);
    filled += read_size;

    // バッファが空の場合、残りのデータはシステムから直接読み込む
    read_size = qMax(0, static_cast<int>(count - filled));
    if (read_size > 0) {
        int n = rawReceive(&data[filled],
                           static_cast<int>(read_size), timeout);
        if (n > 0) {
            filled += n;
        }
    }
    dataReceived(QByteArray(data, filled));
    return static_cast<int>(filled);
}

int TcpDevice::rawReceive(char* data, size_t count, int timeout)
{

#if defined(Q_OS_WIN)
    DWORD tv = timeout;
#else
    struct timeval tv;
    tv.tv_sec = timeout / 1000; // millisecond to seccond
    tv.tv_usec = (timeout % 1000) * 1000; // millisecond to microsecond
#endif
    if (setsockopt(sock_desc, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv,  sizeof(tv))) {
        perror("setsockopt");
    }

    if (count == 0) {
        return 0;
    }

    int result = ::recv(sock_desc, data, count, 0);

    return result;
}

unsigned long TcpDevice::rawLength()
{
    unsigned long length = 4094;
#if defined(Q_OS_WIN)
    ::ioctlsocket(sock_desc, FIONREAD, &length);
#endif
    return length;
}

size_t TcpDevice::size() const
{
//    updateRingBuffer();
    return ring_buffer_.size();
}

void TcpDevice::flush()
{
}

void TcpDevice::clear()
{
    flush();
   ring_buffer_.clear();
}

void TcpDevice::ungetc(const char ch)
{
    if (! isConnected()) {
        return;
    }

    ring_buffer_.ungetc(ch);
}

void TcpDevice::set_block_mode()
{
#if defined(Q_OS_WIN)
    u_long flag = 0;
    ::ioctlsocket(sock_desc, FIONBIO, &flag);
#else
    int flag = 0;
    ::fcntl(sock_desc, F_SETFL, flag);
#endif
}

QString qrk::TcpDevice::getDevice()
{
    return m_device;
}
int TcpDevice::getRecieveBufferSize() const
{
    return recieveBufferSize;
}

void TcpDevice::setRecieveBufferSize(int value)
{
    recieveBufferSize = value;
}

void TcpDevice::updateRingBuffer(void) {
    enum { BufferSize = 65535 };
    char buffer[BufferSize];

    unsigned long maxLength = rawLength();
    if(maxLength > BufferSize) maxLength = BufferSize;

    if(maxLength > 0){
        int n = rawReceive(buffer, maxLength, 1);
        if (n > 0) {
            ring_buffer_.put(buffer, n);
        }
    }
}

