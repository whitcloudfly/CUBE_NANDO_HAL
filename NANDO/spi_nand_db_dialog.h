/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef SPI_NAND_DB_DALOG_H
#define SPI_NAND_DB_DALOG_H

#include "spi_nand_db_table_model.h"
#include <QDialog>
#include <QSortFilterProxyModel>

namespace Ui {
class SpiNandDbDialog;
}

class SpiNandDbDialog : public QDialog
{
    Q_OBJECT

    Ui::SpiNandDbDialog *ui;
    SpiNandDbTableModel chipDbTableModel;
    QSortFilterProxyModel chipDbProxyModel;

public:
    explicit SpiNandDbDialog(SpiNandDb *chipDb, QWidget *parent = nullptr);
    ~SpiNandDbDialog();

private slots:
    void slotAddChipDbButtonClicked();
    void slotDelChipDbButtonClicked();
    void slotOkButtonClicked();
    void slotCancelButtonClicked();
};

#endif // SPI_NAND_DB_DALOG_H
