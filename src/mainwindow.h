#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "openvrthread.h"

const QString default_config_fname = "openvr_config.cfg";

struct StartupOptions {
    bool hasSamplingRate = false;
    int samplingRate = 0;
    bool hasOriginIndex = false;
    int originIndex = 0;
    bool autoScan = false;
    bool autoStart = false;
    bool streamAllDevices = true;
    QList<int> streamDeviceIndices;
    QString configOutFile;
};

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0,
                        const QString config_file = default_config_fname,
                        const StartupOptions& startup_options = StartupOptions());
    ~MainWindow();

private slots:

    void on_actionLoad_Configuration_triggered();
    void on_actionSave_Configuration_triggered();
    void update_connect_label(bool status);
    void update_list_devices(QStringList deviceList);
    void update_stream_button(bool status);

    void on_pushButton_scan_clicked();

    void on_pushButton_stream_clicked();

private:
    void load_config(const QString filename);
    void save_config(const QString filename);
    void apply_startup_options();

    Ui::MainWindow *ui;
    OpenVRThread m_thread;
    StartupOptions m_startupOptions;
    bool m_hasAutoStarted;
};

#endif // MAINWINDOW_H
