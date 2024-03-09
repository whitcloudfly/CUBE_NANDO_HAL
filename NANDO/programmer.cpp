/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "programmer.h"
#include <QDebug>
#include <QTimer>

#ifdef Q_OS_LINUX
  #define USB_DEV_NAME "/dev/ttyACM0"
#else
  #define USB_DEV_NAME "COM1"
#endif

#define READ_TIMEOUT_MS 100
#define ERASE_TIMEOUT_MS 10000
#define WRITE_TIMEOUT_MS 30000

Programmer::Programmer(QObject *parent) : QObject(parent)
{
    usbDevName = USB_DEV_NAME;
    skipBB = true;
    incSpare = false;
    isConn = false;
    QObject::connect(&reader, SIGNAL(log(QtMsgType, QString)), this,
        SLOT(logCb(QtMsgType, QString)));
    QObject::connect(&writer, SIGNAL(log(QtMsgType, QString)), this,
        SLOT(logCb(QtMsgType, QString)));

    QObject::connect(&serialPort, SIGNAL(closed(void)), this,
        SLOT(disconnected(void)));
}

Programmer::~Programmer()
{
    if (isConn)
        disconnect();
}

void Programmer::connectCb(quint64 ret)
{
    serialPort.stop();
    QObject::disconnect(&reader, SIGNAL(result(quint64)), this,
        SLOT(connectCb(quint64)));

    memcpy(&fwVersion, buf.constData(), sizeof(fwVersion));

    if (ret == UINT64_MAX)
    {
        qCritical() << "无法读取固件版本";
        serialPort.close();
        return;
    }

    emit connectCompleted(ret);

    isConn = true;
    qInfo() << "固件版本: " <<
        fwVersionToString(fwVersion).toLatin1().data();
}

void Programmer::disconnected()
{
    isConn = false;
    emit connectCompleted(-1);
}

int Programmer::connect()
{
    Cmd cmd;

    if (!serialPort.open(usbDevName.toLatin1()))
        return -1;

    QObject::connect(&reader, SIGNAL(result(quint64)), this,
        SLOT(connectCb(quint64)));

    cmd.code = CMD_VERSION_GET;

    writeData.clear();
    writeData.append(reinterpret_cast<const char *>(&cmd), sizeof(cmd));
    buf.clear();
    reader.init(&serialPort, &buf,
        sizeof(fwVersion),
        reinterpret_cast<const uint8_t *>(writeData.constData()),
        static_cast<uint32_t>(writeData.size()), false, false);
    reader.start();

    return 0;
}

void Programmer::disconnect()
{
    serialPort.close();
    isConn = false;
}

bool Programmer::isConnected()
{
    return isConn;
}

void Programmer::setUsbDevName(const QString &name)
{
    usbDevName = name;
}

QString Programmer::getUsbDevName()
{
    return usbDevName;
}

bool Programmer::isSkipBB()
{
    return skipBB;
}

void Programmer::setSkipBB(bool skip)
{
    skipBB = skip;
}

bool Programmer::isIncSpare()
{
    return incSpare;
}

void Programmer::setIncSpare(bool isIncSpare)
{
    incSpare = isIncSpare;
}

bool Programmer::isHwEccEnabled()
{
    return enableHwEcc;
}

void Programmer::setHwEccEnabled(bool isHwEccEnabled)
{
    enableHwEcc = isHwEccEnabled;
}

void Programmer::readChipIdCb(quint64 ret)
{
    serialPort.stop();
    QObject::disconnect(&reader, SIGNAL(result(quint64)), this,
        SLOT(readChipIdCb(quint64)));

    memcpy(chipId_p, buf.constData(), sizeof(ChipId));

    emit readChipIdCompleted(ret);
}

void Programmer::readChipId(ChipId *chipId)
{
    Cmd cmd;

    QObject::connect(&reader, SIGNAL(result(quint64)), this,
        SLOT(readChipIdCb(quint64)));

    cmd.code = CMD_NAND_READ_ID;

    writeData.clear();
    writeData.append(reinterpret_cast<const char *>(&cmd), sizeof(cmd));
    chipId_p = chipId;
    buf.clear();
    reader.init(&serialPort, &buf, sizeof(ChipId),
        reinterpret_cast<const uint8_t *>(writeData.constData()),
        static_cast<uint32_t>(writeData.size()), false, false);
    reader.start();
}

void Programmer::eraseChipCb(quint64 ret)
{
    serialPort.stop();
    QObject::disconnect(&reader, SIGNAL(progress(quint64)), this,
        SLOT(eraseProgressChipCb(quint64)));
    QObject::disconnect(&reader, SIGNAL(result(quint64)), this,
        SLOT(eraseChipCb(quint64)));
    emit eraseChipCompleted(ret);
}

void Programmer::eraseProgressChipCb(quint64 progress)
{
    emit eraseChipProgress(progress);
}

void Programmer::eraseChip(quint64 addr, quint64 len)
{
    EraseCmd eraseCmd;

    QObject::connect(&reader, SIGNAL(result(quint64)), this,
        SLOT(eraseChipCb(quint64)));
    QObject::connect(&reader, SIGNAL(progress(quint64)), this,
        SLOT(eraseProgressChipCb(quint64)));

    eraseCmd.cmd.code = CMD_NAND_ERASE;
    eraseCmd.addr = addr;
    eraseCmd.len = len;
    eraseCmd.flags.skipBB = skipBB;
    eraseCmd.flags.incSpare = incSpare;

    writeData.clear();
    writeData.append(reinterpret_cast<const char *>(&eraseCmd),
        sizeof(eraseCmd));
    reader.init(&serialPort, nullptr, 0,
        reinterpret_cast<const uint8_t *>(writeData.constData()),
        static_cast<uint32_t>(writeData.size()), skipBB, false);
    reader.start();
}

void Programmer::readCb(quint64 ret)
{
    serialPort.stop();
    QObject::disconnect(&reader, SIGNAL(progress(quint64)), this,
        SLOT(readProgressCb(quint64)));
    QObject::disconnect(&reader, SIGNAL(result(quint64)), this,
        SLOT(readCb(quint64)));
    emit readChipCompleted(ret);
}

void Programmer::readProgressCb(quint64 progress)
{
    emit readChipProgress(progress);
}

void Programmer::readChip(QVector<uint8_t> *buf, quint64 addr, quint64 len,
    bool isReadLess)
{
    ReadCmd readCmd;

    QObject::connect(&reader, SIGNAL(result(quint64)), this,
        SLOT(readCb(quint64)));
    QObject::connect(&reader, SIGNAL(progress(quint64)), this,
        SLOT(readProgressCb(quint64)));

    readCmd.cmd.code = CMD_NAND_READ;
    readCmd.addr = addr;
    readCmd.len = len;
    readCmd.flags.skipBB = skipBB;
    readCmd.flags.incSpare = incSpare;

    writeData.clear();
    writeData.append(reinterpret_cast<const char *>(&readCmd), sizeof(readCmd));
    reader.init(&serialPort, buf, len,
        reinterpret_cast<const uint8_t *>(writeData.constData()),
        static_cast<uint32_t>(writeData.size()), skipBB,
        isReadLess);
    reader.start();
}

void Programmer::writeCb(int ret)
{
    serialPort.stop();
    QObject::disconnect(&writer, SIGNAL(progress(quint64)), this,
        SLOT(writeProgressCb(quint64)));
    QObject::disconnect(&writer, SIGNAL(result(int)), this, SLOT(writeCb(int)));
    emit writeChipCompleted(ret);
}

void Programmer::writeProgressCb(quint64 progress)
{
    emit writeChipProgress(progress);
}

void Programmer::writeChip(QVector<uint8_t> *buf, quint64 addr, quint64 len,
    uint32_t pageSize)
{
    QObject::connect(&writer, SIGNAL(result(int)), this, SLOT(writeCb(int)));
    QObject::connect(&writer, SIGNAL(progress(quint64)), this,
        SLOT(writeProgressCb(quint64)));

    writer.init(&serialPort, buf, addr, len, pageSize,
        skipBB, incSpare, enableHwEcc, CMD_NAND_WRITE_S, CMD_NAND_WRITE_D,
        CMD_NAND_WRITE_E);
    writer.start();
}

void Programmer::readChipBadBlocksCb(quint64 ret)
{
    serialPort.stop();
    QObject::disconnect(&reader, SIGNAL(result(quint64)), this,
        SLOT(readChipBadBlocksCb(quint64)));
    QObject::disconnect(&reader, SIGNAL(progress(quint64)), this,
        SLOT(readChipBadBlocksProgressCb(quint64)));
    emit readChipBadBlocksCompleted(ret);
}

void Programmer::readChipBadBlocksProgressCb(quint64 progress)
{
    emit readChipBadBlocksProgress(progress);
}

void Programmer::readChipBadBlocks()
{
    Cmd cmd;

    QObject::connect(&reader, SIGNAL(result(quint64)), this,
        SLOT(readChipBadBlocksCb(quint64)));
    QObject::connect(&reader, SIGNAL(progress(quint64)), this,
        SLOT(readChipBadBlocksProgressCb(quint64)));

    cmd.code = CMD_NAND_READ_BB;

    writeData.clear();
    writeData.append(reinterpret_cast<const char *>(&cmd), sizeof(cmd));
    reader.init(&serialPort, nullptr, 0,
        reinterpret_cast<const uint8_t *>(writeData.constData()),
        static_cast<uint32_t>(writeData.size()), false, false);
    reader.start();
}

void Programmer::confChipCb(quint64 ret)
{
    serialPort.stop();
    QObject::disconnect(&reader, SIGNAL(result(quint64)), this,
        SLOT(confChipCb(quint64)));
    emit confChipCompleted(ret);
}

void Programmer::confChip(ChipInfo *chipInfo)
{
    ConfCmd confCmd;

    confCmd.cmd.code = CMD_NAND_CONF;
    confCmd.hal = chipInfo->getHal();
    confCmd.pageSize = chipInfo->getPageSize();
    confCmd.blockSize = chipInfo->getBlockSize();
    confCmd.totalSize = chipInfo->getTotalSize();
    confCmd.spareSize = chipInfo->getSpareSize();
    confCmd.bbMarkOff = chipInfo->getBBMarkOffset();

    QObject::connect(&reader, SIGNAL(result(quint64)), this,
        SLOT(confChipCb(quint64)));

    writeData.clear();
    writeData.append(reinterpret_cast<const char *>(&confCmd), sizeof(confCmd));
    writeData.append(chipInfo->getHalConf());
    reader.init(&serialPort, nullptr, 0,
        reinterpret_cast<const uint8_t *>(writeData.constData()),
        static_cast<uint32_t>(writeData.size()), false, false);
    reader.start();
}

void Programmer::logCb(QtMsgType msgType, QString msg)
{
    switch (msgType)
    {
    case QtDebugMsg:
        qDebug() << msg;
        break;
    case QtInfoMsg:
        qInfo() << msg;
        break;
    case QtWarningMsg:
        qWarning() << msg;
        break;
    case QtCriticalMsg:
        qCritical() << msg;
        break;
    default:
        break;
    }
}

QString Programmer::fwVersionToString(FwVersion fwVersion)
{
    return QString("%1.%2.%3").arg(fwVersion.major).
        arg(fwVersion.minor).arg(fwVersion.build);
}

int Programmer::firmwareImageRead()
{
    qint64 ret, fileSize;
    uint32_t updateImageSize, updateImageOffset;

    QFile file(firmwareFileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        qCritical() << "无法打开文件:" << firmwareFileName <<
            ", 错误:" << file.errorString();
        goto Error;
    }

    fileSize = file.size();
    if (fileSize != (firmwareImage[FIRMWARE_IMAGE_LAST - 1].offset +
        firmwareImage[FIRMWARE_IMAGE_LAST - 1].size))
    {
        qCritical() << "固件镜像 " << firmwareFileName <<
            " 大小 " << fileSize << "无效，应为 " << 256 * 1024;
        goto Error;
    }

    updateImage = activeImage == FIRMWARE_IMAGE_1 ? FIRMWARE_IMAGE_2 :
        FIRMWARE_IMAGE_1;
    updateImageSize = firmwareImage[updateImage].size;
    updateImageOffset = firmwareImage[updateImage].offset;
    buf.resize(updateImageSize);

    if (!file.seek(updateImageOffset))
    {
        qCritical() << "无法查找固件镜像 " << firmwareFileName;
        goto Error;
    }

    if ((ret = file.read((char *)buf.data(), updateImageSize)) < 0)
    {
        qCritical() << "无法读取固件镜像 " << firmwareFileName <<
            ", 错误:" << file.errorString();
        goto Error;
    }

    if (ret != updateImageSize)
    {
        qCritical() << "固件镜像 " << firmwareFileName <<
            " 已部分读取，长度" << ret;
        goto Error;
    }

    return 0;

Error:
    buf.clear();
    emit firmwareUpdateCompleted(-1);
    return -1;
}

void Programmer::firmwareUpdateProgressCb(quint64 progress)
{
    emit firmwareUpdateProgress(progress * 100ULL /
        firmwareImage[updateImage].size);
    memcpy(buf.data(), buf.data() + progress, flashPageSize);
}

void Programmer::firmwareUpdateCb(int ret)
{
    serialPort.stop();
    QObject::disconnect(&writer, SIGNAL(progress(quint64)), this,
        SLOT(firmwareUpdateProgressCb(quint64)));
    QObject::disconnect(&writer, SIGNAL(result(int)), this,
        SLOT(firmwareUpdateCb(int)));

    buf.clear();

    emit firmwareUpdateCompleted(ret);
}

void Programmer::firmwareUpdateStart()
{
    if (firmwareImageRead())
    {
        emit firmwareUpdateCompleted(-1);
        return;
    }

    QObject::connect(&writer, SIGNAL(result(int)), this,
        SLOT(firmwareUpdateCb(int)));
    QObject::connect(&writer, SIGNAL(progress(quint64)), this,
        SLOT(firmwareUpdateProgressCb(quint64)));

    writer.init(&serialPort, &buf,
        firmwareImage[updateImage].address, firmwareImage[updateImage].size,
        flashPageSize, 0, 0, 0, CMD_FW_UPDATE_S, CMD_FW_UPDATE_D,
        CMD_FW_UPDATE_E);
    writer.start();
}

void Programmer::getActiveImageCb(quint64 ret)
{
    serialPort.stop();
    QObject::disconnect(&reader, SIGNAL(result(quint64)), this,
        SLOT(getActiveImageCb(quint64)));

    activeImage = buf[0];

    if (ret == UINT64_MAX)
    {
        qCritical() << "无法获取活动固件镜像";
        goto Error;
    }

    if (activeImage >= FIRMWARE_IMAGE_LAST)
    {
        qCritical() << "错误的活动固件镜像: " << activeImage;
        goto Error;
    }

    qInfo() << "活动固件镜像: " << activeImage;

    // Wait reader stop
    QTimer::singleShot(50, this, &Programmer::firmwareUpdateStart);
    return;

Error:
    emit firmwareUpdateCompleted(-1);
}

void Programmer::firmwareUpdate(const QString &fileName)
{
    Cmd cmd;

    firmwareFileName = fileName;

    QObject::connect(&reader, SIGNAL(result(quint64)), this,
        SLOT(getActiveImageCb(quint64)));

    cmd.code = CMD_ACTIVE_IMAGE_GET;

    writeData.clear();
    writeData.append(reinterpret_cast<const char *>(&cmd), sizeof(cmd));
    buf.clear();
    reader.init(&serialPort, &buf,
        sizeof(activeImage),
        reinterpret_cast<const uint8_t *>(writeData.constData()),
        static_cast<uint32_t>(writeData.size()), false, false);
    reader.start();
}
