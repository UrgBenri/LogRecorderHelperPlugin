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

#ifndef QRK_SERIAL_DEVICE_H
#define QRK_SERIAL_DEVICE_H

/*!
  \file
  \brief シリアル通信

  \author Satofumi KAMIMURA

  $Id: SerialDevice.h 110 2012-07-31 09:20:08Z kristou $
*/

#include "Connection.h"
#include <memory>

namespace qrk
{
//! シリアル通信クラス
class SerialDevice : public Connection
{
public:
    enum {
        DefaultBaudrate = 115200,
    };
    SerialDevice(void);
    virtual ~SerialDevice(void);

    const char* what(void) const;

    bool connect(const char* device, long baudrate = DefaultBaudrate);
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
    ConnectionType connectionType() {return SERIAL_TYPE;}

    QString getDevice();

private:
    SerialDevice(const SerialDevice &rhs);
    SerialDevice &operator = (const SerialDevice &rhs);

    QString m_device;

    struct pImpl;
    const std::auto_ptr<pImpl> pimpl;
};
}

#endif /* !QRK_SERIAL_DEVICE_H */

