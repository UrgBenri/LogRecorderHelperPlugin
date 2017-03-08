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

#include "LogRecorderHelperPlugin.h"
#include "ui_LogRecorderHelperPlugin.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QDateTime>
#include <QMovie>
#include <QThread>
#include <QDebug>

//#include "SettingsWindow.h"
#include "diskUtils.h"

LogRecorderHelperPlugin::LogRecorderHelperPlugin(QWidget *parent)
    : HelperPluginInterface(parent)
    , ui(new Ui::LogRecorderHelperPlugin)
    , m_urg(NULL)
    , m_recording(false)
    , m_writing_thread(&writingThreadProcess, this)
{
    REGISTER_FUNCTION(setDeviceMethod);
    REGISTER_FUNCTION(addSensorDataMethod);
    REGISTER_FUNCTION(stopMethod);

    ui->setupUi(this);

    ui->recordStartButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->recordStopButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);

    connect(ui->recordStartButton, &QAbstractButton::clicked,
            this, &LogRecorderHelperPlugin::recordStartPressed);
    connect(ui->recordStopButton, &QAbstractButton::clicked,
            this, &LogRecorderHelperPlugin::recordStopPressed);
    connect(&m_diskSapceWatcher, &QTimer::timeout,
            this, &LogRecorderHelperPlugin::checkDiskSpace);

    connect(&m_writing_thread, &Thread::finished,
            this, &LogRecorderHelperPlugin::recordStopPressed);
    
    connect(this, &LogRecorderHelperPlugin::dataAdded,
            this, &LogRecorderHelperPlugin::updateUI);

    connect(this, &LogRecorderHelperPlugin::recordStarted,
            this, &LogRecorderHelperPlugin::recordStartPressed);
    connect(this, &LogRecorderHelperPlugin::recordStopped,
            this, &LogRecorderHelperPlugin::recordStopPressed);
    
    QMovie *movie = new QMovie(this);
    movie->setFileName(":/LogRecorderHelperPlugin/loading");
    ui->recordActivityIndicator->setMovie(movie);
    movie->start();

    ui->recordActivityIndicator->hide();
    m_recordLimit = 0;
    ui->elapsedTime->setText("");
    ui->scansCount->setText("");
}

LogRecorderHelperPlugin::~LogRecorderHelperPlugin()
{
    delete ui;
}

void LogRecorderHelperPlugin::saveState(QSettings &settings)
{

}

void LogRecorderHelperPlugin::restoreState(QSettings &settings)
{

}

void LogRecorderHelperPlugin::loadTranslator(const QString &locale)
{
    qApp->removeTranslator(&m_translator);
    if (m_translator.load(QString("plugin.%1").arg(locale), ":/LogRecorderHelperPlugin")) {
        qApp->installTranslator(&m_translator);
    }
}

void LogRecorderHelperPlugin::recordStartPressed()
{
    if(!m_urg) return;

    if (!m_recording) {
        QString defaulName = QString("/") +
                    QDateTime::currentDateTime().toString("yyyy_MM_dd_HH_mm_ss_zzz") +
                    ".ubh";
        QString filename = QFileDialog::getSaveFileName(
                    this,
                    tr("Save Sensor Data"),
                    QDir::currentPath() + defaulName,
                    tr("Log file (*.ubh)")
                    + ";;" +tr("CSV file (*.csv)")
                    + ";;" +tr("XY point file (*.xy)")
                    + (ui->recordLimitBox->value() > 0 ? (";;" + tr("MS Excel file (*.xls)")) : "")
                    );
        if (!filename.isNull()) {
            QFileInfo fi(filename);
            QDir::setCurrent(fi.absolutePath());

            double dTotal;
            double dFree;
            QDir dfi(fi.absoluteDir());
            bool valid = getFreeTotalSpace(dfi.rootPath(), dTotal, dFree);
            if (valid) {
//                if ((dFree / 1024) < SettingsWindow::getSetting("minimumFreeSpace", 500)) {
//                    emit error(QApplication::applicationName(),
//                               tr("The target disk does'nt have the required %1Mb of free space,\n"
//                                  "you have only %2Mb.")
//                               .arg(SettingsWindow::getSetting("minimumFreeSpace", 500)).arg(dFree / 1024));
//                    return;
//                }

                int dRatio = static_cast<int>(((dFree / dTotal) * 100.0) + 0.5);
                //                ui->recordActivityIndicator->setValue(100 - dRatio);
                ui->recordActivityIndicator->show();
                
                ui->elapsedTime->setText("");
                ui->scansCount->setText("");

                if (m_recordLimit == 0) {
                    int speed = m_urg->scanMsec();
                    double nbScan = dFree / 9;
                    double totalMsec = nbScan * speed;
                    int totalHours = qRound(totalMsec / (1000 * 60 * 60));
                    emit information(QApplication::applicationName(),
                                     tr("Knowing you have %1% of free space,\n"
                                        "you can record approximately %2 hours.")
                                     .arg(dRatio).arg(totalHours));
                }
            }

            //            if(receiving_thread.isRunning()){
            //                receiving_thread.wait();
            //            }

            if (m_recorder.create(filename)) {
                m_recordLimit = ui->recordLimitBox->value();

                m_recorder.useFlush(true);

                RangeSensorParameter parameter;
                parameter = m_urg->parameter();

                m_recorder.setCaptureMode(m_urg->captureMode());

                m_recorder.addSensorVersion(m_urg->getFirmwareVersion());
                m_recorder.addRangeSensorParameter(parameter);
                m_recorder.addCaptureMode(m_urg->captureMode());
                m_recorder.addScanMsec(m_urg->scanMsec());

                m_recorder.addStartStep(m_urg->startStep());
                m_recorder.addEndStep(m_urg->endStep());
                m_recorder.addGrouping(m_urg->captureGroupSteps());

                m_recorder.addSerialNumber(m_urg->getSerialNumber());

                ui->recordFilenameLine->setText(filename);
                ui->recordStartButton->setEnabled(false);
                ui->recordStopButton->setEnabled(true);

                m_startTimer.restart();
//                m_diskSapceWatcher.start(SettingsWindow::getSetting("diskWatcherInterval", 60) * 1000);

                m_recordMutex.lock();
                m_writeBuffer.clear();
                m_recordMutex.unlock();

                m_recording = true;

                if(!m_writing_thread.isRunning()){
                    m_writing_thread.run();
                }
            }
            else {
                ui->recordFilenameLine->setText("");
                ui->recordStartButton->setEnabled(true);
                ui->recordStopButton->setEnabled(false);
                m_recording = false;
                ui->recordActivityIndicator->hide();
                emit error(QApplication::applicationName(),
                           tr("This file could not be created.") + QString("\n%1")
                           .arg(filename));
            }
        }
        else {
            ui->recordFilenameLine->setText("");
            ui->recordStartButton->setEnabled(true);
            ui->recordStopButton->setEnabled(false);
            m_recording = false;
            ui->recordActivityIndicator->hide();
        }
    }
    else {
        ui->recordFilenameLine->setText("");
        m_recording = false;
        ui->recordActivityIndicator->hide();
        m_recorder.close();
        ui->recordStartButton->setEnabled(true);
        ui->recordStopButton->setEnabled(false);
    }
}

void LogRecorderHelperPlugin::recordStopPressed()
{
    if(m_writing_thread.isRunning()){
        m_writing_thread.stop();
    }

    ui->recordFilenameLine->setText("");
    ui->recordActivityIndicator->hide();
    m_recorder.close();
    ui->recordStartButton->setEnabled(true);
    ui->recordStopButton->setEnabled(false);

    if (m_recording) {
        emit information(QApplication::applicationName(),
                         tr("%1 scans were saved.")
                         .arg(m_recorder.getWritePosition()));
    }

    m_diskSapceWatcher.stop();

    m_recording = false;

    m_recordMutex.lock();
    m_writeBuffer.clear();
    m_recordMutex.unlock();

    m_recordLimit = 0;
    ui->elapsedTime->setText("");
    ui->scansCount->setText("");
}

void LogRecorderHelperPlugin::setSensor(UrgDevice *urg)
{
    m_urg = urg;
}

QVariant LogRecorderHelperPlugin::setDeviceMethod(const QVariantList &vars)
{
    if(vars.size() == 1){
        setSensor((UrgDevice*) vars[0].value<void *>());
    }
    return QVariant();
}

QVariant LogRecorderHelperPlugin::addSensorDataMethod(const QVariantList &vars)
{
    if(vars.size() >= 3){
        addData(*(SensorDataArray*) vars[0].value<void *>()
                , *(SensorDataArray*) vars[1].value<void *>()
                , *(long*) vars[2].value<void *>());
    }
    return QVariant();
}

QVariant LogRecorderHelperPlugin::stopMethod(const QVariantList &vars)
{
    stop();
    return QVariant();
}

void LogRecorderHelperPlugin::stop()
{
    recordStopPressed();
}

int LogRecorderHelperPlugin::writingThreadProcess(void *args)
{
    LogRecorderHelperPlugin* obj = static_cast<LogRecorderHelperPlugin*>(args);

    while (obj->m_recorder.isOpen()) {
        if(obj->m_writing_thread.exitThread && obj->m_writeBuffer.isEmpty()) break;
        if(!obj->m_writeBuffer.isEmpty()){
            obj->m_recordMutex.lock();
            SensorRecordData data= obj->m_writeBuffer.dequeue();
            obj->m_recordMutex.unlock();

            obj->m_recorder.addData(data.ranges, data.levels, data.timestamp);

            if (obj->m_recordLimit > 0 && obj->m_recorder.getWritePosition() >= obj->m_recordLimit) {
                break;
            }
        }else{
            QThread::msleep(5);
        }
    }

    return 0;
}

void LogRecorderHelperPlugin::checkDiskSpace()
{
    if (m_recorder.isOpen()) {
        QString filename = m_recorder.getFileName();
        QFileInfo fi(filename);
        double dTotal;
        double dFree;
        QDir dfi(fi.absoluteDir());
        bool valid = getFreeTotalSpace(dfi.rootPath(), dTotal, dFree);
        if (valid) {
//            if ((dFree / 1024) < SettingsWindow::getSetting("minimumFreeSpace", 500)) {
//                emit error(QApplication::applicationName(),
//                           tr("The target disk does'nt have the required %1Mb of free space,\n"
//                              "you have only %2Mb.")
//                           .arg(SettingsWindow::getSetting("minimumFreeSpace", 500)).arg(dFree / 1024));
//                recordStopPressed();
//            }
        }
    }
    else {
        m_diskSapceWatcher.stop();
    }
}

void LogRecorderHelperPlugin::updateUI()
{
    int secs = m_startTimer.elapsed() /1000;
    int mins = (secs / 60) % 60;
    int hours = (secs / 3600);
    secs = secs % 60;

    ui->elapsedTime->setText(QString("%1:%2:%3")
                             .arg(hours, 2, 10, QLatin1Char('0'))
                             .arg(mins, 2, 10, QLatin1Char('0'))
                             .arg(secs, 2, 10, QLatin1Char('0')));
    ui->scansCount->setText(tr("%1 scans").arg(m_recorder.getWritePosition()));
}

void LogRecorderHelperPlugin::addData(const SensorDataArray &range, const SensorDataArray &levels, long timeStamp)
{
    if (m_recording) {
        SensorRecordData data;
        data.ranges = range;
        data.levels = levels;
        data.timestamp = timeStamp;

        m_recordMutex.lock();
        m_writeBuffer.enqueue(data);
        m_recordMutex.unlock();

        emit dataAdded();
    }
}

void LogRecorderHelperPlugin::onUnload()
{

}

void LogRecorderHelperPlugin::onLoad(PluginManagerInterface *manager)
{

}

void LogRecorderHelperPlugin::changeEvent(QEvent *e)
{
    HelperPluginInterface::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        if(ui) ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

