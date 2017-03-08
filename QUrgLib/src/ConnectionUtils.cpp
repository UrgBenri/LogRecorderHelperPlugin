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

#include "ConnectionUtils.h"
#include "Connection.h"

using namespace qrk;


bool qrk::isLF(const char ch)
{
    return ((ch == '\r') || (ch == '\n')) ? true : false;
}


void qrk::skip(Connection* con, int total_timeout, int each_timeout)
{
    if (each_timeout <= 0) {
        each_timeout = total_timeout;
    }

    char recv_ch[4094];
    while (1) {
        int n = readline(con, recv_ch, 4094, each_timeout);
        if (n <= 0) {
            break;
        }
    }
}


int qrk::readline(Connection* con, char* buf, const size_t count, int timeout)
{
    enum{
        ERROR_CODE=-1,
    };
    bool is_timeout = false;
    size_t filled = 0;

    while (filled < count) {
        char recv_ch;
        int n = con->receive(&recv_ch, 1, timeout);
        if (n <= 0) {
            is_timeout = true;
            break;
        }
        else if (isLF(recv_ch)) {
            break;
        }
        buf[filled++] = recv_ch;
    }
    if (filled == count) {
        --filled;
        con->ungetc(buf[filled]);
    }
    buf[filled] = '\0';

    if ((filled == 0) && is_timeout) {
        return ERROR_CODE;
    }
    else {
        return static_cast<int>(filled);
    }
}

