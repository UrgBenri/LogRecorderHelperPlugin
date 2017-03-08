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

#include "Connection.h"

#include <QFileInfo>
#include <QDebug>
#include <QDateTime>
#include <QThread>

using namespace qrk;

Connection::Connection():
    m_logging(false),
    m_log_thread(&logThreadProcess, this)
{

}

Connection::~Connection()
{
    stopRecording();
}

bool Connection::startRecording(const QString &locationPart, const QString &sendFilePart, const QString &receiveFilePart)
{
    stopRecording();
    QString sendFilename = getUniqueLogPath(locationPart, sendFilePart);
    QString receiveFilename = getUniqueLogPath(locationPart, receiveFilePart);
    
    sendDebug.setFileName(sendFilename);
    receiveDebug.setFileName(receiveFilename);
    
    if (!sendDebug.open(QIODevice::WriteOnly)) {
        //            pimpl->error_message_ = "Sending debug file could not be created";
        return false;
    }
    
    if (!receiveDebug.open(QIODevice::WriteOnly)) {
        //            pimpl->error_message_ = "Receiving debug file could not be created";
        return false;
    }

    m_logReadLock.lock();
    m_receptionBuffer.clear();
    m_logReadLock.unlock();

    m_logWriteLock.lock();
    m_sendingBuffer.clear();
    m_logWriteLock.unlock();

    if(!m_log_thread.isRunning()){
        m_log_thread.run();
        m_logging = true;
    }
    
    return true;
}

void Connection::stopRecording()
{
    if(m_log_thread.isRunning()){
        m_logging = false;
        m_log_thread.stop();
    }

    if(sendDebug.isOpen()){
        sendDebug.close();
    }
    
    if(receiveDebug.isOpen()){
        receiveDebug.close();
    }

    m_logReadLock.lock();
    m_receptionBuffer.clear();
    m_logReadLock.unlock();


    m_logWriteLock.lock();
    m_sendingBuffer.clear();
    m_logWriteLock.unlock();
}

void Connection::dataSent(const QByteArray &data)
{
    if(m_logging){
        m_logWriteLock.lock();
        m_sendingBuffer += data;
        m_logWriteLock.unlock();
    }
}

void Connection::dataReceived(const QByteArray &data)
{
    if(m_logging){
        m_logReadLock.lock();
        m_receptionBuffer += data;
        m_logReadLock.unlock();
    }
}

QString Connection::getUniqueLogPath(const QString &path, const QString candidate)
{
    QFileInfo appFi(path);
    QString absPath = appFi.absoluteFilePath();
    
    QString filename = absPath + "/" + candidate + "_" +
            QDateTime::currentDateTime().toString("yyyy_MM_dd_HH_mm_ss_zzz") +
            ".log";
    QFileInfo sendFi(filename);
    int count = 0;
    while (sendFi.exists()) {
        filename = absPath + "/" + candidate + "_" + QString::number(count) + ".log";
        sendFi.setFile(filename);
        count++;
    }
    
    return filename;
}

int Connection::logThreadProcess(void *args)
{
    Connection* obj = static_cast<Connection*>(args);
    while (obj->receiveDebug.isOpen() &&
           obj->sendDebug.isOpen()) {

        bool noData = true;
        bool sendingEmpty = obj->m_sendingBuffer.isEmpty();
        bool receptionEmpty = obj->m_receptionBuffer.isEmpty();
        if(obj->m_log_thread.exitThread && sendingEmpty && receptionEmpty) break;
        if(!obj->m_log_thread.exitThread){
            sendingEmpty = obj->m_sendingBuffer.size() < 1024;
            receptionEmpty = obj->m_receptionBuffer.size() < 1024;
        }

        if(!sendingEmpty){
            noData = false;
            obj->m_logWriteLock.lock();
            QByteArray data = obj->m_sendingBuffer;
             obj->m_sendingBuffer.clear();
            obj->m_logWriteLock.unlock();
            obj->sendDebug.write(data);
        }
        if(!receptionEmpty){
            noData = false;
            obj->m_logReadLock.lock();
            QByteArray data = obj->m_receptionBuffer;
            obj->m_receptionBuffer.clear();
            obj->m_logReadLock.unlock();
            obj->receiveDebug.write(data);
        }

        if(noData){
            QThread::msleep(5);
        }
    }

    return 0;
}

