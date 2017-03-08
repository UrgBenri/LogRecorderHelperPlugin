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

#ifndef QRK_CUSTOM_CONNECTION_H
#define QRK_CUSTOM_CONNECTION_H

/*!
  \file
  \brief 送受信データを任意に設定できる接続クラス

  \author Satofumi KAMIMURA

  $Id: CustomConnection.h 146 2012-09-27 23:30:56Z kristou $
*/

#include "Connection.h"
#include <string>
#include <memory>


namespace qrk
{
//! 送受信データを任意に設定できる接続クラス
class CustomConnection : public Connection
{
public:
    enum { NoTimeout = -1 };

    CustomConnection(void);
    ~CustomConnection(void);

    const char* what(void) const;

    bool connect(const char* device, long baudrate);
    void disconnect(void);
    bool setBaudrate(long baudrate);
    long baudrate(void) const;
    bool isConnected(void) const;
    int send(const char* data, size_t count);
    int receive(char* data, size_t count, int timeout);
    size_t size(void) const;
    void flush(void);
    void clear(void);
    void ungetc(const char ch);

    void setReadData(const char* data, size_t count);
    void setReadData(std::string data);
    void readSendData(char* data, size_t count);

    ConnectionType connectionType() { return CUSTOM_TYPE;}

private:
    CustomConnection(const CustomConnection &rhs);
    CustomConnection &operator = (const CustomConnection &rhs);

    struct pImpl;
    std::auto_ptr<pImpl> pimpl;
};
}

#endif /* !QRK_CUSTOM_CONNECTION_H */

