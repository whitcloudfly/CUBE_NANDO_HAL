/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef SPI_NAND_DB_H
#define SPI_NAND_DB_H

#include "chip_db.h"
#include "spi_nand_info.h"

#include <cstdint>
#include <QString>
#include <QObject>
#include <QVector>
#include <QFile>

class SpiNandDb : public ChipDb
{
private:
    QString dbFileName = "nando_spi_nand_db.csv";

protected:
    QString getDbFileName() override;
    ChipInfo *stringToChipInfo(const QString &s) override;
    int chipInfoToString(ChipInfo *chipInfo, QString &s) override;

public:
    enum
    {
        CHIP_PARAM_NAME,
        CHIP_PARAM_PAGE_SIZE,
        CHIP_PARAM_BLOCK_SIZE,
        CHIP_PARAM_TOTAL_SIZE,
        CHIP_PARAM_SPARE_SIZE,
        CHIP_PARAM_MODE_DATA,
        CHIP_PARAM_UNLOCK_DATA,
        CHIP_PARAM_ECC_ERR_BITS_MASK,
        CHIP_PARAM_ECC_ERR_BITS_STATE,
        CHIP_PARAM_READ_DUMMY_PREPEND,
        CHIP_PARAM_PLANE_SELECT_HAVE,
        CHIP_PARAM_DIE_SELECT_TYPE,
        CHIP_PARAM_FREQ,
        CHIP_PARAM_ID1,
        CHIP_PARAM_ID2,
        CHIP_PARAM_ID3,
        CHIP_PARAM_ID4,
        CHIP_PARAM_ID5,
        CHIP_PARAM_NUM
    };

    explicit SpiNandDb();
    virtual ~SpiNandDb();

    ChipInfo *chipInfoGetByName(QString name);
    int getIdByChipId(uint32_t id1, uint32_t id2, uint32_t id3, uint32_t id4,
        uint32_t id5);
    QString getNameByChipId(uint32_t id1, uint32_t id2,
        uint32_t id3, uint32_t id4, uint32_t id5, uint32_t id6) override;
    quint64 getChipParam(int chipIndex, int paramIndex);
    int setChipParam(int chipIndex, int paramIndex, quint64 paramValue);
};

#endif // SPI_NAND_DB_H
