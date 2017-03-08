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

#include "CustomConnection.h"
#include "RingBuffer.h"
#include <iostream>

using namespace qrk;
using namespace std;


struct CustomConnection::pImpl {
    long baudrate_;
    bool connected_;
    RingBuffer<char> recv_buffer_;
    RingBuffer<char> send_buffer_;


    pImpl(void) : baudrate_(0) {
    }
};


CustomConnection::CustomConnection(void) : pimpl(new pImpl)
{
    pimpl->connected_ = false;
}


CustomConnection::~CustomConnection(void)
{
}


const char* CustomConnection::what(void) const
{
    return "Not implemented.";
}


bool CustomConnection::connect(const char* device, long baudrate)
{
    static_cast<void>(device);
    static_cast<void>(baudrate);

    pimpl->connected_ = true;
    cout << "Connect" << endl;
    // !!!
    return true;
}


void CustomConnection::disconnect(void)
{
    // !!!
    pimpl->connected_ = false;
    cout << "disconnect" << endl;
}


bool CustomConnection::setBaudrate(long baudrate)
{
    cout << "setBaudrate: " << baudrate << endl;
    pimpl->baudrate_ = baudrate;
    return true;
}


long CustomConnection::baudrate(void) const
{
    cout << "getBaudrate: " << pimpl->baudrate_ << endl;
    return pimpl->baudrate_;
}


bool CustomConnection::isConnected(void) const
{
    // !!!
    cout << "isConnected: " << pimpl->connected_ << endl;
    return pimpl->connected_;
}


int CustomConnection::send(const char* data, size_t count)
{
    static_cast<void>(data);
    static_cast<void>(count);
    // !!!
    cout << "Send Start -------------" << endl;
    for (int i = 0; i < count; i++) {
        cout << data[i];
    }

    cout << endl;
    cout << "Send End -------------" << endl;

    return count;
}


int CustomConnection::receive(char* data, size_t count, int timeout)
{
    static_cast<void>(timeout);

    int n = std::min(pimpl->recv_buffer_.size(), count);
    pimpl->recv_buffer_.get(data, n);
//    cout << "Receive Start -------------" << endl;
//    cout << "count: " << count << endl;
//    cout << "buffer size: " << pimpl->recv_buffer_.size() << endl;
//    for(int i = 0; i < n; i++){
//        cout << data[i];
//    }

//    cout << endl;
//    cout << "Receive End -------------" << endl;
    return n;
}


size_t CustomConnection::size(void) const
{
    cout << "size: " << pimpl->recv_buffer_.size() << endl;
    return pimpl->recv_buffer_.size();
}


void CustomConnection::flush(void)
{
    // !!!
    cout << "flush" << endl;
}


void CustomConnection::clear(void)
{
    // 登録内容を全てクリアする
    cout << "clear" << endl;
    //return;
    pimpl->recv_buffer_.clear();
    pimpl->send_buffer_.clear();
}


void CustomConnection::ungetc(const char ch)
{
    pimpl->recv_buffer_.ungetc(ch);
    cout << "ungetc: " << ch << endl;
}


void CustomConnection::setReadData(const char* data, size_t count)
{
    pimpl->recv_buffer_.put(data, count);
}


void CustomConnection::setReadData(std::string data)
{
    pimpl->recv_buffer_.put(data.c_str(), data.size());
    cout << "setdata size: " << pimpl->recv_buffer_.size() << endl;

}


void CustomConnection::readSendData(char* data, size_t count)
{
    pimpl->send_buffer_.put(data, count);
}

