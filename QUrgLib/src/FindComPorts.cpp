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

#include "FindComPorts.h"
#include <QtGlobal>
#include <QFileInfo>
#include <QStringList>
#include <QtDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#include <setupapi.h>
#include <algorithm>

#if defined(Q_CC_MSVC)
#pragma comment(lib, "setupapi.lib")
#endif

#else
#include <algorithm>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#endif

using namespace qrk;
using namespace std;


struct FindComPorts::pImpl {
    QStringList base_names_;
    QStringList driver_names_;


    pImpl(void) {
        // Windows はレジストリ情報だけから COM 一覧の探索を行う
        // よって、以降の設定は Linux, MacOS のみで使われる
#if defined(Q_OS_LINUX)
        base_names_.push_back("/dev/ttyUSB");
        base_names_.push_back("/dev/usb/ttyUSB");
#elif defined(Q_OS_MACX)
        base_names_.push_back("/dev/tty.usbmodem");
#endif
    }


    void addBaseName(const QString &base_name) {
        base_names_.insert(base_names_.begin(), base_name);
    }


    void addDriverName(const QString &driver_name) {
        driver_names_.push_back(driver_name);
    }


    void orderByDriver(QStringList &ports, QStringList &drivers,
                       const QString &driver_name,
                       bool all_ports) {
        if (ports.isEmpty()) {
            return;
        }

        if (driver_name.contains(driver_name)) {
            ports.push_front(ports.back());
            ports.pop_back();

            drivers.push_front(drivers.back());
            drivers.pop_back();

        }
        else {
            if (! all_ports) {
                ports.pop_back();
                drivers.pop_back();
            }
        }
    }


    void addFoundPorts(QStringList &found_ports,
                       const QString &port) {
        for (QStringList::iterator it = found_ports.begin();
                it != found_ports.end(); ++it) {

            // !!! データ構造を map にすべきかも
            // !!! 検索のアルゴリズムを用いるべき

            if (! port.compare(*it)) {
                return;
            }
        }
        found_ports.push_back(port);
    }
};


FindComPorts::FindComPorts(void) : pimpl(new pImpl)
{
}


FindComPorts::~FindComPorts(void)
{
}


void FindComPorts::clearBaseNames(void)
{
    pimpl->base_names_.clear();
}


void FindComPorts::addBaseName(const QString &base_name)
{
    pimpl->addBaseName(base_name);
}


QStringList FindComPorts::baseNames(void)
{
    return pimpl->base_names_;
}


void FindComPorts::addDriverName(const QString &driver_name)
{
    pimpl->addDriverName(driver_name);
}


#ifdef Q_OS_WIN
// Windows の場合
size_t FindComPorts::find(QStringList &ports,
                          QStringList &driver_names,
                          bool all_ports)
{
    // デバイスマネージャの一覧から COM デバイスを探す
    QStringList found_ports;
    QStringList found_drivers;

    //4D36E978-E325-11CE-BFC1-08002BE10318
    GUID GUID_DEVINTERFACE_COM_DEVICE = {
        0x4D36E978, 0xE325, 0x11CE,
        {0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 }
    };

    HDEVINFO hdi = SetupDiGetClassDevs(&GUID_DEVINTERFACE_COM_DEVICE, 0, 0,
                                       DIGCF_PRESENT /*| DIGCF_INTERFACEDEVICE*/);
    if (hdi == INVALID_HANDLE_VALUE) {
        return 0;
    }

    SP_DEVINFO_DATA sDevInfo;
    sDevInfo.cbSize = sizeof(SP_DEVINFO_DATA);
    for (DWORD i = 0; SetupDiEnumDeviceInfo(hdi, i, &sDevInfo); ++i) {

        enum { BufferSize = 256 };
        char buffer[BufferSize + 1];
        DWORD dwRegType;
        DWORD dwSize;

        // フレンドリーネームを取得して COM 番号を取り出す
        SetupDiGetDeviceRegistryPropertyA(hdi, &sDevInfo, SPDRP_FRIENDLYNAME,
                                          &dwRegType, (BYTE*)buffer, BufferSize,
                                          &dwSize);
        enum { ComNameLengthMax = 7 };
        size_t n = strlen(buffer);
        if (n < ComNameLengthMax) {
            // COM 名が短過ぎた場合、処理しない
            // 問題がある場合は、修正する
            continue;
        }

        // (COMx) の最後の括弧を探す
        char* p = strrchr(buffer, ')');
        if (p) {
            *p = '\0';
        }

        // COM と番号までの文字列を抜き出す
        p = strstr(&buffer[n - ComNameLengthMax], "COM");
        if (! p) {
            continue;
        }
        found_ports.push_back(QString(p));


        // デバイス名を取得し、IsUsbCom の条件に一致したら、先頭に配置する
        SetupDiGetDeviceRegistryPropertyA(hdi, &sDevInfo, SPDRP_DEVICEDESC,
                                          &dwRegType, (BYTE*)buffer, BufferSize,
                                          &dwSize);
        found_drivers.push_back(buffer);
        pimpl->orderByDriver(found_ports, found_drivers, QString(buffer), all_ports);
    }

    SetupDiDestroyDeviceInfoList(hdi);

    ports << found_ports;
    driver_names << found_drivers;

    return ports.size();
}


#else

namespace
{
void searchFiles(QStringList &ports, const QString &dir_name)
{
    DIR* dp = opendir(dir_name.toLatin1().constData());
    if (! dp) {
        return;
    }

    struct dirent* dir;
    while ((dir = readdir(dp))) {
        struct stat state;
        stat(dir->d_name, &state);
        if (S_ISDIR(state.st_mode)) {
            QString file = dir_name + dir->d_name;
            ports.push_back(file);
        }
    }
}
}


// Linux, Mac の場合
size_t FindComPorts::find(QStringList &ports,
                          QStringList &driver_names,
                          bool all_ports)
{
    static_cast<void>(all_ports);
    static_cast<void>(driver_names);

    QStringList found_ports;

    // 登録ベース名毎に、ファイル名がないかの探索を行う
    for (QStringList::iterator it = pimpl->base_names_.begin();
            it != pimpl->base_names_.end(); ++it) {

        QFileInfo fi(*it);

        QString dir_name = fi.absolutePath() + "/";
        QStringList ports;
        searchFiles(ports, dir_name);

        for (QStringList::iterator fit = ports.begin();
                fit != ports.end(); ++fit) {
            if (fit->startsWith(*it)) {
                // マッチしたら、登録を行う
                pimpl->addFoundPorts(found_ports, *fit);
            }
        }
    }

    for (QStringList::iterator it = found_ports.begin();
            it != found_ports.end(); ++it) {
        ports.push_back(*it);
        driver_names.push_back("");
    }

    return found_ports.size();
}
#endif


size_t FindComPorts::find(QStringList &ports, bool all_ports)
{
    QStringList driver_names_dummy;
    return find(ports, driver_names_dummy, all_ports);
}

