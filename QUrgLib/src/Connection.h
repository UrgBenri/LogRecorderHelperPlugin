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

#ifndef QRK_CONNECTION_H
#define QRK_CONNECTION_H

#include <cstddef>
#include <QString>
#include <QFile>
#include <QMutex>
#include <QQueue>
#include <QCoreApplication>
#include "Thread.h"


namespace qrk
{
enum {
    ReceiveTimeout = -1,
    ErrorLastIndex = -2,
};


class Connection
{
public:
    explicit Connection();
    virtual ~Connection();

    typedef enum {
        SERIAL_TYPE,
        ETHERNET_TYPE,
        CUSTOM_TYPE,
    } ConnectionType;

    virtual const char* what(void) const = 0;

    virtual bool connect(const char* device, long baudrate) = 0;

    virtual void disconnect(void) {}

    virtual bool setBaudrate(long baudrate) = 0;

    virtual long baudrate(void) const = 0;

    virtual bool isConnected(void) const {
        return false;
    }

    virtual int send(const char* data, size_t count) = 0;

    virtual int receive(char* data, size_t count, int timeout) = 0;

    virtual size_t size(void) const = 0;

    virtual void flush(void) = 0;

    virtual void clear(void) = 0;


    virtual void ungetc(const char ch) = 0;

    virtual ConnectionType connectionType() = 0;

    bool startRecording(const QString &locationPart = QCoreApplication::applicationFilePath(),
                        const QString &sendFilePart = "SendFile",
                        const QString &receiveFilePart = "ReceiveFile");
    void stopRecording();

    virtual QString getDevice() = 0;

    static int logThreadProcess(void* args);
    
protected:
    QFile sendDebug;
    QFile receiveDebug;
    QByteArray m_sendingBuffer;
    QByteArray m_receptionBuffer;
    Thread m_log_thread;
    QMutex m_logWriteLock;
    QMutex m_logReadLock;
    bool m_logging;

    void dataSent(const QByteArray &data);
    void dataReceived(const QByteArray &data);

private:
    QString getUniqueLogPath(const QString &path, const QString candidate);
};
}

#endif /* !QRK_CONNECTION_H */

