/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "spi_nand_db_dialog.h"
#include "ui_spi_nand_db_dialog.h"

//#define HEADER_LONG_WIDTH 120
//#define HEADER_MED_WIDTH 110
//#define HEADER_SHORT_WIDTH 50

SpiNandDbDialog::SpiNandDbDialog(SpiNandDb *chipDb, QWidget *parent) :
    QDialog(parent), ui(new Ui::SpiNandDbDialog),
    chipDbTableModel(chipDb, parent)
{
    ui->setupUi(this);

#ifdef Q_OS_WIN32
    QFont font("Courier New", 10);
    ui->chipDbTableView->setFont(font);
#endif

    chipDbProxyModel.setSourceModel(&chipDbTableModel);
    ui->chipDbTableView->setModel(&chipDbProxyModel);
//    ui->chipDbTableView->setColumnWidth(SpiNandDb::CHIP_PARAM_NAME,
//        HEADER_LONG_WIDTH);
//    ui->chipDbTableView->setColumnWidth(SpiNandDb::CHIP_PARAM_PAGE_SIZE,
//        HEADER_MED_WIDTH);
//    ui->chipDbTableView->setColumnWidth(SpiNandDb::CHIP_PARAM_BLOCK_SIZE,
//        HEADER_MED_WIDTH);
//    ui->chipDbTableView->setColumnWidth(SpiNandDb::CHIP_PARAM_TOTAL_SIZE,
//        HEADER_MED_WIDTH);
//    ui->chipDbTableView->setColumnWidth(SpiNandDb::CHIP_PARAM_SPARE_SIZE,
//        HEADER_MED_WIDTH);
//    for (int i = SpiNandDb::CHIP_PARAM_NAME;
//         i <= SpiNandDb::CHIP_PARAM_NUM - 1; i++)
//    {
//        //ui->chipDbTableView->setColumnWidth(i, HEADER_MED_WIDTH);
//        //ui->chipDbTableView->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
//        //ui->chipDbTableView->horizontalHeader()->setMinimumSectionSize(-1);

//    }
        ui->chipDbTableView->resizeColumnsToContents();

    connect(ui->addChipDbButton, SIGNAL(clicked()), this,
        SLOT(slotAddChipDbButtonClicked()));
    connect(ui->delChipDbButton, SIGNAL(clicked()), this,
        SLOT(slotDelChipDbButtonClicked()));
    connect(ui->okCancelButtonBox->button(QDialogButtonBox::Ok),
        SIGNAL(clicked()), this, SLOT(slotOkButtonClicked()));
    connect(ui->okCancelButtonBox->button(QDialogButtonBox::Cancel),
        SIGNAL(clicked()), this, SLOT(slotCancelButtonClicked()));
}

SpiNandDbDialog::~SpiNandDbDialog()
{
    delete ui;
}

void SpiNandDbDialog::slotAddChipDbButtonClicked()
{
    chipDbTableModel.addRow();
}

void SpiNandDbDialog::slotDelChipDbButtonClicked()
{
    QModelIndexList selection = ui->chipDbTableView->selectionModel()->
        selectedRows();

    if (!selection.count())
        return;

    chipDbTableModel.delRow(selection.at(0).row());
}

void SpiNandDbDialog::slotOkButtonClicked()
{
    chipDbTableModel.commit();
}

void SpiNandDbDialog::slotCancelButtonClicked()
{
    chipDbTableModel.reset();
}
