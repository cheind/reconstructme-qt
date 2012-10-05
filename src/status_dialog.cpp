/** @file
  * @copyright Copyright (c) 2012 PROFACTOR GmbH. All rights reserved. 
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted provided that the following conditions are
  * met:
  *
  *     * Redistributions of source code must retain the above copyright
  * notice, this list of conditions and the following disclaimer.
  *     * Redistributions in binary form must reproduce the above
  * copyright notice, this list of conditions and the following disclaimer
  * in the documentation and/or other materials provided with the
  * distribution.
  *     * Neither the name of Google Inc. nor the names of its
  * contributors may be used to endorse or promote products derived from
  * this software without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  * @authors christoph.kopf@profactor.at
  *          florian.eckerstorfer@profactor.at
  */

#include "status_dialog.h"
#include "ui_status_dialog.h"

namespace ReconstructMeGUI {

  status_dialog::status_dialog(QWidget *parent, Qt::WindowFlags f) : 
    QDialog(parent, f),  
    ui(new Ui::status_dialog)
  {
    ui->setupUi(this);

    _status_model = new QStandardItemModel(0, 3, parent);
    _status_model->setHeaderData(0, Qt::Horizontal, tr("Object"));
    _status_model->setHeaderData(1, Qt::Horizontal, tr("Status"));
    _status_model->setHeaderData(2, Qt::Horizontal, tr("Message"));

    ui->statustableView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->statustableView->horizontalHeader()->setStretchLastSection(true);
    ui->statustableView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);    
   
    ui->statustableView->setModel(_status_model);
    ui->statustableView->horizontalHeader()->setSortIndicator(1, Qt::AscendingOrder);

    create_content();
  }

  void status_dialog::create_content() {
    QList< QStandardItem *> items;
    
    _sen_obj_item = new QStandardItem("Sensor");
    _sen_status_item = new QStandardItem(style()->standardIcon(QStyle::SP_MessageBoxQuestion),"");
    _sen_message_item = new QStandardItem("");

    _sen_obj_item->setEditable(false);
    _sen_status_item->setEditable(false);
    _sen_message_item->setEditable(false);

    items.push_back(_sen_obj_item);
    items.push_back(_sen_status_item);
    items.push_back(_sen_message_item);

    _status_model->appendRow(items);
    
    //============================
    QList< QStandardItem *> items2;
    
    _dev_obj_item = new QStandardItem("Device");
    _dev_status_item = new QStandardItem(style()->standardIcon(QStyle::SP_DialogOkButton), QString());
    _dev_message_item = new QStandardItem("");

    _dev_obj_item->setEditable(false);
    _dev_status_item->setEditable(false);
    _dev_message_item->setEditable(false);

    items2.push_back(_dev_obj_item);
    items2.push_back(_dev_status_item);
    items2.push_back(_dev_message_item);

    _status_model->appendRow(items2);

    //============================
    QList< QStandardItem *> items3;
    
    _lic_obj_item = new QStandardItem("License");
    _lic_status_item = new QStandardItem(style()->standardIcon(QStyle::SP_MessageBoxWarning),"");
    _lic_message_item = new QStandardItem("");

    _lic_obj_item->setEditable(false);
    _lic_status_item->setEditable(false);
    _lic_message_item->setEditable(false);

    items3.push_back(_lic_obj_item);
    items3.push_back(_lic_status_item);
    items3.push_back(_lic_message_item);

    _status_model->appendRow(items3);
  }

  status_dialog::~status_dialog() 
  {
    delete ui;
  }

  void status_dialog::initializing(init_t what) {
    QIcon icon(style()->standardIcon(QStyle::SP_DialogResetButton));
    QString message("Initializing, please wait...");

    switch (what) {
      case OPENCL:
        _dev_status_item->setIcon(icon);
        _dev_message_item->setText(message);
        break;
      case SENSOR:
        _sen_status_item->setIcon(icon);
        _sen_message_item->setText(message);
        break;
      case LICENSE:
        _lic_status_item->setIcon(icon);
        _lic_message_item->setText(message);
        break;
    }

    this->show();
  }
  
  void status_dialog::initialized(init_t what, bool success) {
    QIcon icon;
    QString message;
    if (success) {
      icon = style()->standardIcon(QStyle::SP_MessageBoxInformation);
      message = "Successfully initialized";
    }
    else {
      icon = style()->standardIcon(QStyle::SP_MessageBoxCritical);
      message = "Error occured";
    }

    switch (what) {
      case OPENCL:
        _dev_status_item->setIcon(icon);
        _dev_message_item->setText(message);
        break;
      case SENSOR:
        _sen_status_item->setIcon(icon);
        _sen_message_item->setText(message);
        break;
      case LICENSE:
        _lic_status_item->setIcon(icon);
        _lic_message_item->setText(message);
        break;
    }
    
    this->show();
  }
}