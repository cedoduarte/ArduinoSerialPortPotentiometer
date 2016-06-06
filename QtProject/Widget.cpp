#include "Widget.h"
#include "ui_Widget.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QDebug>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    configureArduino();

    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(1023);
    ui->progressBar->setValue(0);

    connect(arduino, &QSerialPort::readyRead, [&]() {
        auto lectura = arduino->readAll();
        if (lectura.endsWith("\r")) {
            const int value = lectura.mid(0, lectura.indexOf("\r")).toInt();
            ui->progressBar->setValue(value);
        }
    });
}

Widget::~Widget()
{
    if (arduino->isOpen()) {
        arduino->close();
    }
    delete ui;
}

void Widget::configureArduino()
{
    arduino_is_available = false;
    arduino_port_name = "";
    arduino = new QSerialPort(this);

    qDebug() << "Number of available ports: "
             << QSerialPortInfo::availablePorts().length();
    foreach (const QSerialPortInfo &serialPortInfo,
             QSerialPortInfo::availablePorts())
    {
        qDebug() << "Has vendor ID: " << serialPortInfo.hasVendorIdentifier();
        if (serialPortInfo.hasVendorIdentifier()) {
            qDebug() << "Vendor ID: " << serialPortInfo.vendorIdentifier();
        }
        qDebug() << "Has Product ID: " << serialPortInfo.hasProductIdentifier();
        if (serialPortInfo.hasProductIdentifier()) {
            qDebug() << "Product ID: " << serialPortInfo.productIdentifier();
        }
    }


    foreach (const QSerialPortInfo &serialPortInfo,
            QSerialPortInfo::availablePorts())
    {
        if(serialPortInfo.hasVendorIdentifier()
                && serialPortInfo.hasProductIdentifier())
        {
            if(serialPortInfo.vendorIdentifier() == arduino_uno_vendor_id) {
                if(serialPortInfo.productIdentifier()
                        == arduino_uno_product_id)
                {
                    arduino_port_name = serialPortInfo.portName();
                    arduino_is_available = true;
                }
            }
        }
    }

    if (arduino_is_available) {
        // open and configure the serialport
        arduino->setPortName(arduino_port_name);
        arduino->open(QSerialPort::ReadOnly);
        arduino->setBaudRate(QSerialPort::Baud9600);
        arduino->setDataBits(QSerialPort::Data8);
        arduino->setParity(QSerialPort::NoParity);
        arduino->setStopBits(QSerialPort::OneStop);
        arduino->setFlowControl(QSerialPort::NoFlowControl);
    } else {
        // give error message if not available
        QMessageBox::warning(this, "Port error", "Couldn't find the Arduino!");
    }
}
