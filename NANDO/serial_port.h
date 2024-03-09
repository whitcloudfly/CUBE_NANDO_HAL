/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include <QObject>
#include <QSerialPort>
#include <QTimer>

class SerialPort : public QObject
{
    Q_OBJECT

public:
    explicit SerialPort(QObject *parent = nullptr);
    ~SerialPort();

    bool open(const char *portName);
    void stop();
    void close();

    int write(const char *buf, int size);
    int asyncRead(char *buf, int size, std::function<void(int)> cb, int timeout);

private:
    int buf_size = 0;
    int buf_index = 0;
    char *buf_pointer = nullptr;
    std::function<void(int)> readCb;
    QSerialPort serialPort;
    QTimer timer;

private slots:
    void handleReadyRead();
    void handleTimeout();
    void handleError(QSerialPort::SerialPortError error);

signals:
    void closed();
};

#endif // SERIAL_PORT_H
