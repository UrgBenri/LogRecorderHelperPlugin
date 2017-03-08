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

#ifndef LOGRECORDERHELPERPLUGIN_H
#define LOGRECORDERHELPERPLUGIN_H

#include "HelperPluginInterface.h"

#include <QMutex>
#include <QTimer>
#include <QTime>
#include <QQueue>
#include <QTranslator>

#include "Thread.h"
#include "UrgDevice.h"
#include "UrgLogHandler.h"

typedef struct recordData{
    SensorDataArray ranges;
    SensorDataArray levels;
    quint64 timestamp;
} SensorRecordData;
Q_DECLARE_METATYPE(SensorRecordData)

namespace Ui {
class LogRecorderHelperPlugin;
}

class LogRecorderHelperPlugin : public HelperPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(HelperPluginInterface)
    Q_PLUGIN_METADATA(IID "org.kristou.UrgBenri.LogRecorderHelperPlugin")
    PLUGIN_FACTORY(LogRecorderHelperPlugin)
public:
    explicit LogRecorderHelperPlugin(QWidget *parent = 0);
    virtual ~LogRecorderHelperPlugin();

    QString pluginName() const{return tr("Log Recorder");}
    QIcon pluginIcon() const {return QIcon(":/LogRecorderHelperPlugin/tabIcon");}
    QString pluginDescription() const {return tr("Log Recorder");}
    PluginVersion pluginVersion() const {return PluginVersion(1, 0, 0);}
    QString pluginAuthorName() const {return "Mehrez Kristou";}
    QString pluginAuthorContact() const {return "mehrez@kristou.com";}
    int pluginLoadOrder() const {return 0;}
    bool pluginIsExperimental() const { return false; }

    QString pluginLicense() const { return "GPL"; }
    QString pluginLicenseDescription() const { return "GPL"; }

    void saveState(QSettings &settings);
    void restoreState(QSettings &settings);

    void loadTranslator(const QString &locale);


public slots:
    void stop();
    void onLoad(PluginManagerInterface *manager);
    void onUnload();

signals:
    void dataAdded();
    void recordStopped();
    void recordStarted();

protected:
    void changeEvent(QEvent* e);

private slots:
    void recordStartPressed();
    void recordStopPressed();

    void checkDiskSpace();
    void updateUI();
private:
    Ui::LogRecorderHelperPlugin *ui;
    UrgLogHandler m_recorder;
    int m_recordLimit;
    bool m_recording;
    QMutex m_recordMutex;
    UrgDevice *m_urg;
    QTimer m_diskSapceWatcher;
    QTime m_startTimer;
    Thread m_writing_thread;
    QQueue<SensorRecordData> m_writeBuffer;
    QTranslator m_translator;

    static int writingThreadProcess(void* args);
    
    void addData(const SensorDataArray &range, const SensorDataArray &levels, long timeStamp);
    void setSensor(UrgDevice *urg);

    //Functions interface
    QVariant setDeviceMethod(const QVariantList &vars);
    QVariant addSensorDataMethod(const QVariantList &vars);
    QVariant stopMethod(const QVariantList &vars);
};

#endif // LOGRECORDERHELPERPLUGIN_H

