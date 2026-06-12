#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>

static bool parseBoolEnv(const QString& value) {
    QString v = value.trimmed().toLower();
    return v == "1" || v == "true" || v == "yes" || v == "on";
}

static int parseOrigin(const QString& origin, bool* ok) {
    QString o = origin.trimmed().toLower();
    if (o == "0" || o == "seated") {
        *ok = true;
        return 0;
    }
    if (o == "1" || o == "standing") {
        *ok = true;
        return 1;
    }
    if (o == "2" || o == "raw") {
        *ok = true;
        return 2;
    }
    *ok = false;
    return 0;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCommandLineParser parser;
    parser.setApplicationDescription("LabStreamingLayer interface for OpenVR.");
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption configFileOption(QStringList() << "c" << "config",
                                        QCoreApplication::translate("main", "Load configuration from <config>."),
                                        QCoreApplication::translate("main", "config"),
                                        default_config_fname);
    parser.addOption(configFileOption);

    QCommandLineOption sampleRateOption(QStringList() << "sampling-rate", "Sampling rate in Hz.", "hz");
    parser.addOption(sampleRateOption);
    QCommandLineOption originOption(QStringList() << "origin", "Tracking origin: seated|standing|raw (or 0|1|2).", "origin");
    parser.addOption(originOption);
    QCommandLineOption devicesOption(QStringList() << "devices", "Comma-separated device indices to stream, or 'all'.", "devices");
    parser.addOption(devicesOption);
    QCommandLineOption autoScanOption(QStringList() << "auto-scan", "Automatically scan for devices at startup.");
    parser.addOption(autoScanOption);
    QCommandLineOption autoStartOption(QStringList() << "auto-start", "Automatically start streams after first device scan.");
    parser.addOption(autoStartOption);
    QCommandLineOption writeConfigOption(QStringList() << "write-config", "Write effective startup settings to <config> and continue.", "config");
    parser.addOption(writeConfigOption);

    parser.process(a);

    StartupOptions startupOptions;

    bool ok = false;
    QString samplingEnv = qEnvironmentVariable("OPENVR_SAMPLING_RATE");
    if (!samplingEnv.isEmpty()) {
        int srate = samplingEnv.toInt(&ok);
        if (ok) {
            startupOptions.hasSamplingRate = true;
            startupOptions.samplingRate = srate;
        }
    }
    if (parser.isSet(sampleRateOption)) {
        int srate = parser.value(sampleRateOption).toInt(&ok);
        if (ok) {
            startupOptions.hasSamplingRate = true;
            startupOptions.samplingRate = srate;
        }
    }

    QString originEnv = qEnvironmentVariable("OPENVR_ORIGIN_STYLE");
    if (!originEnv.isEmpty()) {
        int originIx = parseOrigin(originEnv, &ok);
        if (ok) {
            startupOptions.hasOriginIndex = true;
            startupOptions.originIndex = originIx;
        }
    }
    if (parser.isSet(originOption)) {
        int originIx = parseOrigin(parser.value(originOption), &ok);
        if (ok) {
            startupOptions.hasOriginIndex = true;
            startupOptions.originIndex = originIx;
        }
    }

    QString devices = qEnvironmentVariable("OPENVR_STREAM_DEVICES");
    if (parser.isSet(devicesOption)) {
        devices = parser.value(devicesOption);
    }
    if (!devices.isEmpty()) {
        QString d = devices.trimmed().toLower();
        if (d != "all") {
            startupOptions.streamAllDevices = false;
            QStringList parts = devices.split(",", Qt::SkipEmptyParts);
            for (const QString& part : parts) {
                int devIx = part.trimmed().toInt(&ok);
                if (ok) startupOptions.streamDeviceIndices.append(devIx);
            }
        }
    }

    startupOptions.autoScan = parser.isSet(autoScanOption) || parseBoolEnv(qEnvironmentVariable("OPENVR_AUTO_SCAN"));
    startupOptions.autoStart = parser.isSet(autoStartOption) || parseBoolEnv(qEnvironmentVariable("OPENVR_AUTO_START"));

    QString configOut = qEnvironmentVariable("OPENVR_CONFIG_OUT");
    if (parser.isSet(writeConfigOption)) {
        configOut = parser.value(writeConfigOption);
    }
    startupOptions.configOutFile = configOut;

    QString configFilename = parser.value(configFileOption);
    MainWindow w(0, configFilename, startupOptions);
    w.show();

    return a.exec();
}
