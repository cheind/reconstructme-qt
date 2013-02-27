/** @file
  * @copyright Copyright (c) 2013 PROFACTOR GmbH. All rights reserved. 
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
  *     * Neither the name of Profactor GmbH nor the names of its
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

#include <QStandardItemModel>
#include <QStandardItem>
#include <QPushButton>
#include <QTimer>

#include <iostream>

namespace ReconstructMeGUI {

  status_dialog::status_dialog(std::shared_ptr<reme_resource_manager> rm, QWidget *parent, Qt::WindowFlags f) : 
    QDialog(parent, f),
    _rm(rm),
    _ui(new Ui::status_dialog),
    _has_license(false)
  {
    _ui->setupUi(this);

    _status_model = new QStandardItemModel(0, 3, parent);
    _status_model->setHeaderData(0, Qt::Horizontal, tr("Object"));
    _status_model->setHeaderData(1, Qt::Horizontal, tr("Status"));
    _status_model->setHeaderData(2, Qt::Horizontal, tr("Message"));

    _ui->statustableView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    _ui->statustableView->horizontalHeader()->setStretchLastSection(true);
    _ui->statustableView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);    
   
    _ui->statustableView->setModel(_status_model);
    _ui->statustableView->horizontalHeader()->setSortIndicator(1, Qt::AscendingOrder);

    setModal(true);

    connect(_ui->closeBtn, SIGNAL(clicked()), SLOT(hide()));
    connect(_ui->onlineHelpBtn, SIGNAL(clicked()), SLOT(hide()));
    _ui->closeBtn->connect(_rm.get(), SIGNAL(initializing_sdk()), SLOT(hide()));
    _ui->closeBtn->connect(_rm.get(), SIGNAL(sdk_initialized(bool)), SLOT(show()));
    
    connect(_rm.get(), SIGNAL(initializing(init_t)), SLOT(initializing(init_t)), Qt::BlockingQueuedConnection);
    connect(_rm.get(), SIGNAL(initialized(init_t, bool)), SLOT(initialized(init_t, bool)), Qt::BlockingQueuedConnection);
    connect(_rm.get(), SIGNAL(initializing_sdk()), SLOT(reset()));
    connect(_rm.get(), SIGNAL(initializing_sdk()), SLOT(show()));

    create_content();
    reset();
  }

  void status_dialog::create_content() {
    QList< QStandardItem *> sensor_items;
    
    _sen_obj_item = new QStandardItem("3D Capture Sensor");
    _sen_status_item = new QStandardItem();
    _sen_message_item = new QStandardItem();

    _sen_obj_item->setEditable(false);
    _sen_status_item->setEditable(false);
    _sen_message_item->setEditable(false);

    sensor_items.push_back(_sen_obj_item);
    sensor_items.push_back(_sen_status_item);
    sensor_items.push_back(_sen_message_item);

    //============================
    QList< QStandardItem *> device_items;
    
    _dev_obj_item = new QStandardItem("Processing Device");
    _dev_status_item = new QStandardItem();
    _dev_message_item = new QStandardItem();

    _dev_obj_item->setEditable(false);
    _dev_status_item->setEditable(false);
    _dev_message_item->setEditable(false);

    device_items.push_back(_dev_obj_item);
    device_items.push_back(_dev_status_item);
    device_items.push_back(_dev_message_item);

    //============================
    QList< QStandardItem *> license_items;
    
    _lic_obj_item = new QStandardItem("License");
    _lic_status_item = new QStandardItem();
    _lic_message_item = new QStandardItem();

    _lic_obj_item->setEditable(false);
    _lic_status_item->setEditable(false);
    _lic_message_item->setEditable(false);

    license_items.push_back(_lic_obj_item);
    license_items.push_back(_lic_status_item);
    license_items.push_back(_lic_message_item);

    _status_model->appendRow(license_items);
    _status_model->appendRow(device_items);
    _status_model->appendRow(sensor_items);
    
    _t = new QTimer(this);
    _t->setInterval(250);
    _t->setSingleShot(true);
    _ui->closeBtn->connect(_t, SIGNAL(timeout()), SLOT(click()));
  }

  status_dialog::~status_dialog() {
    delete _ui;
  }

  void status_dialog::reset() {
    QIcon icon("");
    QString message("Waiting for initialization...");

    _dev_status_item->setIcon(icon);
    _dev_message_item->setText(message);
    _sen_status_item->setIcon(icon);
    _sen_message_item->setText(message);
    _lic_status_item->setIcon(icon);
    _lic_message_item->setText(message);

    _has_license = false;
    _has_sensor  = false;
    _has_device  = false;
  }

  void status_dialog::initializing(init_t what) {
    QIcon icon(":/images/status_refresh.png");

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
      icon.swap(QIcon(":/images/status_ok.png"));
      message = "Successfully initialized";
    }
    else {
      icon.swap(QIcon(":/images/status_error.png"));
      message = "Failed to initialize";
    }

    switch (what) {
      case OPENCL:
        _dev_status_item->setIcon(icon);
        _dev_message_item->setText(message);
        _has_device = success;
        break;
      case SENSOR:
        _sen_status_item->setIcon(icon);
        _sen_message_item->setText(message);
        _has_sensor = success;
        break;
      case LICENSE:
        _lic_status_item->setIcon(icon);
        _has_license = success;
        if (success) 
          _lic_message_item->setText("License applied successfully");
        else 
          _lic_message_item->setText("Unlicensed");
        break;
    }

    if (_has_license && _has_sensor && _has_device) {
      _t->start();
    }
  }

  const QPushButton *status_dialog::closeBtn() {
    return _ui->closeBtn;
  }

  QPushButton *status_dialog::onlineHelpBtn() {
    return _ui->onlineHelpBtn;
  }

  bool status_dialog::licensed() {
    return _has_license;
  }
}