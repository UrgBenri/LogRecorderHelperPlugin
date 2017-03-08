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

#include "UrgUsbCom.h"
#include "isUsingComDriver.h"

using namespace qrk;
using namespace std;


UrgUsbCom::UrgUsbCom(void)
{
}

QVector<string> UrgUsbCom::baseNames(void)
{
    QVector<string> ports;
#if defined(Q_OS_LIN)
    ports.push_back("/dev/ttyACM");

#elif defined(Q_OS_MACX)
    ports.push_back("/dev/tty.usbmodem");
    ports.push_back("/dev/tty.usbserial");
#endif
    return ports;
}


bool UrgUsbCom::isUsbCom(const QString &com_port)
{
    // "URG Series USB Device Driver (COMx)" や
    // "URG-X002 USB Device Driver (COMx)" が Value に含まれているか検索
    if (isUsingComDriver(com_port, "URG Series USB Device Driver") ||
            isUsingComDriver(com_port, "URG-X002 USB Device Driver")) {
        return true;

    }
    else {
        return false;
    }
}

