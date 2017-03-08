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

#include <QtGlobal>
#include <windows.h>
#include <setupapi.h>
#include <string>
#include <cstdio>

#include "delay.h"

using namespace qrk;

#ifdef Q_CC_MSVC
#define snprintf _snprintf
#endif

using namespace std;

#undef min
#undef max


class RawSerialDevice
{
    string error_message_;
    HANDLE hCom_;
    int current_timeout_;
    string com_name_;


public:
    RawSerialDevice(void)
        : error_message_("no error."), hCom_(INVALID_HANDLE_VALUE),
          current_timeout_(0) {
    }


    const char* what(void) {
        return error_message_.c_str();
    }


    bool connect(const char* device, long baudrate) {
        enum { NameLength = 11 };
        char adjusted_device[NameLength];
        snprintf(adjusted_device, NameLength, "\\\\.\\%s", device);

        hCom_ = CreateFileA(adjusted_device, GENERIC_READ | GENERIC_WRITE, 0,
                                   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hCom_ == INVALID_HANDLE_VALUE) {
            error_message_ = string("open failed: ") + device;
            return false;
        }
        com_name_ = device;

        // 通信サイズの更新
        SetupComm(hCom_, 4096 * 8, 4096);

        // タイムアウト設定
        setTimeout(current_timeout_);

        // ボーレートの変更
        bool ret = setBaudrate(baudrate);
        if (! ret) {
            error_message_ = "fail SerialDevice::setBaudrate()";
        }

        return ret;
    }

    void setTimeout(int timeout) {
        COMMTIMEOUTS timeouts;
        GetCommTimeouts(hCom_, &timeouts);

        timeouts.ReadIntervalTimeout = (timeout == 0) ? MAXDWORD : 0;
        timeouts.ReadTotalTimeoutConstant = timeout;
        timeouts.ReadTotalTimeoutMultiplier = 0;

        SetCommTimeouts(hCom_, &timeouts);
    }


    void disconnect(void) {
        if (hCom_ != INVALID_HANDLE_VALUE) {
            CloseHandle(hCom_);
            hCom_ = INVALID_HANDLE_VALUE;
        }
    }


    bool isConnected(void) {
        return (hCom_ == INVALID_HANDLE_VALUE) ? false : true;
    }

    bool setBaudrate(long baudrate) {
        DCB dcb;
        GetCommState(hCom_, &dcb);
        dcb.BaudRate = baudrate;
        dcb.ByteSize = 8;
        dcb.Parity = NOPARITY;
        dcb.fParity = FALSE;
        dcb.StopBits = ONESTOPBIT;

        //New
        dcb.fBinary = TRUE; //If this member is TRUE, binary mode is enabled. Windows does not support nonbinary mode transfers, so this member must be TRUE.
        dcb.fInX = FALSE;
        dcb.fOutX = FALSE;
        dcb.fAbortOnError = FALSE;
        dcb.fNull = FALSE;
        dcb.fDtrControl = DTR_CONTROL_ENABLE;
        dcb.fRtsControl = RTS_CONTROL_DISABLE;
        if (SetCommState(hCom_, &dcb) == 0) {
            flush();
//            DWORD dw = GetLastError();
            return false;
        }
        else {
            return true;
        }
    }


    int send(const char* data, size_t count) {
        DWORD n;
        WriteFile(hCom_, data, (DWORD)count, &n, NULL);
        return n;
    }

    unsigned long receivedSize(){
        COMSTAT	stat;
        DWORD	dwError;
        ClearCommError(hCom_,&dwError,&stat);

        return stat.cbInQue;
    }

    int receive(char data[], int count, int timeout) {
        if (count <= 0) {
            return 0;
        }

        if (timeout != current_timeout_) {
            setTimeout(timeout);
            current_timeout_ = timeout;
        }

        DWORD n;
        ReadFile(hCom_, data, count, &n, NULL);

        return n;
    }


    void flush(void) {
        PurgeComm(hCom_,
                  PURGE_RXABORT | PURGE_TXABORT |
                  PURGE_RXCLEAR | PURGE_TXCLEAR);
    }
};

