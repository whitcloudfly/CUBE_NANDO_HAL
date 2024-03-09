/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "spi_nor_db_table_model.h"
#include <limits>

#define CHIP_DB_TABLE_MODEL_MIN_CYCLES 1
#define CHIP_DB_TABLE_MODEL_MAX_CYCLES 4

SpiNorDbTableModel::SpiNorDbTableModel(SpiNorDb *chipDb,
    QObject *parent) : QAbstractTableModel(parent)
{
    this->chipDb = chipDb;
}

int SpiNorDbTableModel::rowCount(const QModelIndex & /*parent*/) const
{
    return chipDb->size();
}

int SpiNorDbTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    return SpiNorDb::CHIP_PARAM_NUM;
}

QVariant SpiNorDbTableModel::data(const QModelIndex &index, int role) const
{
    int column;
    QString paramStr;

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    column = index.column();
    switch (column)
    {
    case SpiNorDb::CHIP_PARAM_NAME:
        return chipDb->getChipName(index.row());
    case SpiNorDb::CHIP_PARAM_PAGE_SIZE:
        chipDb->getHexStringFromParam(chipDb->getPageSize(index.row()),
            paramStr);
        return paramStr;
    case SpiNorDb::CHIP_PARAM_BLOCK_SIZE:
        chipDb->getHexStringFromParam(chipDb->getBlockSize(index.row()),
            paramStr);
        return paramStr;
    case SpiNorDb::CHIP_PARAM_TOTAL_SIZE:
        chipDb->getHexStringFromParam(chipDb->getTotalSize(index.row()),
            paramStr);
        return paramStr;
    case SpiNorDb::CHIP_PARAM_PAGE_OFF:
        return (uint)chipDb->getChipParam(index.row(),
            SpiNorInfo::CHIP_PARAM_PAGE_OFF);
    case SpiNorDb::CHIP_PARAM_READ_CMD:
        chipDb->getHexStringFromParam(chipDb->getChipParam(index.row(),
            SpiNorInfo::CHIP_PARAM_READ_CMD), paramStr);
        return paramStr;
    case SpiNorDb::CHIP_PARAM_READ_ID_CMD:
        chipDb->getHexStringFromParam(chipDb->getChipParam(index.row(),
            SpiNorInfo::CHIP_PARAM_READ_ID_CMD), paramStr);
        return paramStr;
    case SpiNorDb::CHIP_PARAM_WRITE_CMD:
        chipDb->getHexStringFromParam(chipDb->getChipParam(index.row(),
            SpiNorInfo::CHIP_PARAM_WRITE_CMD), paramStr);
        return paramStr;
    case SpiNorDb::CHIP_PARAM_WRITE_EN_CMD:
        chipDb->getHexStringFromOptParam(chipDb->getChipParam(index.row(),
            SpiNorInfo::CHIP_PARAM_WRITE_EN_CMD), paramStr);
        return paramStr;
    case SpiNorDb::CHIP_PARAM_ERASE_CMD:
        chipDb->getHexStringFromParam(chipDb->getChipParam(index.row(),
            SpiNorInfo::CHIP_PARAM_ERASE_CMD), paramStr);
        return paramStr;
    case SpiNorDb::CHIP_PARAM_STATUS_CMD:
        chipDb->getHexStringFromParam(chipDb->getChipParam(index.row(),
            SpiNorInfo::CHIP_PARAM_STATUS_CMD), paramStr);
        return paramStr;
    case SpiNorDb::CHIP_PARAM_BUSY_BIT:
        return (uint)chipDb->getChipParam(index.row(),
            SpiNorInfo::CHIP_PARAM_BUSY_BIT);
    case SpiNorDb::CHIP_PARAM_BUSY_STATE:
        return (uint)chipDb->getChipParam(index.row(),
            SpiNorInfo::CHIP_PARAM_BUSY_STATE);
    case SpiNorDb::CHIP_PARAM_FREQ:
        return (uint)chipDb->getChipParam(index.row(),
            SpiNorInfo::CHIP_PARAM_FREQ);
    case SpiNorDb::CHIP_PARAM_ID1:
        chipDb->getHexStringFromParam(chipDb->getChipParam(index.row(),
            SpiNorInfo::CHIP_PARAM_ID1), paramStr);
        return paramStr;
    case SpiNorDb::CHIP_PARAM_ID2:
        chipDb->getHexStringFromParam(chipDb->getChipParam(index.row(),
            SpiNorInfo::CHIP_PARAM_ID2), paramStr);
        return paramStr;
    case SpiNorDb::CHIP_PARAM_ID3:
        chipDb->getHexStringFromOptParam(chipDb->getChipParam(index.row(),
            SpiNorInfo::CHIP_PARAM_ID3), paramStr);
        return paramStr;
    case SpiNorDb::CHIP_PARAM_ID4:
        chipDb->getHexStringFromOptParam(chipDb->getChipParam(index.row(),
            SpiNorInfo::CHIP_PARAM_ID4), paramStr);
        return paramStr;
    case SpiNorDb::CHIP_PARAM_ID5:
        chipDb->getHexStringFromOptParam(chipDb->getChipParam(index.row(),
            SpiNorInfo::CHIP_PARAM_ID5), paramStr);
        return paramStr;
    }

    return QVariant();
}

QVariant SpiNorDbTableModel::headerData(int section,
    Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case SpiNorDb::CHIP_PARAM_NAME: return tr("Name");
        case SpiNorDb::CHIP_PARAM_PAGE_SIZE: return tr("Page size");
        case SpiNorDb::CHIP_PARAM_BLOCK_SIZE: return tr("Block size");
        case SpiNorDb::CHIP_PARAM_TOTAL_SIZE: return tr("Total size");
        case SpiNorDb::CHIP_PARAM_PAGE_OFF: return tr("Page off.");
        case SpiNorDb::CHIP_PARAM_READ_CMD: return tr("Read com.");
        case SpiNorDb::CHIP_PARAM_READ_ID_CMD: return tr("Read ID com.");
        case SpiNorDb::CHIP_PARAM_WRITE_CMD: return tr("Write com.");
        case SpiNorDb::CHIP_PARAM_WRITE_EN_CMD: return tr("Write en. com.");
        case SpiNorDb::CHIP_PARAM_ERASE_CMD: return tr("Erase com.");
        case SpiNorDb::CHIP_PARAM_STATUS_CMD: return tr("Status com.");
        case SpiNorDb::CHIP_PARAM_BUSY_BIT: return tr("Busy bit");
        case SpiNorDb::CHIP_PARAM_BUSY_STATE: return tr("Busy bit state");
        case SpiNorDb::CHIP_PARAM_FREQ: return tr("Freq. (kHz)");
        case SpiNorDb::CHIP_PARAM_ID1: return tr("ID 1");
        case SpiNorDb::CHIP_PARAM_ID2: return tr("ID 2");
        case SpiNorDb::CHIP_PARAM_ID3: return tr("ID 3");
        case SpiNorDb::CHIP_PARAM_ID4: return tr("ID 4");
        case SpiNorDb::CHIP_PARAM_ID5: return tr("ID 5");
        }
    }

    if (role == Qt::ToolTipRole)
    {
        switch (section)
        {
        case SpiNorDb::CHIP_PARAM_NAME:
            return tr("Chip name");
        case SpiNorDb::CHIP_PARAM_PAGE_SIZE:
            return tr("Page size in bytes");
        case SpiNorDb::CHIP_PARAM_BLOCK_SIZE:
            return tr("Block size in bytes");
        case SpiNorDb::CHIP_PARAM_TOTAL_SIZE:
            return tr("Total size in bytes");
        case SpiNorDb::CHIP_PARAM_PAGE_OFF:
            return tr("Page offset in address");
        case SpiNorDb::CHIP_PARAM_READ_CMD:
            return tr("Page read command");
        case SpiNorDb::CHIP_PARAM_READ_ID_CMD:
            return tr("Read ID command");
        case SpiNorDb::CHIP_PARAM_WRITE_CMD:
            return tr("Page write command");
        case SpiNorDb::CHIP_PARAM_WRITE_EN_CMD:
            return tr("Write enable command");
        case SpiNorDb::CHIP_PARAM_ERASE_CMD:
            return tr("Block erase command");
        case SpiNorDb::CHIP_PARAM_STATUS_CMD:
            return tr("Read status command");
        case SpiNorDb::CHIP_PARAM_BUSY_BIT:
            return tr("Busy bit number (0-7) in status register");
        case SpiNorDb::CHIP_PARAM_BUSY_STATE:
            return tr("Busy bit active state (0/1)");
        case SpiNorDb::CHIP_PARAM_FREQ:
            return tr("Maximum supported SPI frequency in kHz");
        case SpiNorDb::CHIP_PARAM_ID1:
            return tr("Chip ID 1st byte");
        case SpiNorDb::CHIP_PARAM_ID2:
            return tr("Chip ID 2nd byte");
        case SpiNorDb::CHIP_PARAM_ID3:
            return tr("Chip ID 3rd byte");
        case SpiNorDb::CHIP_PARAM_ID4:
            return tr("Chip ID 4th byte");
        case SpiNorDb::CHIP_PARAM_ID5:
            return tr("Chip ID 5th byte");
        }
    }

    return QVariant();
}

Qt::ItemFlags SpiNorDbTableModel::flags (const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool SpiNorDbTableModel::setData(const QModelIndex &index,
    const QVariant &value, int role)
{
    quint64 paramVal;

    if (role != Qt::EditRole)
        return false;

    switch (index.column())
    {
    case SpiNorDb::CHIP_PARAM_NAME:
        chipDb->setChipName(index.row(), value.toString());
        return true;
    case SpiNorDb::CHIP_PARAM_PAGE_SIZE:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        chipDb->setPageSize(index.row(), paramVal);
        return true;
    case SpiNorDb::CHIP_PARAM_BLOCK_SIZE:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        chipDb->setBlockSize(index.row(), paramVal);
        return true;
    case SpiNorDb::CHIP_PARAM_TOTAL_SIZE:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        chipDb->setTotalSize(index.row(), paramVal);
        return true;
    case SpiNorDb::CHIP_PARAM_PAGE_OFF:
        if (chipDb->getParamFromString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, 0, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiNorInfo::CHIP_PARAM_PAGE_OFF,
            paramVal);
        return true;
    case SpiNorDb::CHIP_PARAM_READ_CMD:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiNorInfo::CHIP_PARAM_READ_CMD,
            paramVal);
        return true;
    case SpiNorDb::CHIP_PARAM_READ_ID_CMD:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiNorInfo::CHIP_PARAM_READ_ID_CMD,
            paramVal);
        return true;
    case SpiNorDb::CHIP_PARAM_WRITE_CMD:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiNorInfo::CHIP_PARAM_WRITE_CMD,
            paramVal);
        return true;
    case SpiNorDb::CHIP_PARAM_WRITE_EN_CMD:
        if (chipDb->getOptParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isOptParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiNorInfo::CHIP_PARAM_WRITE_EN_CMD,
            paramVal);
        return true;
    case SpiNorDb::CHIP_PARAM_ERASE_CMD:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiNorInfo::CHIP_PARAM_ERASE_CMD,
            paramVal);
        return true;
    case SpiNorDb::CHIP_PARAM_STATUS_CMD:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiNorInfo::CHIP_PARAM_STATUS_CMD,
            paramVal);
        return true;
    case SpiNorDb::CHIP_PARAM_BUSY_BIT:
        if (chipDb->getParamFromString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, 0, 7))
            return false;
        chipDb->setChipParam(index.row(), SpiNorInfo::CHIP_PARAM_BUSY_BIT,
            paramVal);
        return true;
    case SpiNorDb::CHIP_PARAM_BUSY_STATE:
        if (chipDb->getParamFromString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, 0, 1))
            return false;
        chipDb->setChipParam(index.row(), SpiNorInfo::CHIP_PARAM_BUSY_STATE,
            paramVal);
        return true;
    case SpiNorDb::CHIP_PARAM_FREQ:
        if (chipDb->getParamFromString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, 0x00, 0xFFFFFFFF))
            return false;
        chipDb->setChipParam(index.row(), SpiNorInfo::CHIP_PARAM_FREQ,
            paramVal);
        return true;
    case SpiNorDb::CHIP_PARAM_ID1:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiNorInfo::CHIP_PARAM_ID1,
            paramVal);
        return true;
    case SpiNorDb::CHIP_PARAM_ID2:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiNorInfo::CHIP_PARAM_ID2,
            paramVal);
        return true;
    case SpiNorDb::CHIP_PARAM_ID3:
        if (chipDb->getOptParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isOptParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiNorInfo::CHIP_PARAM_ID3,
            paramVal);
        return true;
    case SpiNorDb::CHIP_PARAM_ID4:
        if (chipDb->getOptParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isOptParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiNorInfo::CHIP_PARAM_ID4,
            paramVal);
        return true;
    case SpiNorDb::CHIP_PARAM_ID5:
        if (chipDb->getOptParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isOptParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiNorInfo::CHIP_PARAM_ID5,
            paramVal);
        return true;
    }

    return false;
}

void SpiNorDbTableModel::addRow()
{
    SpiNorInfo *chipInfo = new SpiNorInfo();

    beginResetModel();
    chipDb->addChip(chipInfo);
    endResetModel();
}

void SpiNorDbTableModel::delRow(int index)
{
    beginResetModel();
    chipDb->delChip(index);
    endResetModel();
}

void SpiNorDbTableModel::commit()
{
    chipDb->commit();
}

void SpiNorDbTableModel::reset()
{
    beginResetModel();
    chipDb->reset();
    endResetModel();
}

