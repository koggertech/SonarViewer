#ifndef SONARDRIVERINTERFACE_H
#define SONARDRIVERINTERFACE_H

#include "SonarDriver.h"

class SonarDriverInterface : public SonarDriver
{
    Q_OBJECT
public:
    explicit SonarDriverInterface(QObject *parent = nullptr) :
        SonarDriver(parent){
    }

    Q_PROPERTY(int distMax READ distMax WRITE setDistMax NOTIFY distSetupChanged)
    Q_PROPERTY(int distDeadZone READ distDeadZone WRITE setDistDeadZone NOTIFY distSetupChanged)
    Q_PROPERTY(int distConfidence READ distConfidence WRITE setConfidence NOTIFY distSetupChanged)

    Q_PROPERTY(int chartSamples READ chartSamples WRITE setChartSamples NOTIFY chartSetupChanged)
    Q_PROPERTY(int chartResolution READ chartResolution WRITE setChartResolution NOTIFY chartSetupChanged)
    Q_PROPERTY(int chartOffset READ chartOffset WRITE setChartOffset NOTIFY chartSetupChanged)

    Q_PROPERTY(int datasetDist READ datasetDist WRITE setDatasetDist NOTIFY datasetChanged)
    Q_PROPERTY(int datasetChart READ datasetChart WRITE setDatasetChart NOTIFY datasetChanged)
    Q_PROPERTY(int datasetTemp READ datasetTemp WRITE setDatasetTemp NOTIFY datasetChanged)
    Q_PROPERTY(int datasetSDDBT READ datasetSDDBT WRITE setDatasetSDDBT NOTIFY datasetChanged)
    Q_PROPERTY(int datasetSDDBT_P2 READ datasetSDDBT_P2 WRITE setDatasetSDDBT_P2 NOTIFY datasetChanged)
    Q_PROPERTY(int ch1Period READ ch1Period WRITE setCh1Period NOTIFY datasetChanged)
    Q_PROPERTY(int ch2Period READ ch2Period WRITE setCh2Period NOTIFY datasetChanged)

    Q_PROPERTY(int transFreq READ transFreq WRITE setTransFreq NOTIFY transChanged)
    Q_PROPERTY(int transPulse READ transPulse WRITE setTransPulse NOTIFY transChanged)
    Q_PROPERTY(int transBoost READ transBoost WRITE setTransBoost NOTIFY transChanged)

    Q_PROPERTY(int soundSpeed READ soundSpeed WRITE setSoundSpeed NOTIFY soundChanged)

    Q_PROPERTY(int busAddress READ getBusAddress WRITE setBusAddress NOTIFY UARTChanged)
    Q_PROPERTY(int devAddress READ getDevAddress WRITE setDevAddress NOTIFY UARTChanged)
    Q_PROPERTY(int devDefAddress READ getDevDefAddress WRITE setDevDefAddress NOTIFY UARTChanged)

    Q_PROPERTY(int upgradeFWStatus READ upgradeFWStatus NOTIFY upgradeProgressChanged)

signals:

public slots:

private:
};



#endif // SONARDRIVERINTERFACE_H
