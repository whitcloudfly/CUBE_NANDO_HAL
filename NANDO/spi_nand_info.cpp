/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "spi_nand_info.h"

typedef struct __attribute__((__packed__))
{
    uint32_t spare_offset;
    uint8_t mode_data;
    uint8_t unlock_data;
    uint8_t ecc_err_bits_mask;
    uint8_t ecc_err_bits_state;
    uint8_t read_dummy_prepend;
    uint8_t plane_select_have;
    uint8_t die_select_type;
    uint32_t freq;
} SpiNandConf;

SpiNandInfo::SpiNandInfo()
{
    hal = CHIP_HAL_SPI_NAND;
}

SpiNandInfo::~SpiNandInfo()
{
}

const QByteArray &SpiNandInfo::getHalConf()
{
    SpiNandConf conf;
    conf.spare_offset = getPageSize();
    conf.mode_data = static_cast<uint8_t>(params[CHIP_PARAM_MODE_DATA]);
    conf.unlock_data = static_cast<uint8_t>(params[CHIP_PARAM_UNLOCK_DATA]);
    conf.ecc_err_bits_mask = static_cast<uint8_t>(params[CHIP_PARAM_ECC_ERR_BITS_MASK]);
    conf.ecc_err_bits_state = static_cast<uint8_t>(params[CHIP_PARAM_ECC_ERR_BITS_STATE]);
    conf.read_dummy_prepend = static_cast<uint8_t>(params[CHIP_PARAM_READ_DUMMY_PREPEND]);
    conf.plane_select_have = static_cast<uint8_t>(params[CHIP_PARAM_PLANE_SELECT_HAVE]);
    conf.die_select_type = static_cast<uint8_t>(params[CHIP_PARAM_DIE_SELECT_TYPE]);
    conf.freq = params[CHIP_PARAM_FREQ];

    halConf.clear();
    halConf.append(reinterpret_cast<const char *>(&conf), sizeof(conf));

    return halConf;
}

quint64 SpiNandInfo::getParam(uint32_t num)
{
    if (num >= CHIP_PARAM_NUM)
        return 0;

    return params[num];
}

int SpiNandInfo::setParam(uint32_t num, quint64 value)
{
    if (num >= CHIP_PARAM_NUM)
        return -1;

    params[num] = value;

    return 0;
}
