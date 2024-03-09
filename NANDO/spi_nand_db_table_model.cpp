/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "spi_nand_db_table_model.h"
#include <limits>

#define CHIP_DB_TABLE_MODEL_MIN_CYCLES 1
#define CHIP_DB_TABLE_MODEL_MAX_CYCLES 4

SpiNandDbTableModel::SpiNandDbTableModel(SpiNandDb *chipDb,
    QObject *parent) : QAbstractTableModel(parent)
{
    this->chipDb = chipDb;
}

int SpiNandDbTableModel::rowCount(const QModelIndex & /*parent*/) const
{
    return chipDb->size();
}

int SpiNandDbTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    return SpiNandDb::CHIP_PARAM_NUM;
}

QVariant SpiNandDbTableModel::data(const QModelIndex &index, int role) const
{
    int column;
    QString paramStr;

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    column = index.column();
    switch (column)
    {
    case SpiNandDb::CHIP_PARAM_NAME:
        return chipDb->getChipName(index.row());
    case SpiNandDb::CHIP_PARAM_PAGE_SIZE:
        chipDb->getHexStringFromParam(chipDb->getPageSize(index.row()),
            paramStr);
        return paramStr;
    case SpiNandDb::CHIP_PARAM_BLOCK_SIZE:
        chipDb->getHexStringFromParam(chipDb->getBlockSize(index.row()),
            paramStr);
        return paramStr;
    case SpiNandDb::CHIP_PARAM_TOTAL_SIZE:
        chipDb->getHexStringFromParam(chipDb->getTotalSize(index.row()),
            paramStr);
        return paramStr;
    case SpiNandDb::CHIP_PARAM_SPARE_SIZE:
        chipDb->getHexStringFromParam(chipDb->getSpareSize(index.row()),
            paramStr);
        return paramStr;
    case SpiNandDb::CHIP_PARAM_MODE_DATA:
        chipDb->getHexStringFromOptParam(chipDb->getChipParam(index.row(),
            SpiNandInfo::CHIP_PARAM_MODE_DATA), paramStr);
        return paramStr;
    case SpiNandDb::CHIP_PARAM_UNLOCK_DATA:
        chipDb->getHexStringFromOptParam(chipDb->getChipParam(index.row(),
            SpiNandInfo::CHIP_PARAM_UNLOCK_DATA), paramStr);
        return paramStr;
    case SpiNandDb::CHIP_PARAM_ECC_ERR_BITS_MASK:
        chipDb->getHexStringFromOptParam(chipDb->getChipParam(index.row(),
            SpiNandInfo::CHIP_PARAM_ECC_ERR_BITS_MASK), paramStr);
        return paramStr;
    case SpiNandDb::CHIP_PARAM_ECC_ERR_BITS_STATE:
        chipDb->getHexStringFromOptParam(chipDb->getChipParam(index.row(),
            SpiNandInfo::CHIP_PARAM_ECC_ERR_BITS_STATE), paramStr);
        return paramStr;
    case SpiNandDb::CHIP_PARAM_READ_DUMMY_PREPEND:
        chipDb->getHexStringFromOptParam(chipDb->getChipParam(index.row(),
            SpiNandInfo::CHIP_PARAM_READ_DUMMY_PREPEND), paramStr);
        return paramStr;
    case SpiNandDb::CHIP_PARAM_PLANE_SELECT_HAVE:
        chipDb->getHexStringFromOptParam(chipDb->getChipParam(index.row(),
            SpiNandInfo::CHIP_PARAM_PLANE_SELECT_HAVE), paramStr);
        return paramStr;
    case SpiNandDb::CHIP_PARAM_DIE_SELECT_TYPE:
        chipDb->getHexStringFromOptParam(chipDb->getChipParam(index.row(),
            SpiNandInfo::CHIP_PARAM_DIE_SELECT_TYPE), paramStr);
        return paramStr;
    case SpiNandDb::CHIP_PARAM_FREQ:
        return (uint)chipDb->getChipParam(index.row(),
            SpiNandInfo::CHIP_PARAM_FREQ);
    case SpiNandDb::CHIP_PARAM_ID1:
        chipDb->getHexStringFromParam(chipDb->getChipParam(index.row(),
            SpiNandInfo::CHIP_PARAM_ID1), paramStr);
        return paramStr;
    case SpiNandDb::CHIP_PARAM_ID2:
        chipDb->getHexStringFromParam(chipDb->getChipParam(index.row(),
            SpiNandInfo::CHIP_PARAM_ID2), paramStr);
        return paramStr;
    case SpiNandDb::CHIP_PARAM_ID3:
        chipDb->getHexStringFromOptParam(chipDb->getChipParam(index.row(),
            SpiNandInfo::CHIP_PARAM_ID3), paramStr);
        return paramStr;
    case SpiNandDb::CHIP_PARAM_ID4:
        chipDb->getHexStringFromOptParam(chipDb->getChipParam(index.row(),
            SpiNandInfo::CHIP_PARAM_ID4), paramStr);
        return paramStr;
    case SpiNandDb::CHIP_PARAM_ID5:
        chipDb->getHexStringFromOptParam(chipDb->getChipParam(index.row(),
            SpiNandInfo::CHIP_PARAM_ID5), paramStr);
        return paramStr;
    }

    return QVariant();
}

QVariant SpiNandDbTableModel::headerData(int section,
    Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case SpiNandDb::CHIP_PARAM_NAME: return tr("Chip name");
        case SpiNandDb::CHIP_PARAM_PAGE_SIZE: return tr("Page\nsize");
        case SpiNandDb::CHIP_PARAM_BLOCK_SIZE: return tr("Block\nsize");
        case SpiNandDb::CHIP_PARAM_TOTAL_SIZE: return tr("Total\nsize");
        case SpiNandDb::CHIP_PARAM_SPARE_SIZE: return tr("Spare\nsize");
        case SpiNandDb::CHIP_PARAM_MODE_DATA: return tr("Mode\ndata");
        case SpiNandDb::CHIP_PARAM_UNLOCK_DATA: return tr("Unlock\ndata");
        case SpiNandDb::CHIP_PARAM_ECC_ERR_BITS_MASK: return tr("ECC err\nmask");
        case SpiNandDb::CHIP_PARAM_ECC_ERR_BITS_STATE: return tr("ECC err\nstate");
        case SpiNandDb::CHIP_PARAM_READ_DUMMY_PREPEND: return tr("Read\nprepend");
        case SpiNandDb::CHIP_PARAM_PLANE_SELECT_HAVE: return tr("Plane\nhave");
        case SpiNandDb::CHIP_PARAM_DIE_SELECT_TYPE: return tr("Die select\ntype");
        case SpiNandDb::CHIP_PARAM_FREQ: return tr("Frequency\n(kHz)");
        case SpiNandDb::CHIP_PARAM_ID1: return tr("ID 1");
        case SpiNandDb::CHIP_PARAM_ID2: return tr("ID 2");
        case SpiNandDb::CHIP_PARAM_ID3: return tr("ID 3");
        case SpiNandDb::CHIP_PARAM_ID4: return tr("ID 4");
        case SpiNandDb::CHIP_PARAM_ID5: return tr("ID 5");
        }
    }

    if (role == Qt::ToolTipRole)
    {
        switch (section)
        {
        case SpiNandDb::CHIP_PARAM_NAME:
            return tr("Chip name");
        case SpiNandDb::CHIP_PARAM_PAGE_SIZE:
            return tr("Page size in bytes");
        case SpiNandDb::CHIP_PARAM_BLOCK_SIZE:
            return tr("Block size in bytes");
        case SpiNandDb::CHIP_PARAM_TOTAL_SIZE:
            return tr("Total size in bytes");
        case SpiNandDb::CHIP_PARAM_SPARE_SIZE:
            return tr("User spare part size in bytes");
        case SpiNandDb::CHIP_PARAM_MODE_DATA:
            return tr("Data byte for mode registr");
        case SpiNandDb::CHIP_PARAM_UNLOCK_DATA:
            return tr("Data byte for unlock registr");
        case SpiNandDb::CHIP_PARAM_ECC_ERR_BITS_MASK:
            return tr("ECC error bits mask.");
        case SpiNandDb::CHIP_PARAM_ECC_ERR_BITS_STATE:
            return tr("ECC error bits state.");
        case SpiNandDb::CHIP_PARAM_READ_DUMMY_PREPEND:
            return tr("Dummy mode prepend flag for read.");
        case SpiNandDb::CHIP_PARAM_PLANE_SELECT_HAVE:
            return tr("True if plane select have.");
        case SpiNandDb::CHIP_PARAM_DIE_SELECT_TYPE:
            return tr("0 or die select type number. (0/1/2)");
        case SpiNandDb::CHIP_PARAM_FREQ:
            return tr("Maximum supported SPI frequency in kHz");
        case SpiNandDb::CHIP_PARAM_ID1:
            return tr("Chip ID 1st byte");
        case SpiNandDb::CHIP_PARAM_ID2:
            return tr("Chip ID 2nd byte");
        case SpiNandDb::CHIP_PARAM_ID3:
            return tr("Chip ID 3rd byte");
        case SpiNandDb::CHIP_PARAM_ID4:
            return tr("Chip ID 4th byte");
        case SpiNandDb::CHIP_PARAM_ID5:
            return tr("Chip ID 5th byte");
        }
    }

    return QVariant();
}

Qt::ItemFlags SpiNandDbTableModel::flags (const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool SpiNandDbTableModel::setData(const QModelIndex &index,
    const QVariant &value, int role)
{
    quint64 paramVal;

    if (role != Qt::EditRole)
        return false;

    switch (index.column())
    {
    case SpiNandDb::CHIP_PARAM_NAME:
        chipDb->setChipName(index.row(), value.toString());
        return true;
    case SpiNandDb::CHIP_PARAM_PAGE_SIZE:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        chipDb->setPageSize(index.row(), paramVal);
        return true;
    case SpiNandDb::CHIP_PARAM_BLOCK_SIZE:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        chipDb->setBlockSize(index.row(), paramVal);
        return true;
    case SpiNandDb::CHIP_PARAM_TOTAL_SIZE:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        chipDb->setTotalSize(index.row(), paramVal);
        return true;
    case SpiNandDb::CHIP_PARAM_SPARE_SIZE:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        chipDb->setSpareSize(index.row(), paramVal);
        return true;
    case SpiNandDb::CHIP_PARAM_MODE_DATA:
        if (chipDb->getOptParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isOptParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiNandInfo::CHIP_PARAM_MODE_DATA,
            paramVal);
        return true;
    case SpiNandDb::CHIP_PARAM_UNLOCK_DATA:
        if (chipDb->getOptParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isOptParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiNandInfo::CHIP_PARAM_UNLOCK_DATA,
            paramVal);
        return true;
    case SpiNandDb::CHIP_PARAM_ECC_ERR_BITS_MASK:
        if (chipDb->getOptParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isOptParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiNandInfo::CHIP_PARAM_ECC_ERR_BITS_MASK,
                             paramVal);
        return true;
    case SpiNandDb::CHIP_PARAM_ECC_ERR_BITS_STATE:
        if (chipDb->getOptParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isOptParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiNandInfo::CHIP_PARAM_ECC_ERR_BITS_STATE,
                             paramVal);
        return true;
    case SpiNandDb::CHIP_PARAM_READ_DUMMY_PREPEND:
        if (chipDb->getOptParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isOptParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiNandInfo::CHIP_PARAM_READ_DUMMY_PREPEND,
                             paramVal);
        return true;
    case SpiNandDb::CHIP_PARAM_PLANE_SELECT_HAVE:
        if (chipDb->getOptParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isOptParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiNandInfo::CHIP_PARAM_PLANE_SELECT_HAVE,
                             paramVal);
        return true;
    case SpiNandDb::CHIP_PARAM_DIE_SELECT_TYPE:
        if (chipDb->getOptParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isOptParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiNandInfo::CHIP_PARAM_DIE_SELECT_TYPE,
                             paramVal);
        return true;
    case SpiNandDb::CHIP_PARAM_FREQ:
        if (chipDb->getParamFromString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, 0x00, 0xFFFFFFFF))
            return false;
        chipDb->setChipParam(index.row(), SpiNandInfo::CHIP_PARAM_FREQ,
            paramVal);
        return true;
    case SpiNandDb::CHIP_PARAM_ID1:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiNandInfo::CHIP_PARAM_ID1,
            paramVal);
        return true;
    case SpiNandDb::CHIP_PARAM_ID2:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiNandInfo::CHIP_PARAM_ID2,
            paramVal);
        return true;
    case SpiNandDb::CHIP_PARAM_ID3:
        if (chipDb->getOptParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isOptParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiNandInfo::CHIP_PARAM_ID3,
            paramVal);
        return true;
    case SpiNandDb::CHIP_PARAM_ID4:
        if (chipDb->getOptParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isOptParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiNandInfo::CHIP_PARAM_ID4,
            paramVal);
        return true;
    case SpiNandDb::CHIP_PARAM_ID5:
        if (chipDb->getOptParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isOptParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiNandInfo::CHIP_PARAM_ID5,
            paramVal);
        return true;
    }

    return false;
}

void SpiNandDbTableModel::addRow()
{
    SpiNandInfo *chipInfo = new SpiNandInfo();

    beginResetModel();
    chipDb->addChip(chipInfo);
    endResetModel();
}

void SpiNandDbTableModel::delRow(int index)
{
    beginResetModel();
    chipDb->delChip(index);
    endResetModel();
}

void SpiNandDbTableModel::commit()
{
    chipDb->commit();
}

void SpiNandDbTableModel::reset()
{
    beginResetModel();
    chipDb->reset();
    endResetModel();
}

