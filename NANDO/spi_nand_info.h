/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef SPI_NAND_INFO_H
#define SPI_NAND_INFO_H

#include "chip_info.h"

class SpiNandInfo : public ChipInfo
{
public:
    enum
    {
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
        CHIP_PARAM_ID6,
        CHIP_PARAM_NUM,
    };

private:
    QByteArray halConf;
    quint64 params[CHIP_PARAM_NUM] = {};

public:
    SpiNandInfo();
    virtual ~SpiNandInfo();
    const QByteArray &getHalConf() override;
    quint64 getParam(uint32_t num) override;
    int setParam(uint32_t num, quint64 value) override;
};

#endif // SPI_NAND_INFO_H
