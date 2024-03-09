/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "serial_port.h"
#include <QDebug>

SerialPort::SerialPort(QObject *parent):
    QObject(parent),
    serialPort(new QSerialPort(this))
{
    connect(&timer, &QTimer::timeout, this, &SerialPort::handleTimeout);
}

SerialPort::~SerialPort()
{
    close();
}

int SerialPort::write(const char *buf, int size)
{
    int ret;

    if (!serialPort.isOpen())
    {
        qCritical() << serialPort.portName() << ": 端口未打开";
        return -1;
    }

    if (!size)
        return 0;

    ret = serialPort.write(buf, size);
    if (ret == -1)
    {
        qCritical() << serialPort.portName() << ": 写入错误:" << serialPort.errorString();
        return -1;
    }

    return ret;
}

int SerialPort::asyncRead(char *buf, int size, std::function<void(int)> cb, int timeout)
{
    if (!serialPort.isOpen())
    {
        qCritical() << serialPort.portName() << ": 端口未打开";
        return -1;
    }

    if(timeout)
        timer.start(timeout * 1000);

    buf_pointer = buf;
    buf_index = 0;
    buf_size = size;
    readCb = cb;

    buf_index = serialPort.read(buf, size);

    if(buf_index)
        readCb(buf_index);

    return 0;
}

void SerialPort::handleReadyRead()
{
    buf_index += serialPort.read(&buf_pointer[buf_index], buf_size - buf_index);

    if(timer.isActive() && (buf_index == buf_size))
        timer.stop();

    readCb(buf_index);
}

void SerialPort::handleTimeout()
{
    timer.stop();
    qCritical() << serialPort.portName() << ": 读取超时";
    readCb(-1);
    close();
    emit closed();
}

bool SerialPort::open(const char *portName)
{
    serialPort.setPortName(portName);
    if(serialPort.isOpen())
    {
         qWarning() << serialPort.portName() << ": 端口已打开";
        return false;
    }

    if(serialPort.open(QIODevice::ReadWrite))
    {
        qInfo() << serialPort.portName() << ": 开启";
        connect(&serialPort, &QSerialPort::errorOccurred, this, &SerialPort::handleError);
    }
    else
    {
        qWarning() << serialPort.portName() << ": " << serialPort.errorString();
        serialPort.close();
        return false;
    }

    serialPort.readAll();
    connect(&serialPort, &QSerialPort::readyRead, this, &SerialPort::handleReadyRead);

    return true;
}

void SerialPort::stop()
{
     timer.stop();
}

void SerialPort::close()
{
     timer.stop();

     disconnect(&serialPort, &QSerialPort::errorOccurred, this, &SerialPort::handleError);
     disconnect(&serialPort, &QSerialPort::readyRead, this, &SerialPort::handleReadyRead);

     if(serialPort.isOpen())
    {
        serialPort.close();
        qInfo() << serialPort.portName() << ": 关闭";
    }
}

void SerialPort::handleError(QSerialPort::SerialPortError serialPortError)
{
    if(serialPortError != QSerialPort::NoError)
    {
        qCritical() << serialPort.portName() << ": " << serialPort.errorString();
        close();
        emit closed();
    }
}
