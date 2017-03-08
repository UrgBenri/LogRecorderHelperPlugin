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

#include "SerialDevice.h"

#include <iostream>

#include "RingBuffer.h"

using namespace qrk;
using namespace std;


#if defined(Q_OS_WIN)
#include "SerialDevice_win.cpp"   // Windows (win32) 環境
#else
#include "SerialDevice_lin.cpp"   // Linux, Mac 環境 (共通)
#endif


struct SerialDevice::pImpl {
    string error_message_;
    long baudrate_;
    RawSerialDevice raw_;
    RingBuffer<char> ring_buffer_; //!< 受信バッファ


    pImpl(void) : error_message_("no error"), baudrate_(0) {
    }


    void updateRingBuffer(void) {
        enum { BufferSize = 65535 };
        char buffer[BufferSize];

        unsigned long maxLength = raw_.receivedSize();
        if(maxLength > BufferSize) maxLength = BufferSize;

        if(maxLength > 0){
            int n = raw_.receive(buffer, maxLength, 1);
            if (n > 0) {
                ring_buffer_.put(buffer, n);
            }
        }
    }


    int receive(char* data, size_t count, int timeout) {
        if (! isConnected()) {
            error_message_ = "no connection.";
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
            int n = raw_.receive(&data[filled],
                                 static_cast<int>(read_size), timeout);
            if (n > 0) {
                filled += n;
            }
        }
        //dataReceived(QByteArray(data, filled));
        return static_cast<int>(filled);
    }


    bool isConnected(void) {
        return raw_.isConnected();
    }
};


SerialDevice::SerialDevice(void) :
    pimpl(new pImpl)
{

}

SerialDevice::~SerialDevice(void)
{
    disconnect();
}


const char* SerialDevice::what(void) const
{
    return pimpl->error_message_.c_str();
}


bool SerialDevice::connect(const char* device, long baudrate)
{
    disconnect();
    clear();
    if (! pimpl->raw_.connect(device, baudrate)) {
        pimpl->error_message_ = pimpl->raw_.what();
        return false;
    }
    else {
        m_device = QString::fromLatin1(device);
        return true;
    }
}


void SerialDevice::disconnect(void)
{
    pimpl->raw_.disconnect();
    clear();
}


bool SerialDevice::setBaudrate(long baudrate)
{
    if (! pimpl->raw_.setBaudrate(baudrate)) {
        pimpl->error_message_ = pimpl->raw_.what();
        pimpl->baudrate_ = 0;
        return false;
    }
    pimpl->baudrate_ = baudrate;
    return true;
}


long SerialDevice::baudrate(void) const
{
    return pimpl->baudrate_;
}


bool SerialDevice::isConnected(void) const
{
    return pimpl->isConnected();
}


int SerialDevice::send(const char* data, size_t count)
{
    if (! isConnected()) {
        pimpl->error_message_ = "no connection.";
        return 0;
    }

    int n = pimpl->raw_.send(data, static_cast<int>(count));
    if (n < 0) {
        pimpl->error_message_ = pimpl->raw_.what();
    }

//    std::cerr << "sent: " << data << endl;

    dataSent(QByteArray(data, n));
    return n;
}


int SerialDevice::receive(char* data, size_t count, int timeout)
{
    if (! isConnected()) {
        pimpl->error_message_ = "no connection.";
        return 0;
    }

    int received = pimpl->receive(data, count, timeout);
//    std::cerr << (char)data[0];

    dataReceived(QByteArray(data, received));
    return received;
}


size_t SerialDevice::size(void) const
{
    pimpl->updateRingBuffer();
    return pimpl->ring_buffer_.size();
}


void SerialDevice::flush(void)
{
    if (! isConnected()) {
        pimpl->error_message_ = "no connection.";
        return;
    }

    return pimpl->raw_.flush();
}


void SerialDevice::clear(void)
{
    pimpl->raw_.flush();
    pimpl->ring_buffer_.clear();
}


void SerialDevice::ungetc(const char ch)
{
    if (! isConnected()) {
        pimpl->error_message_ = "no connection.";
        return;
    }

    pimpl->ring_buffer_.ungetc(ch);
}

QString SerialDevice::getDevice()
{
    return m_device;
}

