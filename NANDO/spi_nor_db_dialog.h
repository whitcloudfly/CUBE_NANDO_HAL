/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef SPI_NOR_DB_DALOG_H
#define SPI_NOR_DB_DALOG_H

#include "spi_nor_db_table_model.h"
#include <QDialog>
#include <QSortFilterProxyModel>

namespace Ui {
class SpiNorDbDialog;
}

class SpiNorDbDialog : public QDialog
{
    Q_OBJECT

    Ui::SpiNorDbDialog *ui;
    SpiNorDbTableModel chipDbTableModel;
    QSortFilterProxyModel chipDbProxyModel;

public:
    explicit SpiNorDbDialog(SpiNorDb *chipDb, QWidget *parent = nullptr);
    ~SpiNorDbDialog();

private slots:
    void slotAddChipDbButtonClicked();
    void slotDelChipDbButtonClicked();
    void slotOkButtonClicked();
    void slotCancelButtonClicked();
};

#endif // SPI_NOR_DB_DALOG_H
