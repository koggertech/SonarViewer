#ifndef CORE_H
#define CORE_H

#include <QObject>
#include <connection.h>
#include <console.h>
#include <SonarDriverInterface.h>
#include <QUrl>
#include <QQmlApplicationEngine>
#include <waterfall.h>


#ifdef FLASHER
#include "flasher.h"
#endif

class Core : public QObject
{
    Q_OBJECT

public:
    explicit Core();

    Q_PROPERTY(ConsoleListModel* consoleList READ consoleList CONSTANT)

    ConsoleListModel* consoleList() {
        return m_console->listModel();
    }

    Console *console() {
        return m_console;
    }

    void consoleInfo(QString msg) {
        console()->put(QtMsgType::QtInfoMsg, msg);
    }

    void consoleWarning(QString msg) {
        console()->put(QtMsgType::QtWarningMsg, msg);
    }

    void consoleProto(ProtoBin &parser);

    void setEngine(QQmlApplicationEngine *engine) {
        m_engine = engine;
    }

public slots:
    QList<QSerialPortInfo> availableSerial();
    QStringList availableSerialName();
    bool openConnectionAsSerial(const QString &name, int baudrate, bool mode);
    bool openConnectionAsFile(const QString &name);
    bool isOpenConnection();
    bool closeConnection();
    QString deviceName();
    long deviceSerialNumber();

    bool upgradeFW(const QString &name);

#ifdef FLASHER
    bool factoryFlash(const QString &name, int sn, QString pn);
#endif

    void setPlotStartLevel(int level) {
        m_plot->setStartLevel(level);
    }

    void setPlotStopLevel(int level) {
        m_plot->setStopLevel(level);
    }

    void setTimelinePosition(double position) {
        m_plot->setTimelinePosition(position);
    }

    void setChartVis(bool visible) {
        m_plot->setChartVis(visible);
    }

    void setOscVis(bool visible) {
        m_plot->setOscVis(visible);
    }

    void setDistVis(bool visible) {
        m_plot->setDistVis(visible);
    }

    void UILoad(QObject *object, const QUrl &url);

signals:
    void connectionChanged(bool duplex = false);

public:
    Console *m_console;
    Connection *m_connection;
    SonarDriverInterface *dev_driver;
    PlotCash* m_plot;
    WaterFall* m_waterFall;

    QQmlApplicationEngine *m_engine = nullptr;

#ifdef FLASHER
    Flasher flasher;
#endif

private slots:
    void closing();

#ifdef FLASHER
    void flasherConnectionChanged(Flasher::BootState connection_status);
#endif

protected:
    QByteArray fw_data;

};

#endif // CORE_H
