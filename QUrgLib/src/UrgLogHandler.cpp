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

#include "UrgLogHandler.h"
#include <fstream>
#include <cstdlib>
//#include <sys/time.h>
#include <time.h>
#include <QDateTime>

#include "delay.h"

#include <QFileInfo>
#include <QStringList>
#include <QTextStream>
#include <QApplication>

//#include <iostream>
#include <QDebug>

using namespace qrk;
using namespace std;


namespace
{
enum {
    InvalidTimestamp = -1,
};
}

UrgLogHandler::UrgLogHandler(void)
{
    m_lastTimestamp = 0;
    m_timeStamp = 0;
    m_cached_timestamp = InvalidTimestamp;
    m_captureMode = Unknown_Capture_mode;
    m_captureModeRead = Unknown_Capture_mode;
    blockSeparator = ";";
    dataSeparator = "&";
    intensitySeparator = "|";
    m_errorMessage = tr("No errors");
    m_useFlush = false;
    m_currentMode = UnknownMode;

    frontStep = 0;
    totalSteps = 0;
    minDistance = 0;
    maxDistance = 0;
    motorSpeed = 0;
    grouping = 1;
    scanMsec = 0;

    m_readPosition = 0;
    m_writePosition = 0;

    startStep = 0;
    endStep = 0;
    m_startStepRead = 0;
    m_endStepRead = 0;

    m_totalTimestamps = 0;
    m_maxEchoNumber = 3;
    m_isClosed = false;

    appVersion = "No version";
    model = "No model";
    serialNumber = "No serial number";

    applicationNameKey = "[appName]";
    applicationVersionKey = "[appVersion]";
    captureModeKey = "[captureMode]";
    modelKey = "[model]";
    serialNumberKey = "[serialNumber]";
    firmwareVersionKey = "[firmwareVersion]";
    startStepKey = "[startStep]";
    endStepKey = "[endStep]";
    groupingKey = "[grouping]";
    frontStepKey = "[frontStep]";
    totalStepsKey = "[totalSteps]";
    minDistanceKey = "[minDistance]";
    maxDistanceKey = "[maxDistance]";
    motorSpeedKey = "[motorSpeed]";
    scanMsecKey = "[scanMsec]";

    timestampKey = "[timestamp]";

    scanKey = "[scan]";
    logtimeKey = "[logtime]";

    m_shouldStopInit = false;

    m_markPoints.clear();
}


UrgLogHandler::~UrgLogHandler(void)
{
    close();
}


bool UrgLogHandler::create(const QString &file_name)
{

    m_filename = file_name;
    QFileInfo fi(m_filename);
    m_logFormat = fi.suffix().toLower();
    m_firstTime = true;

    if (m_logFormat == "ubh") {

        m_sout.setFileName(m_filename);

        if (! m_sout.open(QIODevice::WriteOnly | QIODevice::Text)) {
            m_errorMessage = tr("File could not be created.");

            m_isClosed = true;
            return false;
        }

        add(applicationNameKey, QApplication::applicationName());
        add(applicationVersionKey, QApplication::applicationVersion());
    }
    else if (m_logFormat == "xls") {
//        m_excel.New(m_maxEchoNumber + 1);
//        m_excel.RenameWorksheet("Sheet1", "Info");
//        for (int i = 0; i < m_maxEchoNumber; ++i) {
//            QString sheet("Sheet");
//            sheet += QString::number(i + 2);
//            QString echo("Echo ");
//            echo += QString::number(i + 1);
//            m_excel.RenameWorksheet(sheet.toLatin1().constData(),
//                                  echo.toLatin1().constData());
//        }

        m_excel.New((m_maxEchoNumber * 2) + 1);
        m_excel.RenameWorksheet("Sheet1", "Info");
        for (int i = 0; i < m_maxEchoNumber; ++i)
        {
            QString sheet("Sheet");
            sheet += QString::number(i + 2);
            QString echo("Echo Distance");
            echo += QString::number(i + 1);
            m_excel.RenameWorksheet(sheet.toLatin1().constData(),
                                  echo.toLatin1().constData());
        }
        for (int i = 0; i < m_maxEchoNumber; ++i)
        {
            QString sheet("Sheet");
            sheet += QString::number(m_maxEchoNumber + i + 2);
            QString echo("Echo Intensity");
            echo += QString::number(i + 1);
            m_excel.RenameWorksheet(sheet.toLatin1().constData(),
                                  echo.toLatin1().constData());
        }
    }
    else if (m_logFormat == "csv")
    {
        m_sout.setFileName(m_filename);

        if (! m_sout.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            m_errorMessage = tr("File could not be created.");
            return false;
        }
    }
    else if (m_logFormat == "xy")
    {
        m_sout.setFileName(m_filename);
        if (! m_sout.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            m_errorMessage = tr("File could not be created.");
            return false;
        }
    }
    else{
        m_errorMessage = tr("File format not supported.");
        return false;
    }

    m_currentMode = WriteMode;
    m_isClosed = false;
    m_writePosition = 0;
    return true;
}

bool UrgLogHandler::load(const QString &file_name)
{
    m_filename = file_name;
    QFileInfo fi(m_filename);
    m_logFormat = fi.suffix().toLower();
    m_isClosed = false;
    m_currentMode = ReadMode;

    if (m_logFormat == "ubh") {
        m_sin.setFileName(m_filename);

        if (m_sin.open(QIODevice::ReadOnly | QIODevice::Text)) {
            initHeaderRecords();
        }
        else{
            m_errorMessage = tr("File could not be opened.");
            m_isClosed = true;
            return false;
        }
    }
    else{
        m_errorMessage = tr("File format not supported.");
        m_isClosed = true;
        return false;
    }

    m_errorMessage = tr("No errors.");
    return true;
}



int UrgLogHandler::readIntFromHeader(const QString &key)
{
    QString line = readStringFromHeader(key);
    if(!line.isEmpty()){
        return line.toInt();
    }
    return -1;
}

long UrgLogHandler::readLongFromHeader(const QString &key)
{
    QString line = readStringFromHeader(key);
    if(!line.isEmpty()){
        return line.toLong();
    }
    return -1;
}

QString UrgLogHandler::readStringFromHeader(const QString &key)
{
    for(int i = 0; i < m_header.size() -1; ++i){
        QString line = m_header[i];
        if (line.startsWith(key)) {
            return m_header[i +1];
        }
    }
    m_errorMessage = tr("Key %1 does not exist.").arg(key);
    return "";
}

QStringList UrgLogHandler::supportedModes()
{
    RangeCaptureMode capture = getCaptureMode();
    switch (capture) {
    case GD_Capture_mode:
        return QStringList() << "GD" << "MD";
        break;
    case MD_Capture_mode:
        return QStringList() << "GD" << "MD";
        break;
    case GE_Capture_mode:
        return QStringList() << "GD" << "MD" << "GE" << "ME";
        break;
    case ME_Capture_mode:
        return QStringList() << "GD" << "MD" << "GE" << "ME";
        break;
    case HD_Capture_mode:
        return QStringList() << "GD" << "MD" << "HD" << "ND";
        break;
    case HE_Capture_mode:
        return QStringList() << "GD" << "MD" << "GE" << "ME" << "HD" << "HE"  << "ND" << "NE";
        break;
    case ND_Capture_mode:
        return QStringList() << "GD" << "MD" << "HD" << "ND";
        break;
    case NE_Capture_mode:
        return QStringList() << "GD" << "MD" << "GE" << "ME" << "HD" << "HE"  << "ND" << "NE";
        break;
    default:
        return QStringList();
        break;
    }
}


bool UrgLogHandler::isOpen()
{
    if (m_currentMode == ReadMode) {
        return m_sin.isOpen();
    }
    if (m_currentMode == WriteMode) {
        if (m_logFormat == "xls") {
            return !m_isClosed;
        }
        else{
            return m_sout.isOpen();
        }
    }

    return false;
}

RangeCaptureMode UrgLogHandler::readCaptureMode(const QString &key)
{
    QString line = readStringFromHeader(key);

    if (line.startsWith("GD_Capture_mode")) {
        return GD_Capture_mode;
    }

    if (line.startsWith("MD_Capture_mode")) {
        return MD_Capture_mode;
    }

    if (line.startsWith("GE_Capture_mode")) {
        return GE_Capture_mode;
    }

    if (line.startsWith("ME_Capture_mode")) {
        return ME_Capture_mode;
    }

    if (line.startsWith("HD_Capture_mode")) {
        return HD_Capture_mode;
    }

    if (line.startsWith("ND_Capture_mode")) {
        return ND_Capture_mode;
    }

    if (line.startsWith("HE_Capture_mode")) {
        return HE_Capture_mode;
    }

    if (line.startsWith("NE_Capture_mode")) {
        return NE_Capture_mode;
    }

    m_errorMessage = tr("Key %1 does not exist.").arg(key);
    return Unknown_Capture_mode;
}

int UrgLogHandler::guessStartStep(const QString &model)
{
    //classic-URG
    if (model.contains("URG-")) {
        return 44;
    }

    //Rapid-URG
    if (model.contains("UBG-")) {
        return 44;
    }

    //Hi-URG
    if (model.contains("UHG-")) {
        return 0;
    }

    //Top-URG
    if (model.contains("UTM-")) {
        return 0;
    }

    //Tough-URG
    if (model.contains("UXM-")) {
        return 0;
    }

    return -1;
}

int UrgLogHandler::guessEndStep(const QString &model)
{
    //classic-URG
    if (model.contains("URG-")) {
        return 725;
    }

    //Rapid-URG
    if (model.contains("UBG-")) {
        return 725;
    }

    //Hi-URG
    if (model.contains("UHG-")) {
        return 768;
    }

    //Top-URG
    if (model.contains("UTM-")) {
        return 1080;
    }

    //Tough-URG
    if (model.contains("UXM-")) {
        return 760;
    }
    return -1;
}

int UrgLogHandler::guessGrouping(const QString &model)
{
    Q_UNUSED(model);
    return 1;
}


int UrgLogHandler::guessFrontStep(const QString &model)
{
    //classic-URG
    if (model.contains("URG-")) {
        return 384;
    }

    //Rapid-URG
    if (model.contains("UBG-")) {
        return 384;
    }

    //Hi-URG
    if (model.contains("UHG-")) {
        return 384;
    }

    //Top-URG
    if (model.contains("UTM-")) {
        return 540;
    }

    //Tough-URG
    if (model.contains("UXM-")) {
        return 380;
    }

    return -1;
}

int UrgLogHandler::guessTotalSteps(const QString &model)
{
    //classic-URG
    if (model.contains("URG-")) {
        return 1024;
    }

    //Rapid-URG
    if (model.contains("UBG-")) {
        return 1024;
    }

    //Hi-URG
    if (model.contains("UHG-")) {
        return 1024;
    }

    //Top-URG
    if (model.contains("UTM-")) {
        return 1440;
    }

    //Tough-URG
    if (model.contains("UXM-")) {
        return 1440;
    }

    return -1;
}

int UrgLogHandler::guessMinDistance(const QString &model)
{
    //classic-URG
    if (model.contains("URG-")) {
        return 60;
    }

    //Rapid-URG
    if (model.contains("UBG-")) {
        return 20;
    }

    //Hi-URG
    if (model.contains("UHG-")) {
        return 100;
    }

    //Top-URG
    if (model.contains("UTM-")) {
        return 23;
    }

    //Tough-URG
    if (model.contains("UXM-")) {
        return 23;
    }

    return -1;
}

int UrgLogHandler::guessMaxDistance(const QString &model)
{
    //classic-URG
    if (model.contains("URG-")) {
        return 4095;
    }

    //Rapid-URG
    if (model.contains("UBG-")) {
        return 5600;
    }

    //Hi-URG
    if (model.contains("UHG-")) {
        return 8000;
    }

    //Top-URG
    if (model.contains("UTM-")) {
        return 60000;
    }

    //Tough-URG
    if (model.contains("UXM-")) {
        return 100000;
    }

    return -1;
}

int UrgLogHandler::guessScanMsec(const QString &model)
{
    //classic-URG
    if (model.contains("URG-")) {
        return 100;
    }

    //Rapid-URG
    if (model.contains("UBG-")) {
        return 27;
    }
    if (model.contains("UBG-05LX-S01")) {
        return 100;
    }
    if (model.contains("UBG-04LX-F01")) {
        return 28;
    }

    //Hi-URG
    if (model.contains("UHG-")) {
        return 67;
    }

    //Top-URG
    if (model.contains("UTM-")) {
        return 25;
    }

    //Tough-URG
    if (model.contains("UXM-")) {
        return 50;
    }

    return -1;
}

int UrgLogHandler::headerCheck()
{
    int returnValue = 0;

    appName = readStringFromHeader(applicationNameKey);
    if (appName.isEmpty()) {
        appName = "UrgBenri"; // Assume the latest version before introduction
    }

    appVersion = readStringFromHeader(applicationVersionKey);
    if (appVersion.isEmpty()) {
        appVersion = "1.2.5"; // Assume the latest version before introduction
    }

    model = readStringFromHeader(modelKey);
    if (model.isEmpty()) {
        returnValue = -1;
    }

    serialNumber = readStringFromHeader(serialNumberKey);
    if (serialNumber.isEmpty()) {
        returnValue = -2;
    }

    firmwareVersion = readStringFromHeader(firmwareVersionKey);
    if (firmwareVersion.isEmpty()) {
        returnValue = -2;
    }

    startStep = m_startStepRead = readIntFromHeader(startStepKey);
    if (startStep < 0) {
        startStep = m_startStepRead = guessStartStep(model);
        returnValue = -3;
    }

    endStep = m_endStepRead = readIntFromHeader(endStepKey);
    if (endStep < 0) {
        endStep = m_endStepRead = guessEndStep(model);
        returnValue = -4;
    }

    grouping = readIntFromHeader(groupingKey);
    if (grouping < 0) {
        grouping = guessGrouping(model);
        returnValue = -4;
    }

    frontStep = readIntFromHeader(frontStepKey);
    if (frontStep < 0) {
        frontStep = guessFrontStep(model);
        returnValue = -5;
    }

    totalSteps = readIntFromHeader(totalStepsKey);
    if (totalSteps < 0) {
        totalSteps = guessTotalSteps(model);
        returnValue = -6;
    }

    minDistance = readLongFromHeader(minDistanceKey);
    if (minDistance < 0) {
        minDistance = guessMinDistance(model);
        returnValue = -7;
    }

    maxDistance = readLongFromHeader(maxDistanceKey);
    if (maxDistance < 0) {
        maxDistance = guessMaxDistance(model);
        returnValue = -8;
    }

    motorSpeed = readIntFromHeader(motorSpeedKey);
    if (motorSpeed < 0) {
        motorSpeed = 2400;
        returnValue = -9;
    }

    scanMsec = readIntFromHeader(scanMsecKey);
    if (scanMsec < 0) {
        scanMsec = guessScanMsec(model);
        returnValue = -10;
    }

    m_captureMode = m_captureModeRead = readCaptureMode(captureModeKey);
    if (m_captureMode == Unknown_Capture_mode) {
        returnValue = -11;
    }

    return returnValue;
}

bool UrgLogHandler::timestampSequential(long &totalTimestamp, long &skipTimestamp)
{
    QString line;
    long timestamp;
    long last_timestamp = 0;

    m_sin.reset();
    m_sin.seek(0);

    totalTimestamp = 0;
    skipTimestamp = 0;

    int scanThres = (scanMsec * 1.25);

    m_markPoints.clear();

    m_markPoints.push_back(0);

    bool result = true;
    m_shouldStopInit = false;

    while (!m_sin.atEnd() && !m_shouldStopInit) {
        line = m_sin.readLine();
        if (line.startsWith(timestampKey)) {
            qint64 currentPos = m_sin.pos() - (qint64)(timestampKey.length() + 5);
            m_markPoints.push_back(currentPos);
            if (!m_sin.atEnd()) {
                line = m_sin.readLine();
                if (!line.isEmpty()) {
                    timestamp = line.toLong();
                    totalTimestamp++;

                    if (last_timestamp >= timestamp) {
                        m_errorMessage = tr("Non sequential timestamp.");
                        result = false;
                    }

                    if (last_timestamp == 0) {
                        last_timestamp = timestamp;
                    }

                    if ((timestamp - last_timestamp) > scanThres) {
                        skipTimestamp++;
                    }

                    last_timestamp = timestamp;
                }
                else {
                    m_errorMessage = tr("An empty timestamp is found.");
                    result = false;
                }
            }
            else {

            }
        }
    }

    totalTimestamp--;

    m_totalTimestamps = totalTimestamp;

    return result;
}

long UrgLogHandler::getSkippedTimeStamps()
{
    long skipTimestamp = 0;
    if (m_markPoints.size() > 0) {
        int scanThres = (scanMsec * 1.25);
        long last_timestamp = timestampAt(0);

        for (int i = 1; i < m_markPoints.size(); ++i) {
            long current_timetsamp = timestampAt(i);
            if ((current_timetsamp - last_timestamp) > scanThres) {
                skipTimestamp++;
            }

            last_timestamp = current_timetsamp;
        }
    }

    m_sin.reset();
    m_sin.seek(0);

    return skipTimestamp;
}

long UrgLogHandler::timestampAt(qint64 pos)
{
    long timestamp = 0;
    if(setDataPos(pos) == pos){
        if(getTimestamp(timestamp) <0){
            timestamp = 0;
        }
    }
    return timestamp;
}


bool UrgLogHandler::init(bool noFreeze)
{
    if (!m_sin.isOpen()) {
        m_errorMessage = tr("Log file is not open.");
        return false;
    }

    bool result = true;

    getDataInit();

    QString line;

    qint64 size = m_sin.size();

    m_sin.reset();
    m_sin.seek(0);

    m_markPoints.clear();
    qint64 avgCount =  size / 10419;
    m_markPoints.reserve(avgCount);


    m_shouldStopInit = false;
    qint64 totalTimestamp = 0;

    while (!m_sin.atEnd() && !m_shouldStopInit) {
        line = m_sin.readLine();
        qint64 currentPos = m_sin.pos() - (qint64)(timestampKey.length() + 5);
        if (line.startsWith(timestampKey)) {
            m_markPoints.push_back(currentPos);
            ++totalTimestamp;
            if (!m_sin.atEnd()) {
                line = m_sin.readLine();
                if (line.isEmpty()) {
                    m_errorMessage = tr("An empty timestamp is found.");
                    result &= false;
                }
            }
            else {
                m_errorMessage = tr("End of file reached before getting the timestamp value.");
                result &= false;
            }

        }
        if (size > 0) {
            int progress = static_cast<int>(((double)currentPos / (double)size) * 100.0);
            emit initProgress(progress);
        }
        else {
            emit initProgress(0);
        }

        if (noFreeze) {
            QApplication::processEvents();
        }
    }

    if (m_shouldStopInit) {
        m_markPoints.clear();
        m_markPoints.push_back(0);
        result &= false;
        m_errorMessage = tr("Initialization canceled.");
    }

    m_totalTimestamps = totalTimestamp;
    emit initProgress(100);

    m_sin.reset();
    m_sin.seek(0);

    return result;
}

long UrgLogHandler::getTotalTimestamps()
{
    return m_totalTimestamps;
}

void UrgLogHandler::setCaptureMode(RangeCaptureMode mode)
{
    if (m_currentMode == ReadMode) {
        switch (m_captureMode) {
        case GD_Capture_mode: {
            if (mode == GD_Capture_mode) {
                m_captureModeRead = mode;
            }
        }
        break;
        case MD_Capture_mode: {
            if (mode == MD_Capture_mode || mode == GD_Capture_mode) {
                m_captureModeRead = mode;
            }
        }
        break;
        case GE_Capture_mode: {
            if (mode == GD_Capture_mode || mode == GE_Capture_mode) {
                m_captureModeRead = mode;
            }
        }
        break;
        case ME_Capture_mode: {
            if (mode == GD_Capture_mode || mode == GE_Capture_mode ||
                    mode == MD_Capture_mode || mode == ME_Capture_mode) {
                m_captureModeRead = mode;
            }
        }
        break;
        case HD_Capture_mode: {
            if (mode == HD_Capture_mode || mode == MD_Capture_mode ||
                    mode == GD_Capture_mode) {
                m_captureModeRead = mode;
            }
        }
        break;
        case ND_Capture_mode: {
            if (mode == ND_Capture_mode || mode == HD_Capture_mode ||
                    mode == MD_Capture_mode || mode == GD_Capture_mode) {
                m_captureModeRead = mode;
            }
        }
        break;
        case HE_Capture_mode: {
            if (mode == HD_Capture_mode || mode == HE_Capture_mode ||
                    mode == GD_Capture_mode || mode == GE_Capture_mode ||
                    mode == MD_Capture_mode || mode == ME_Capture_mode) {
                m_captureModeRead = mode;
            }
        }
        break;
        case NE_Capture_mode: {
            if (mode == ND_Capture_mode || mode == NE_Capture_mode ||
                    mode == HD_Capture_mode || mode == HE_Capture_mode ||
                    mode == GD_Capture_mode || mode == GE_Capture_mode ||
                    mode == MD_Capture_mode || mode == ME_Capture_mode) {
                m_captureModeRead = mode;
            }
        }
        default: {
            m_captureModeRead = m_captureMode;
        }
        }
    }

    if (m_currentMode == WriteMode) {
        m_captureMode = mode;
    }

}

//bool UrgLogHandler::getDataFromCSV(const QString &source,
//                                   QVector<QVector<long> > &ranges,
//                                   QVector<QVector<long> > &levels,
//                                   int distanceNumber = 3,
//                                   int intensityNumber = 3)
//{
//    data.clear();
//    intensity.clear();

//    QFileInfo fileInfo(source);
//    if (!fileInfo.exists()) {
//        error_message = tr("File does not exist.");
//        return false;
//    }

//    QFile file(source);

//    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//        int scanNumber = 0;

//        if (!file.atEnd()) {
//            QString header = file.readLine();
//            QStringList strings = header.split(",");

//            if ((strings.size() % (distanceNumber + intensityNumber)) != 1) {
//                error_message = tr("File format does not fit 6 culums format.");
//                return false;
//            }

//            scanNumber = strings.size() / (distanceNumber + intensityNumber);
//            for (int i = 0; i < scanNumber; ++i) {
//                QVector<long> ldata;
//                QVector<long> lintensity;
//                data.push_back(ldata);
//                intensity.push_back(lintensity);
//            }
//            if (!file.atEnd()) {
//                file.readLine();
//            }
//        }

//        while (!file.atEnd()) {
//            QString line = file.readLine();
//            QStringList strings = line.split(",");
//            for (int i = 0; i < scanNumber; ++i) {
//                for (int k = 0; k < distanceNumber; k++) {
//                    if (i < data.size()) {
//                        bool ok;
//                        data[i].push_back(strings[(i * (distanceNumber + intensityNumber)) + k].toLong(&ok, 10));
//                    }
//                }
//                data[i].push_back(-1);
//                for (int l = distanceNumber; l < (distanceNumber + intensityNumber); l++) {
//                    if (i < intensity.size()) {
//                        bool ok;
//                        intensity[i].push_back(strings[(i * (distanceNumber + intensityNumber)) + l].toLong(&ok, 10));
//                    }
//                }
//                intensity[i].push_back(-1);
//            }
//        }

//        trimVector(data);
//        trimVector(intensity);
//        file.close();
//    }
//    return true;
//}

bool UrgLogHandler::getDataFromCSV(const QString &source,
                                   QVector<SensorDataArray> &ranges,
                                   QVector<SensorDataArray> &levels,
                                   int distanceNumber = 3,
                                   int intensityNumber = 3)
{
    ranges.clear();
    levels.clear();

    QFileInfo fileInfo(source);
    if (!fileInfo.exists()) {
        m_errorMessage = tr("File does not exist.");
        return false;
    }

    QFile file(source);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        int scanNumber = 0;

        if (!file.atEnd()) {
            QString header = file.readLine();
            QStringList strings = header.split(",");

            if ((strings.size() % (distanceNumber + intensityNumber)) != 1) {
                m_errorMessage = tr("File format does not fit 6 culums format.");
                return false;
            }

            scanNumber = strings.size() / (distanceNumber + intensityNumber);
            for (int i = 0; i < scanNumber; ++i) {
                SensorDataArray ldata;
                SensorDataArray lintensity;
                ranges.push_back(ldata);
                levels.push_back(lintensity);
            }
            if (!file.atEnd()) {
                file.readLine();
            }
        }

        while (!file.atEnd()) {
            QString line = file.readLine();
            QStringList strings = line.split(",");
            for (int i = 0; i < scanNumber; ++i) {
                QVector<long> dtmp;
                for (int k = 0; k < distanceNumber; k++) {
                    dtmp.push_back(strings[(i * (distanceNumber + intensityNumber)) + k].toLong(NULL, 10));
                }
                ranges[i].steps.push_back(dtmp);

                QVector<long> itmp;
                for (int l = distanceNumber; l < (distanceNumber + intensityNumber); l++) {
                    itmp.push_back(strings[(i * (distanceNumber + intensityNumber)) + l].toLong(NULL, 10));
                }
                levels[i].steps.push_back(itmp);
            }
        }
        file.close();
    }
    return true;
}

//void UrgLogHandler::trimVector(QVector<QVector<long> > &ldata)
//{
//    for (int i = 0; i < ldata.size(); ++i) {
//        if (ldata[i].size() > 0) {
//            if (ldata[i][ldata[i].size() - 1] == -1) {
//                ldata[i].pop_back();
//            }
//        }
//    }
//}

bool UrgLogHandler::scanCoherent(bool noFreeze)
{

    if (!m_sin.isOpen()) {
        m_errorMessage = tr("Log file is not open.");
        return false;
    }

    if (headerCheck() < 0) {
        m_errorMessage = tr("Header check is failing.");
        return false;
    }

    m_sin.reset();
    m_sin.seek(0);

    QString line;
    long lineCnt = 0;
    m_shouldStopInit = false;

    while (!m_sin.atEnd() && !m_shouldStopInit) {
        line = m_sin.readLine();
        lineCnt++;
        if (line.startsWith(scanKey)) {
            if (!m_sin.atEnd()) {
                line = m_sin.readLine();
                lineCnt++;
                if (line.isEmpty()) {
                    m_errorMessage = tr("An empty scan is found.");
                    return false;
                }

                QStringList tokens = line.split(blockSeparator);

                int size = tokens.size();
                if (size < (endStep - startStep)) {
                    m_errorMessage = tr("Not enough data count int line %1. %2/%3.").arg(lineCnt).arg(size).arg(endStep - startStep);
                    return false;
                }

                for (int i = 0; i < tokens.size(); ++i) {
                    switch (m_captureMode) {
                    case GD_Capture_mode:
                    case MD_Capture_mode: {
                        if (tokens.at(i).contains(intensitySeparator)) {
                            m_errorMessage = tr("Intensity values in distance only mode.");
                            return false;
                        }
                        if (tokens.at(i).contains(dataSeparator)) {
                            m_errorMessage = tr("Multiecho values in distance only mode.");
                            return false;
                        }
                    }
                    break;
                    case GE_Capture_mode:
                    case ME_Capture_mode: {
                        if (tokens.at(i).contains(dataSeparator)) {
                            m_errorMessage = tr("Multiecho values in intensity mode.");
                            return false;
                        }
                        if (!tokens.at(i).contains(intensitySeparator)) {
                            m_errorMessage = tr("No Intensity values in intensity mode.");
                            return false;
                        }
                    }
                    break;
                    case HD_Capture_mode:
                    case ND_Capture_mode: {
                        if (tokens.at(i).contains(intensitySeparator)) {
                            m_errorMessage = tr("Intensity values in distance only mode.");
                            return false;
                        }
                    }
                    break;
                    case HE_Capture_mode:
                    case NE_Capture_mode: {
                        if (!tokens.at(i).contains(intensitySeparator)) {
                            m_errorMessage = tr("No Intensity values in intensity mode.");
                            return false;
                        }
                    }
                    break;
                    default:
                        m_errorMessage = tr("Capture mode unknown.");
                        return false;
                    }
                }
            }
        }

        if (noFreeze) {
            QApplication::processEvents();
        }
    }

    return true;
}

bool UrgLogHandler::close()
{
    if (m_isClosed) {
        return false;
    }

    m_isClosed = false;
    if (m_sout.isOpen()) {
        m_sout.close();
        m_isClosed = true;
    }

    if (m_sin.isOpen()) {
        m_sin.close();
        m_isClosed = true;
    }

    if (m_logFormat == "xls") {
        m_excel.SaveAs(m_filename.toUtf8().constData());
        m_isClosed = true;
    }


    m_errorMessage = m_isClosed ? "" : tr("Log file is already closed");
    return m_isClosed;
}

int UrgLogHandler::getLastTimestamp(void)
{
    return m_lastTimestamp;
}

long UrgLogHandler::addCaptureMode(RangeCaptureMode mode)
{
    m_captureMode = mode;

    switch (m_captureMode) {
    case GD_Capture_mode: {
        return add(captureModeKey, "GD_Capture_mode");
    }
    break;
    case MD_Capture_mode: {
        return add(captureModeKey, "MD_Capture_mode");
    }
    break;
    case GE_Capture_mode: {
        return add(captureModeKey, "GE_Capture_mode");
    }
    break;
    case ME_Capture_mode: {
        return add(captureModeKey, "ME_Capture_mode");
    }
    break;
    case HD_Capture_mode: {
        return add(captureModeKey, "HD_Capture_mode");
    }
    break;
    case ND_Capture_mode: {
        return add(captureModeKey, "ND_Capture_mode");
    }
    break;
    case HE_Capture_mode: {
        return add(captureModeKey, "HE_Capture_mode");
    }
    break;
    case NE_Capture_mode: {
        return add(captureModeKey, "NE_Capture_mode");
    }
    break;
    default:
        return 0;
    }
}

RangeCaptureMode UrgLogHandler::getCaptureMode()
{
    return m_captureModeRead;
}

QString UrgLogHandler::what()
{
    return m_errorMessage;
}

//long UrgLogHandler::addData(const QVector<long> &ranges, const QVector<long> &levels, long timestamp)
//{

//    QVector<QVector<long> > structuredData;
//    convertData(structuredData, data);

//    QVector<QVector<long> > structuredIntensity;
//    convertData(structuredIntensity, intensity);

//    return addData(structuredData, structuredIntensity, timestamp);

//}

long UrgLogHandler::addDataUbh(const SensorDataArray &ranges,
                const SensorDataArray &levels,
                long timestamp)
{
    long writtenCount = 0;

    if (! m_sout.isOpen()) {
        m_errorMessage = tr("Create log file first!");
        return writtenCount;
    }

    QTextStream out(&m_sout);

    m_lastTimestamp = timestamp;

    out << timestampKey << endl;
    writtenCount += timestampKey.size() + 1;
    out << timestamp << endl;
    writtenCount += QString::number(timestamp).size() + 1;

    out << logtimeKey << endl;
    writtenCount += logtimeKey.size() + 1;

    QString logTimeS = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");

    out << logTimeS << endl;
    writtenCount += logTimeS.size() + 1;

    out << scanKey << endl;
    writtenCount += scanKey.size() + 1;

    switch (m_captureMode) {
    case GD_Capture_mode:
    case MD_Capture_mode: {
        QString scanS;
        for (int i = 0; i < ranges.steps.size(); ++i) {
            if (ranges.steps[i].size() > 0) {
                scanS += QString("%1").arg(ranges.steps[i][0]);
            }
            else {
                scanS += QString("%1").arg(0);
            }

            if (i < (ranges.steps.size() - 1)) {
                scanS += blockSeparator;
            }
        }
        out << scanS << endl;
        writtenCount += scanS.size() + 1;
    }
    break;
    case GE_Capture_mode:
    case ME_Capture_mode: {
        QString scanS;
        for (int i = 0; i < ranges.steps.size(); ++i) {
            if (ranges.steps[i].size() > 0) {
                scanS += QString("%1").arg(ranges.steps[i][0]);
            }
            else {
                scanS += QString("%1").arg(0);
            }

            scanS += intensitySeparator;

            if (levels.steps.size() > i && levels.steps[i].size() > 0) {
                scanS += QString("%1").arg(levels.steps[i][0]);
            }
            else {
                scanS += QString("%1").arg(0);
            }

            if (i < (ranges.steps.size() - 1)) {
                scanS += blockSeparator;
            }
        }
        out << scanS << endl;
        writtenCount += scanS.size() + 1;
    }
    break;
    case HD_Capture_mode:
    case ND_Capture_mode: {
        QString scanS;
        for (int i = 0; i < ranges.steps.size(); ++i) {
            for (int j = 0; j < ranges.steps[i].size(); ++j) {
                scanS += QString("%1").arg(ranges.steps[i][j]);
                if (j < (ranges.steps[i].size() - 1)) {
                    scanS += dataSeparator;
                }
            }
            if (i < (ranges.steps.size() - 1)) {
                scanS += blockSeparator;
            }
        }
        out << scanS << endl;
        writtenCount += scanS.size() + 1;
    }
    break;
    case HE_Capture_mode:
    case NE_Capture_mode:
    case UnknownMode: {
        QString scanS;
        for (int i = 0; i < ranges.steps.size(); ++i) {
            for (int j = 0; j < ranges.steps[i].size(); ++j) {
                scanS += QString("%1").arg(ranges.steps[i][j]);

                scanS += intensitySeparator;

                if (levels.steps.size() > i && levels.steps[i].size() > j) {
                    scanS += QString("%1").arg(levels.steps[i][j]);
                }
                else {
                    scanS += QString("%1").arg(0);
                }

                if (j < (ranges.steps[i].size() - 1)) {
                    scanS += dataSeparator;
                }
            }
            if (i < (ranges.steps.size() - 1)) {
                scanS += blockSeparator;
            }
        }
        out << scanS << endl;
        writtenCount += scanS.size() + 1;
    }
    }

    if (m_useFlush) {
        out.flush();
    }

    return writtenCount;
}

long UrgLogHandler::addDataXls(const SensorDataArray &ranges,
                            const SensorDataArray &levels,
                            long timestamp)
{
    Q_UNUSED(timestamp)
    long writtenCount = 0;

//    for (int i = 0; i < m_maxEchoNumber; ++i) {
//        BasicExcelWorksheet* sheet = m_excel.GetWorksheet(i + 1);
//        if (sheet) {
//            sheet->Cell(0, 0)->SetString("Step N");
//            QString distance("Distance");
//            distance += QString::number(m_writePosition);
//            sheet->Cell(0, (m_writePosition * 2) + 1)->SetString(distance.toLatin1().constData());
//            QString intensity("Itensity");
//            intensity += QString::number(m_writePosition);
//            sheet->Cell(0, (m_writePosition * 2) + 2)->SetString(intensity.toLatin1().constData());

//            for (int j = 0; j < ranges.steps.size(); ++j) {
//                sheet->Cell(j + 1, 0)->SetDouble(j + startStep);
//                if (ranges.steps[j].size() > i) {
//                    sheet->Cell(j + 1, (m_writePosition * 2) + 1)->SetDouble(ranges.steps[j][i]);
//                }
//                else {
//                    sheet->Cell(j + 1, (m_writePosition * 2) + 1)->SetDouble(0);
//                }

//                if (levels.steps.size() > j && levels.steps[j].size() > i) {
//                    sheet->Cell(j + 1, (m_writePosition * 2) + 2)->SetDouble(levels.steps[j][i]);
//                }
//                else {
//                    sheet->Cell(j + 1, (m_writePosition * 2) + 2)->SetDouble(0);
//                }
//            }
//        }
//    }

    for (int i = 0; i < m_maxEchoNumber; ++i)
    {
        BasicExcelWorksheet *sheet = m_excel.GetWorksheet(i + 1);
        if (sheet)
        {
            sheet->Cell(0, 0)->SetString("Step N");
            QString distance("Distance ");
            distance += QString::number(m_writePosition);
            sheet->Cell(0, m_writePosition + 1)->SetString(distance.toLatin1().constData());

            for (int j = 0; j < ranges.steps.size(); ++j)
            {
                sheet->Cell(j + 1, 0)->SetDouble(j + startStep);
                if (i < ranges.steps[j].size())
                {
                    sheet->Cell(j + 1, m_writePosition + 1)->SetDouble(ranges.steps[j][i]);
                }
                else
                {
                    sheet->Cell(j + 1, m_writePosition + 1)->SetDouble(0);
                }

            }
        }
    }

    for (int i = 0; i < m_maxEchoNumber; ++i)
    {
        BasicExcelWorksheet *sheet = m_excel.GetWorksheet(m_maxEchoNumber + i + 1);
        if (sheet)
        {
            sheet->Cell(0, 0)->SetString("Step N");
            QString intensity("Intensity ");
            intensity += QString::number(m_writePosition);
            sheet->Cell(0, m_writePosition + 1)->SetString(intensity.toLatin1().constData());

            for (int j = 0; j < levels.steps.size(); ++j)
            {
                sheet->Cell(j + 1, 0)->SetDouble(j + startStep);
                if (i < levels.steps[j].size())
                {
                    sheet->Cell(j + 1, m_writePosition + 1)->SetDouble(levels.steps[j][i]);
                }
                else
                {
                    sheet->Cell(j + 1, m_writePosition + 1)->SetDouble(0);
                }

            }
        }
    }

    return writtenCount;
}

long UrgLogHandler::addDataCsv(const SensorDataArray &ranges,
                            const SensorDataArray &levels,
                            long timestamp)
{
    long writtenCount = 0;

    if (! m_sout.isOpen())
    {
        m_errorMessage = tr("Create log file first!");
        return -1;
    }

    QTextStream out(&m_sout);

    if(m_firstTime){
        out << "Timestamp" << ",";

        for(int i = 0; i < ranges.steps.size(); ++i)
        {
            for(int j = 0; j < m_maxEchoNumber; ++j)
            {
                out << "Step " << i << ",";
            }
        }
        out << endl;

        out << "Timestamp" << ",";
        for(int i = 0; i < ranges.steps.size(); ++i)
        {
            if(m_maxEchoNumber > 1){
                for(int j = 0; j < m_maxEchoNumber; ++j)
                {
                    if(m_isRange){
                        out << "Range " << j << ",";
                    }
                    else{
                        out << "Level " << j << ",";
                    }
                }
            }
            else{
                if(m_isRange){
                    out << "Range" << ",";
                }
                else{
                    out << "Level" << ",";
                }
            }
        }
        out << endl;
        m_firstTime = false;
    }

    out << timestamp << ",";

    for(int i = 0; i < ranges.steps.size(); ++i)
    {
        for(int j = 0; j < m_maxEchoNumber; ++j)
        {
            if(m_isRange){
                if(j < ranges.steps[i].size())
                {
                    out << ranges.steps[i][j] << ",";
                }
                else
                {
                    out << 0 << ",";
                }
            }
            else{
                if(i < levels.steps.size() && j < levels.steps[i].size())
                {
                    out << levels.steps[i][j] << ",";
                }
                else
                {
                    out << 0 << ",";
                }
            }
        }
    }

    out << endl;
    writtenCount += 10;


    if (m_useFlush) {
        out.flush();
    }

    return writtenCount;
}

long UrgLogHandler::addDataXy(const SensorDataArray &ranges,
                            const SensorDataArray &levels,
                            long timestamp)
{
    long writtenCount = 0;

    if (! m_sout.isOpen())
    {
        m_errorMessage = tr("Create log file first!");
        return -1;
    }

    QTextStream out(&m_sout);

    if(m_firstTime){
        out << "Timestamp" << ":";

        for(int i = 0; i < ranges.steps.size(); ++i)
        {
            for(int j = 0; j < m_maxEchoNumber; ++j)
            {
                out << "Step " << i << ",";
            }
        }

        out << endl;

        out << "Timestamp" << ":";

        for(int i = 0; i < ranges.steps.size(); ++i)
        {
            if(m_maxEchoNumber > 1){
                for(int j = 0; j < m_maxEchoNumber; ++j)
                {
                    out << "(" << "X" << j << blockSeparator << "Y" << j << blockSeparator << "Level " << j << ")" << ",";
                }
            }
            else{
                out << "(" << "X" << blockSeparator << "Y" << blockSeparator << "Level" << ")" << ",";
            }
        }

        out << endl;
        m_firstTime = false;
    }

    out << timestamp << ":";

    for(int i = 0; i < ranges.steps.size(); ++i)
    {
        if(m_maxEchoNumber > 1){
            for(int j = 0; j < m_maxEchoNumber; ++j)
            {
                if(j < ranges.steps[i].size())
                {
                    QPointF point = ranges.converter.range2point(
                                ranges.converter.index2Step(i),
                                ranges.steps[i][j]);
                    out << "(";
                    out << point.x() << blockSeparator << point.y() << blockSeparator;
                    if(i < levels.steps.size() && j < levels.steps[i].size())
                    {
                        out << levels.steps[i][j];
                    }
                    else
                    {
                        out << 0;
                    }
                    out << ")" << ",";
                }
                else{
                    out << "(";
                    out << 0 << blockSeparator << 0 << blockSeparator;
                    out << 0;
                    out << ")" << ",";
                }
            }
        }
        else{
            int j = 0;
            if(j < ranges.steps[i].size())
            {
                QPointF point = ranges.converter.range2point(
                            ranges.converter.index2Step(i),
                            ranges.steps[i][j]);
                out << "(";
                out << point.x() << blockSeparator << point.y() << blockSeparator;
                if(i < levels.steps.size() && j < levels.steps[i].size())
                {
                    out << levels.steps[i][j];
                }
                else
                {
                    out << 0;
                }
                out << ")" << ",";
            }
        }
    }

    out << endl;
    writtenCount += 10;    

    if (m_useFlush) {
        out.flush();
    }

    return writtenCount;
}

void UrgLogHandler::initHeaderRecords()
{
    if (m_isClosed || (m_currentMode != ReadMode)) return;
    m_sin.reset();
    m_sin.seek(0);
    QString line;
    m_header.clear();
    while (!m_sin.atEnd()) {
        line = m_sin.readLine().trimmed();
        if (line.startsWith(timestampKey)) {
            break;
        }
        else{
            m_header << line;
        }
    }
    m_sin.reset();
    m_sin.seek(0);
}


long UrgLogHandler::addData(const SensorDataArray &ranges,
                            const SensorDataArray &levels,
                            long timestamp)
{
    long writtenCount = 0;
    if (m_isClosed) {
        return writtenCount;
    }
    QMutexLocker locker(&m_mutex);
    if (m_logFormat == "ubh") {
        writtenCount = addDataUbh(ranges, levels, timestamp);
    }
    else if (m_logFormat == "xls") {
        writtenCount = addDataXls(ranges, levels, timestamp);
    }
    else if (m_logFormat == "csv")
    {
        writtenCount = addDataCsv(ranges, levels, timestamp);
    }
    else if (m_logFormat == "xy")
    {
        writtenCount = addDataXy(ranges, levels, timestamp);
    }
    m_writePosition++;

    return writtenCount;
}

bool UrgLogHandler::fileExists()
{
    QFileInfo fi(m_filename);
    return fi.exists();
}

//void UrgLogHandler::convertData(QVector<QVector<long> > &output, QVector<long> &input)
//{
//    output.clear();

//    bool multiEchoMode = false;
//    for (int i = 0; i < input.size(); ++i) {
//        if (input[i] == -1) {
//            multiEchoMode = true;
//            break;
//        }
//    }

//    if (multiEchoMode) {
//        if (input.size() > 0) {
//            QVector<long> step;
//            output.push_back(step);
//            for (int i = 0; i < input.size(); ++i) {
//                if (input[i] == -1) {
//                    QVector<long> step;
//                    output.push_back(step);
//                    continue;
//                }
//                output[output.size() - 1].push_back(input[i]);
//            }
//        }
//    }
//    else {
//        for (int i = 0; i < input.size(); ++i) {
//            QVector<long> step;
//            step.push_back(input[i]);
//            output.push_back(step);
//        }
//    }
//}

bool UrgLogHandler::getDataInit()
{
    int res = headerCheck();

    m_sin.reset();
    m_sin.seek(0);

    m_readPosition = 0;
    m_writePosition = 0;
    return res == 0;
}

long UrgLogHandler::setDataPos(long pos)
{
    QMutexLocker locker(&m_mutex);
    if ((pos >= 0) && (pos < m_markPoints.size())) {
        m_sin.reset();
        m_sin.seek(m_markPoints[pos]);
        m_readPosition = pos;
        return m_readPosition;
    }
    else {
        m_errorMessage = tr("Target position is out of range.");
        return -1;
    }
}

void UrgLogHandler::skipDataForward(long pos)
{
    setDataPos(m_readPosition + pos);
}

void UrgLogHandler::skipDataBackward(long pos)
{
    setDataPos(m_readPosition - pos);
}

//void UrgLogHandler::getAllData(QVector< QVector<long> > &vdata, QVector< QVector<long> > &vlevels, QVector<long> &vtimestamp)
//{
//    long currentPos = 0;
//    do {
//        QVector<long> data;
//        QVector<long> levels;
//        long timestamp = 0;
//        currentPos = getData(data, levels, timestamp);
//        vdata.push_back(data);
//        vlevels.push_back(levels);
//        vtimestamp.push_back(timestamp);
//    }
//    while (currentPos != -1);
//}

void UrgLogHandler::getAllData(QVector<SensorDataArray> &vdata, QVector<SensorDataArray> &vlevels,
                               QVector<long> &vtimestamp)
{
    long currentPos = 0;
    do {
        SensorDataArray ranges;
        SensorDataArray levels;
        long timestamp = 0;
        currentPos = getData(ranges, levels, timestamp);
        vdata.push_back(ranges);
        vlevels.push_back(levels);
        vtimestamp.push_back(timestamp);
    }
    while (currentPos != -1);
}

//long UrgLogHandler::getNextData(QVector<long> &ranges, QVector<long> &levels, long &timestamp)
//{
//    long lastReadPosition = getReadPosition();
//    long currentReadPosition = getData(data, levels, timestamp);
//    setDataPos(lastReadPosition);
//    return currentReadPosition;
//}

long UrgLogHandler::getNextData(SensorDataArray &ranges, SensorDataArray &levels, long &timestamp)
{
    long lastReadPosition = getReadPosition();
    long currentReadPosition = getData(ranges, levels, timestamp);
    setDataPos(lastReadPosition);
    return currentReadPosition;
}

//long UrgLogHandler::getData(QVector<long> &ranges, QVector<long> &levels, long &timestamp)
//{
//    QMutexLocker locker(&mutex);
//    if (captureMode == Unknow_Capture_mode) {
//        error_message = tr("Please run getDataInit() first!");
//        return -1;
//    }

//    QString line;

//    data.clear();
//    levels.clear();

//    bool recordFound = false;

//    while (!sin.atEnd() && !shouldStopInit) {
//        line = sin.readLine();
//        if (line.startsWith(timestampKey)) {
//            recordFound = true;
//            break;
//        }
//    }

//    if (!recordFound) {
//        error_message = tr("No record found!");
//        return -1;
//    }

//    if (sin.atEnd() || (line = sin.readLine()).isEmpty()) {
//        error_message = tr("An empty timestamp is found.");
//        return -1;
//    }
//    timestamp = line.toLong();
//    timeStamp = timestamp;

//    if (sin.atEnd() || (line = sin.readLine()).isEmpty()) {
//        error_message = tr("Log time is not found");
//        return -1;
//    }

//    if (!line.startsWith(logtimeKey)) {
//        error_message = tr("Log time place order is wrong.");
//        return -1;
//    }

//    if (sin.atEnd() || (line = sin.readLine()).isEmpty()) {
//        error_message = tr("Log time data is not found");
//        return -1;
//    }
//    logTime = line.trimmed();

//    if (sin.atEnd() || (line = sin.readLine()).isEmpty()) {
//        error_message = tr("Scan record is not found");
//        return -1;
//    }

//    if (!line.startsWith(scanKey)) {
//        error_message = tr("Scan place order is wrong.");
//        return -1;
//    }

//    if (sin.atEnd() || (line = sin.readLine()).isEmpty()) {
//        error_message = tr("Scan data is not found");
//        return -1;
//    }

//    QStringList tokens = line.split(blockSeparator);

//    int size = tokens.size();
//    if (size < (endStep - startStep)) {
//        error_message = tr("Number of scan data is not fit to the steps record.");
//        return 0;
//    }

//    for (int i = 0; i < tokens.size(); ++i) {
//        switch (captureMode) {
//        case GD_Capture_mode:
//        case MD_Capture_mode: {
//            if (tokens.at(i).contains(intensitySeparator)) {
//                error_message = tr("Intensity values in distance only mode.");
//                return -1;
//            }
//            if (tokens.at(i).contains(dataSeparator)) {
//                error_message = tr("Multiecho values in distance only mode.");
//                return -1;
//            }

//            long distance = tokens.at(i).toLong();
//            data.push_back(distance);
//        }
//        break;
//        case GE_Capture_mode:
//        case ME_Capture_mode: {
//            if (tokens.at(i).contains(dataSeparator)) {
//                error_message = tr("Multiecho values in distance and intensity only mode.");
//                return -1;
//            }
//            if (!tokens.at(i).contains(intensitySeparator)) {
//                error_message = tr("No Intensity values in intenisty mode.");
//                return -1;
//            }
//            QStringList blockTokens = tokens.at(i).split(intensitySeparator);
//            data.push_back(blockTokens.at(0).toLong());
//            levels.push_back(blockTokens.at(1).toLong());

//        }
//        break;
//        case HD_Capture_mode:
//        case ND_Capture_mode: {
//            if (tokens.at(i).contains(intensitySeparator)) {
//                error_message = tr("Intensity values in distance only mode.");
//                return -1;
//            }

//            QStringList blockTokens = tokens.at(i).split(dataSeparator);
//            for (int i = 0; i < blockTokens.size(); ++i) {
//                data.push_back(blockTokens.at(i).toLong());

//            }

//            long separator = -1;
//            data.push_back(separator);

//        }
//        break;
//        case HE_Capture_mode:
//        case NE_Capture_mode: {
//            if (!tokens.at(i).contains(intensitySeparator)) {
//                error_message = tr("No Intensity values in intenisty mode.");
//                return -1;
//            }

//            QStringList blockTokens = tokens.at(i).split(dataSeparator);
//            for (int i = 0; i < blockTokens.size(); ++i) {

//                QStringList intenistyTokens = blockTokens.at(i).split(intensitySeparator);
//                data.push_back(intenistyTokens.at(0).toLong());
//                levels.push_back(intenistyTokens.at(1).toLong());

//            }
//            long separator = -1;
//            data.push_back(separator);
//            levels.push_back(separator);
//        }
//        break;
//        default:
//            error_message = tr("Capture mode unknown.");
//            return -1;
//        }
//    }

//    if (data.size() > 0) {
//        if (data[data.size() - 1] == -1) {
//            data.pop_back();
//        }
//        eraseVectorLast(data, endStepRead, startStep);
//        eraseVectorFront(data, startStepRead, startStep);

//        if (captureModeRead == GE_Capture_mode || captureModeRead == ME_Capture_mode ||
//                captureModeRead == GD_Capture_mode || captureModeRead == MD_Capture_mode) {
//            data = getFirstEcho(data);
//        }

//    }

//    if (captureModeRead == GE_Capture_mode || captureModeRead == ME_Capture_mode ||
//            captureModeRead == HE_Capture_mode || captureModeRead == NE_Capture_mode) {
//        if (levels.size() > 0) {
//            if (levels[levels.size() - 1] == -1) {
//                levels.pop_back();
//            }

//            // Desired steps interval fitting
//            eraseVectorLast(levels, endStepRead, startStep);
//            eraseVectorFront(levels, startStepRead, startStep);

//            // Non multi data desired
//            if (captureModeRead == GE_Capture_mode || captureModeRead == ME_Capture_mode ||
//                    captureModeRead == GD_Capture_mode || captureModeRead == MD_Capture_mode) {
//                levels = getFirstEcho(levels);
//            }
//        }
//    }
//    else {
//        levels.clear();
//    }

//    readPosition++;

//    return readPosition - 1;
//}

long UrgLogHandler::getData(SensorDataArray &ranges, SensorDataArray &levels, long &timestamp)
{
    QMutexLocker locker(&m_mutex);
    if (m_captureMode == Unknown_Capture_mode) {
        m_errorMessage = tr("Please run getDataInit() first!");
        return -1;
    }

    QString line;

    ranges.steps.clear();
    levels.steps.clear();

    bool recordFound = false;

    while (!m_sin.atEnd() && !m_shouldStopInit) {
        line = m_sin.readLine();
        if (line.startsWith(timestampKey)) {
            recordFound = true;
            break;
        }
    }

    if (!recordFound) {
        m_errorMessage = tr("No record found!");
        return -1;
    }

    if (m_sin.atEnd() || (line = m_sin.readLine()).isEmpty()) {
        m_errorMessage = tr("An empty timestamp is found.");
        return -1;
    }
    timestamp = line.toLong();
    m_timeStamp = timestamp;

    if (m_sin.atEnd() || (line = m_sin.readLine()).isEmpty()) {
        m_errorMessage = tr("Log time is not found");
        return -1;
    }

    if (!line.startsWith(logtimeKey)) {
        m_errorMessage = tr("Log time place order is wrong.");
        return -1;
    }

    if (m_sin.atEnd() || (line = m_sin.readLine()).isEmpty()) {
        m_errorMessage = tr("Log time data is not found");
        return -1;
    }
    m_logTime = line.trimmed();

    if (m_sin.atEnd() || (line = m_sin.readLine()).isEmpty()) {
        m_errorMessage = tr("Scan record is not found");
        return -1;
    }

    if (!line.startsWith(scanKey)) {
        m_errorMessage = tr("Scan place order is wrong.");
        return -1;
    }

    if (m_sin.atEnd() || (line = m_sin.readLine()).isEmpty()) {
        m_errorMessage = tr("Scan data is not found");
        return -1;
    }

    QStringList tokens = line.split(blockSeparator);

//    int size = tokens.size();
//    if (size < (endStep - startStep)) {
//        error_message = tr("Number of scan data is not fit to the steps record.");
//        return 0;
//    }

    for (int i = 0; i < tokens.size(); ++i) {
        switch (m_captureMode) {
        case GD_Capture_mode:
        case MD_Capture_mode: {
            if (tokens.at(i).contains(intensitySeparator)) {
                m_errorMessage = tr("Intensity values in distance only mode.");
                return -1;
            }
            if (tokens.at(i).contains(dataSeparator)) {
                m_errorMessage = tr("Multiecho values in distance only mode.");
                return -1;
            }

            long distance = tokens.at(i).toLong();
            QVector<long> dtmp;
            dtmp.push_back(distance);
            ranges.steps.push_back(dtmp);
        }
        break;
        case GE_Capture_mode:
        case ME_Capture_mode: {
            if (tokens.at(i).contains(dataSeparator)) {
                m_errorMessage = tr("Multiecho values in distance and intensity only mode.");
                return -1;
            }
            if (!tokens.at(i).contains(intensitySeparator)) {
                m_errorMessage = tr("No Intensity values in intensity mode.");
                return -1;
            }
            QStringList blockTokens = tokens.at(i).split(intensitySeparator);
            QVector<long> dtmp;
            dtmp.push_back(blockTokens.at(0).toLong());
            ranges.steps.push_back(dtmp);

            QVector<long> itmp;
            itmp.push_back(blockTokens.at(1).toLong());
            levels.steps.push_back(itmp);
        }
        break;
        case HD_Capture_mode:
        case ND_Capture_mode: {
            if (tokens.at(i).contains(intensitySeparator)) {
                m_errorMessage = tr("Intensity values in distance only mode.");
                return -1;
            }

            QStringList blockTokens = tokens.at(i).split(dataSeparator);
            QVector<long> dtmp;
            for (int i = 0; i < blockTokens.size(); ++i) {
                dtmp.push_back(blockTokens.at(i).toLong());

            }

            ranges.steps.push_back(dtmp);

        }
        break;
        case HE_Capture_mode:
        case NE_Capture_mode: {
            if (!tokens.at(i).contains(intensitySeparator)) {
                m_errorMessage = tr("No Intensity values in intensity mode.");
                return -1;
            }

            QStringList blockTokens = tokens.at(i).split(dataSeparator);
            QVector<long> dtmp;
            QVector<long> itmp;
            for (int i = 0; i < blockTokens.size(); ++i) {

                QStringList intenistyTokens = blockTokens.at(i).split(intensitySeparator);
                dtmp.push_back(intenistyTokens.at(0).toLong());
                itmp.push_back(intenistyTokens.at(1).toLong());

            }
            ranges.steps.push_back(dtmp);
            levels.steps.push_back(itmp);
        }
        break;
        default:
            m_errorMessage = tr("Capture mode unknown.");
            return -1;
        }
    }

    if (ranges.steps.size() > 0) {
        if(m_startStepRead > startStep){
            ranges.steps.erase(ranges.steps.begin(), ranges.steps.begin() + (m_startStepRead - startStep));
        }
        if(m_endStepRead < endStep){
            ranges.steps.erase(ranges.steps.end() - (endStep - m_endStepRead), ranges.steps.end());
        }
    }

    if (levels.steps.size() > 0) {
        if(m_startStepRead > startStep){
            levels.steps.erase(levels.steps.begin(), levels.steps.begin() + (m_startStepRead - startStep));
        }
        if(m_endStepRead < endStep){
            levels.steps.erase(levels.steps.end() - (endStep - m_endStepRead), levels.steps.end());
        }
    }

    if (m_captureModeRead == GE_Capture_mode || m_captureModeRead == ME_Capture_mode ||
            m_captureModeRead == GD_Capture_mode || m_captureModeRead == MD_Capture_mode) {
        getFirstEcho(ranges);
        getFirstEcho(levels);
    }
    if (m_captureModeRead == GD_Capture_mode || m_captureModeRead == MD_Capture_mode ||
            m_captureModeRead == HD_Capture_mode || m_captureModeRead == ND_Capture_mode) {
        levels.steps.clear();
    }

    ranges.converter = getConverter();
    levels.converter = getConverter();
    ranges.timestamp = timestamp;
    levels.timestamp = timestamp;

    m_readPosition++;
    return m_readPosition - 1;
}

long UrgLogHandler::getTimestamp(long &timestamp)
{
    QMutexLocker locker(&m_mutex);

    QString line;

    bool recordFound = false;

    while (!m_sin.atEnd() && !m_shouldStopInit) {
        line = m_sin.readLine();
        if (line.startsWith(timestampKey)) {
            recordFound = true;
            break;
        }
    }

    if (!recordFound) {
        m_errorMessage = tr("No record found!");
        return -1;
    }

    if (m_sin.atEnd() || (line = m_sin.readLine()).isEmpty()) {
        m_errorMessage = tr("An empty timestamp is found.");
        return -1;
    }
    timestamp = line.toLong();
    m_timeStamp = timestamp;

    m_readPosition++;
    return m_readPosition - 1;
}

//QVector<long> UrgLogHandler::getFirstEcho(QVector<long> &ranges)
//{
//    QVector<long> distances;
//    distances.clear();
//    bool multiEchoMode = false;
//    for (int i = 0; i < data.size(); ++i) {
//        if (data[i] == -1) {
//            multiEchoMode = true;
//            break;
//        }
//    }
//    if (multiEchoMode) {

//        distances.push_back(data[0]);

//        for (int i = 0; i < data.size(); ++i) {
//            if (data[i] == -1) {
//                distances.push_back(data[i + 1]);
//            }
//        }
//    }
//    else {
//        for (int i = 0; i < data.size(); ++i) {
//            distances.push_back(data[i]);
//        }
//    }

//    return distances;
//}

void UrgLogHandler::getFirstEcho(SensorDataArray &ranges)
{
    for(int i = 0; i < ranges.steps.size(); ++i){
        ranges.steps[i].erase(ranges.steps[i].begin() +1, ranges.steps[i].end());
    }
}

//void UrgLogHandler::eraseVectorLast(QVector<long> &ranges, int limit, int startS)
//{

//    int dataSize = data.size();
//    if (dataSize > 0) {
//        switch (captureMode) {
//        case GD_Capture_mode:
//        case MD_Capture_mode:
//        case GE_Capture_mode:
//        case ME_Capture_mode: {
//            if ((limit - startS) < (dataSize - 1)) {
//                data.erase(data.begin() + (limit - startS + 1), data.end());
//            }
//        }
//        break;
//        case HD_Capture_mode:
//        case ND_Capture_mode:
//        case HE_Capture_mode:
//        case NE_Capture_mode: {
//            int stepCnt = 1;
//            for (int i = 0; i < dataSize; ++i) if (data[i] == -1) {
//                    stepCnt++;
//                }

//            if ((limit - startS) < (stepCnt - 1)) {
//                int step = 0;
//                for (int i = 0; i < data.size(); ++i) {
//                    if (step == (limit - startS + 1)) {
//                        data.erase(data.begin() + i - 1, data.end());
//                        break;
//                    }
//                    if (data[i] == -1) {
//                        step++;
//                    }
//                }
//            }
//        }
//        break;
//        default: {
//        }
//        }
//    }
//}

//void UrgLogHandler::eraseVectorFront(QVector<long> &ranges, int limit, int startS)
//{
//    if (limit < 1 || data.size() < 1) {
//        return;
//    }

//    if (data.size() > 0) {
//        switch (captureMode) {
//        case GD_Capture_mode:
//        case MD_Capture_mode:
//        case GE_Capture_mode:
//        case ME_Capture_mode: {
//            if ((limit - startS) > 0) {
//                data.erase(data.begin(), data.begin() + (limit - startS));
//            }
//        }
//        break;
//        case HD_Capture_mode:
//        case ND_Capture_mode:
//        case HE_Capture_mode:
//        case NE_Capture_mode: {
//            unsigned int stepCnt = 1;
//            for (int i = 0; i < data.size(); ++i) if (data[i] == -1) {
//                    stepCnt++;
//                }

//            if ((limit - startS) > 0) {
//                int step = 0;
//                for (int i = 0; i < data.size(); ++i) {
//                    if (step == (limit - startS)) {
//                        data.erase(data.begin(), data.begin() + i);
//                        break;
//                    }
//                    if (data[i] == -1) {
//                        step++;
//                    }
//                }
//            }
//        }
//        break;
//        default: {

//        }
//        }
//    }
//}

void UrgLogHandler::setSeparators(const QString &block, const QString &ranges, const QString &levels)
{
    blockSeparator = block;
    dataSeparator = ranges;
    intensitySeparator = levels;
}

long UrgLogHandler::addModel(const QString &value)
{
    return add(modelKey, value);
}

long UrgLogHandler::addSerialNumber(const QString &value)
{
    return add(serialNumberKey, value);
}

long UrgLogHandler::addSensorVersion(const QString &value)
{
    return add(firmwareVersionKey, value);
}

long UrgLogHandler::addRangeSensorParameter(RangeSensorParameter parameter)
{
    long usedSize = 0;
    usedSize += addModel(QString::fromStdString(parameter.model));
    usedSize += addFrontStep(parameter.area_front);
    usedSize += addMinDistance(parameter.distance_min);
    usedSize += addMaxDistance(parameter.distance_max);
    usedSize += addMotorSpeed(parameter.scan_rpm);
    usedSize += addTotalSteps(parameter.area_total);
    m_urgParameter = parameter;

    return usedSize;
}

RangeSensorParameter UrgLogHandler::getRangeSensorParameter()
{
    RangeSensorParameter parameter;
    parameter.model = getModel().toStdString();
    parameter.area_min = getStartStep();
    parameter.area_max = getEndStep();
    parameter.area_front = getFrontStep();
    parameter.distance_min = getMinDistance();
    parameter.distance_max = getMaxDistance();
    parameter.scan_rpm = getMotorSpeed();
    parameter.area_total = getTotalSteps();

    return parameter;
}

long UrgLogHandler::add(const QString &key, int value)
{
    long usedSize = 0;
    if (m_logFormat == "ubh") {
        if (! m_sout.isOpen()) {
            m_errorMessage = tr("Create log file first!");
            return false;
        }

        QTextStream out(&m_sout);

        out << key << endl << value << endl;
        usedSize += key.size() + 1 + QString::number(value).size() + 1;

        if (m_useFlush) {
            out.flush();
        }
    }
    else if (m_logFormat == "xls") {
        BasicExcelWorksheet* sheet = m_excel.GetWorksheet((size_t)0);
        if (sheet) {
            int i = 0;
            while (sheet->Cell(i, 0)->Type() != BasicExcelCell::UNDEFINED) {
                ++i;
            }
            sheet->Cell(i, 0)->SetString(key.toLatin1().constData());
            sheet->Cell(i, 1)->SetInteger(value);
        }
    }

    return usedSize;
}

long UrgLogHandler::add(const QString &key, const QString &value)
{
    long usedSize = 0;
    if (m_logFormat == "ubh") {
        if (! m_sout.isOpen()) {
            m_errorMessage = tr("Create log file first!");
            return false;
        }

        QTextStream out(&m_sout);

        out << key << endl << value << endl;
        usedSize += key.size() + 1 + value.size() + 1;

        if (m_useFlush) {
            out.flush();
        }


    }
    else if (m_logFormat == "xls") {
        BasicExcelWorksheet* sheet = m_excel.GetWorksheet((size_t)0);
        if (sheet) {
            int i = 0;
            while (sheet->Cell(i, 0)->Type() != BasicExcelCell::UNDEFINED) {
                ++i;
            }
            sheet->Cell(i, 0)->SetString(key.toLatin1().constData());
            sheet->Cell(i, 1)->SetString(value.toLatin1().constData());
        }
    }

    return usedSize;
}

long UrgLogHandler::add(const QString &key, long value)
{
    long usedSize = 0;
    if (m_logFormat == "ubh") {
        if (! m_sout.isOpen()) {
            m_errorMessage = tr("Create log file first!");
            return false;
        }

        QTextStream out(&m_sout);

        out << key << endl << value << endl;
        usedSize += key.size() + 1 + QString::number(value).size() + 1;

        if (m_useFlush) {
            out.flush();
        }
    }
    else if (m_logFormat == "xls") {
        BasicExcelWorksheet* sheet = m_excel.GetWorksheet((size_t)0);
        if (sheet) {
            int i = 0;
            while (sheet->Cell(i, 0)->Type() != BasicExcelCell::UNDEFINED) {
                ++i;
            }
            sheet->Cell(i, 0)->SetString(key.toLatin1().constData());
            sheet->Cell(i, 1)->SetDouble(value);
        }
    }

    return usedSize;
}

long UrgLogHandler::addStartStep(int value)
{
    return add(startStepKey,  value);
}

long UrgLogHandler::addEndStep(int value)
{
    return add(endStepKey, value);
}

long UrgLogHandler::addGrouping(int value)
{
    return add(groupingKey, value);
}

long UrgLogHandler::addFrontStep(int value)
{
    return add(frontStepKey, value);
}

long UrgLogHandler::addTotalSteps(int value)
{
    return add(totalStepsKey, value);
}

long UrgLogHandler::addMinDistance(long value)
{
    return add(minDistanceKey, value);
}

long UrgLogHandler::addMaxDistance(long value)
{
    return add(maxDistanceKey, value);
}

long UrgLogHandler::addMotorSpeed(int value)
{
    return add(motorSpeedKey, value);
}

long UrgLogHandler::addScanMsec(int value)
{
    return add(scanMsecKey, value);
}

void UrgLogHandler::setMaxEchoNumber(int number)
{
    m_maxEchoNumber = number;
}

void UrgLogHandler::setReadStartStep(int step)
{
    if (step >= startStep) {
        m_startStepRead = step;
    }
}

void UrgLogHandler::setReadEndStep(int step)
{
    if (step <= endStep) {
        m_endStepRead = step;
    }
}

Converter UrgLogHandler::getConverter()
{
    return Converter(frontStep, totalSteps,
                     m_startStepRead, m_endStepRead,
                     grouping);
}

void UrgLogHandler::useFlush(bool state)
{
    m_useFlush = state;
}

//QVector<long> UrgLogHandler::getEcho(int echo, QVector<long> &ranges, int length)
//{
//    QVector<long> distances;
//    distances.clear();
//    bool multiEchoMode = false;
//    for (int i = 0; i < data.size(); ++i) {
//        if (data[i] == -1) {
//            multiEchoMode = true;
//            break;
//        }
//    }
//    if (multiEchoMode) {
//        int echoCnt = 0;
//        for (int i = 0; i < data.size(); ++i) {
//            if (data[i] == -1) {
//                if (echo >= echoCnt) {
//                    distances.push_back(0);
//                }
//                echoCnt = 0;
//                continue;
//            }
//            if (echoCnt == echo) {
//                distances.push_back(data[i]);
//            }
//            echoCnt++;
//        }
//        if (echo >= echoCnt) {
//            distances.push_back(0);
//        }
//    }
//    else {
//        for (int i = 0; i < data.size(); ++i) {
//            if (echo == 0) {
//                distances.push_back(data[i]);
//            }
//            else {
//                distances.push_back(0);
//            }
//        }
//    }

//    while (distances.size() < length) {
//        distances.push_back(0);
//    }
//    return distances;
//}

