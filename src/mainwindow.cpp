#include <QFileDialog>
#include <QtXml>
#include <QDebug>
#include <QSet>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent, const QString config_file, const StartupOptions& startup_options)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_startupOptions(startup_options)
    , m_hasAutoStarted(false)
{
    ui->setupUi(this);
    load_config(config_file);
    apply_startup_options();
    connect(&m_thread, SIGNAL(openvrConnected(bool)), this, SLOT(update_connect_label(bool)));
    connect(&m_thread, SIGNAL(deviceListUpdated(QStringList)), this, SLOT(update_list_devices(QStringList)));
    connect(&m_thread, SIGNAL(outletsStarted(bool)), this, SLOT(update_stream_button(bool)));

    if (!m_startupOptions.configOutFile.isEmpty()) {
        save_config(m_startupOptions.configOutFile);
    }

    if (m_startupOptions.autoScan) {
        on_pushButton_scan_clicked();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::apply_startup_options() {
    if (m_startupOptions.hasSamplingRate) {
        ui->spinBox_sampling_rate->setValue(m_startupOptions.samplingRate);
    }
    if (m_startupOptions.hasOriginIndex) {
        ui->comboBox_origin->setCurrentIndex(m_startupOptions.originIndex);
    }
}

void MainWindow::load_config(const QString filename)
{
    QFile* xmlFile = new QFile(filename);
    if (!xmlFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Could not load XML from file " << filename;
        return;
    }
    QXmlStreamReader* xmlReader = new QXmlStreamReader(xmlFile);
    while(!xmlReader->atEnd() && !xmlReader->hasError()) {
        xmlReader->readNext();
        if(xmlReader->isStartElement() && xmlReader->name() != "settings")
        {
            QStringRef elname = xmlReader->name();
            if (elname == "sampling-rate")
                ui->spinBox_sampling_rate->setValue(xmlReader->readElementText().toInt());
            if (elname == "origin-style")
            {
                ui->comboBox_origin->setCurrentIndex(xmlReader->readElementText().toInt());
            }
        }
    }
    if(xmlReader->hasError()) {
        qDebug() << "Config file parse error "
                 << xmlReader->error()
                 << ": "
                 << xmlReader->errorString();
    }
    xmlReader->clear();
    xmlFile->close();
}

void MainWindow::save_config(const QString filename)
{
    QFile xmlFile(filename);
    if (!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qDebug() << "Could not write XML config file " << filename;
        return;
    }

    QXmlStreamWriter xmlWriter(&xmlFile);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("settings");
    xmlWriter.writeTextElement("sampling-rate", QString::number(ui->spinBox_sampling_rate->value()));
    xmlWriter.writeTextElement("origin-style", QString::number(ui->comboBox_origin->currentIndex()));
    xmlWriter.writeEndElement();
    xmlWriter.writeEndDocument();
    qDebug() << "Saved configuration to" << filename;
}

void MainWindow::on_actionLoad_Configuration_triggered()
{
    QString sel = QFileDialog::getOpenFileName(this,
                                               "Load Configuration File",
                                               "",
                                               "Configuration Files (*.cfg)");
    if (!sel.isEmpty())
    {
        load_config(sel);
    }
}

void MainWindow::on_actionSave_Configuration_triggered()
{
    QString sel = QFileDialog::getSaveFileName(this,"Save Configuration File","","Configuration Files (*.cfg)");
    if (!sel.isEmpty())
    {
        save_config(sel);
    }
}

void MainWindow::update_connect_label(bool status)
{
    if (status)
    {
        ui->label_conn_status->setText("Connected to OpenVR");
        ui->pushButton_stream->setText("Start Streams");
    }
    else
    {
        ui->label_conn_status->setText("Not Connected");
        ui->pushButton_stream->setText("No streams to start.");
    }
}

void MainWindow::update_list_devices(QStringList deviceList)
{
    ui->list_devices->clear();
    ui->list_devices->addItems(deviceList);

    if (!m_startupOptions.streamAllDevices) {
        QSet<int> allowed = QSet<int>::fromList(m_startupOptions.streamDeviceIndices);
        for (int i = 0; i < ui->list_devices->count(); ++i) {
            QListWidgetItem* item = ui->list_devices->item(i);
            QStringList pieces = item->text().split(":");
            int devIx = pieces.value(0).toInt();
            if (allowed.contains(devIx)) {
                item->setSelected(true);
            }
        }
    }

    if (m_startupOptions.autoStart && !m_hasAutoStarted) {
        m_hasAutoStarted = true;
        on_pushButton_stream_clicked();
    }
}

void MainWindow::update_stream_button(bool status)
{
    if (status)
    {
        ui->pushButton_stream->setText("Stop Streams");
    }
    else
    {
        ui->pushButton_stream->setText("Start Streams");
    }
}

void MainWindow::on_pushButton_scan_clicked()
{
    m_thread.initOpenVR(ui->spinBox_sampling_rate->value());
    ui->pushButton_scan->setText("Scanning...");
    ui->pushButton_scan->setDisabled(true);
}

void MainWindow::on_pushButton_stream_clicked()
{
    int originIndex = ui->comboBox_origin->currentIndex();
    QStringList devStringList;
    QList<QListWidgetItem *> lwi = ui->list_devices->selectedItems();
    for( int i=0; i<lwi.count(); ++i )
    {
        devStringList << lwi[i]->text();
    }
    m_thread.startStreams(devStringList, originIndex);
}
