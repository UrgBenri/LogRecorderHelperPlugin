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

#include "isUsingComDriver.h"

#include <QtGlobal>

#ifdef Q_OS_WIN
#include <windows.h>
#include <string>
#endif

using namespace std;


#ifdef Q_OS_WIN
namespace
{
bool existRegValue(HKEY hkey, LPCSTR subkey, const char* find_value)
{
    HKEY next_hkey;
    if (RegOpenKeyExA(hkey, subkey,
                      0, KEY_READ, &next_hkey) != ERROR_SUCCESS) {
        return false;
    }
    enum { MaxLength = 1024 };

    CHAR device[MaxLength + 1];
    char name[MaxLength + 1];

    DWORD ret = ERROR_SUCCESS;
    for (int i = 0; ret == ERROR_SUCCESS; ++i) {
        DWORD dl = MaxLength;
        DWORD nl = MaxLength;
        ret = RegEnumValueA(hkey, i, device, &dl,
                            NULL, NULL, (BYTE*)name, &nl);
        if (ret != ERROR_SUCCESS) {
            break;
        }
        if (! strncmp(name, find_value, nl)) {
            RegCloseKey(next_hkey);
            return true;
        }
    }

    char next_subkey[MaxLength];
    FILETIME filetime;

    ret = ERROR_SUCCESS;
    for (int i = 0; ret == ERROR_SUCCESS; ++i) {
        DWORD dl = MaxLength, nl = MaxLength;
        ret = RegEnumKeyExA(next_hkey, i, next_subkey,
                            &dl, NULL, NULL, &nl, &filetime);
        if (ret != ERROR_SUCCESS) {
            break;
        }

        bool value_exist =
            existRegValue(next_hkey, next_subkey, find_value);
        if (value_exist) {
            RegCloseKey(next_hkey);
            return true;
        }
    }

    RegCloseKey(next_hkey);
    return false;
}
}


bool qrk::isUsingComDriver(const QString &com_port, const QString &driver_name)
{
    QString value_pattern = driver_name + " (" + com_port + ")";
    if (existRegValue(HKEY_LOCAL_MACHINE,
                      "SYSTEM\\CurrentControlSet\\Enum\\USB",
                      value_pattern.toLatin1().constData())) {
        return true;
    }
    return false;
}
#else


bool qrk::isUsingComDriver(const QString &com_port, const QString &driver_name)
{
    Q_UNUSED(com_port);
    Q_UNUSED(driver_name);

    return false;
}
#endif

