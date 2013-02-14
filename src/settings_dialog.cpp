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
  
#include "settings_dialog.h"
#include "ui_settings_dialog.h"
#include "settings.h"
#include "strings.h"
#include "opencl_info.pb.h"

#include <QSettings>
#include <QFileDialog>
#include <QFile>
#include <QStringList>
#include <QFileSystemWatcher>
#include <QMessageBox>

#include <iostream>

namespace ReconstructMeGUI {

  settings_dialog::settings_dialog(std::shared_ptr<reme_resource_manager> rm, QWidget *parent) :
    QDialog(parent),
    _rm(rm),
    _ui(new Ui::settings_dialog())
  {
    // setup this
    _ui->setupUi(this);

    _file_watcher = new QFileSystemWatcher(this);
    connect(_file_watcher, SIGNAL(fileChanged(const QString &)), SLOT(trigger_scanner_with_file(const QString &)));

    // create default settings at first program start or restore settings
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, profactor_tag, reme_tag);

    _device_id    = settings.value(opencl_device_tag, opencl_device_default_tag).toInt();
    _sens_path    = settings.value(sensor_path_tag, sensor_path_default_tag).toString();
    _cfg_path     = settings.value(config_path_tag, config_path_default_tag).toString();
    _license_file = settings.value(license_file_tag, license_file_default_tag).toString();
    
    if (QFile::exists(_sens_path))    _file_watcher->addPath(_sens_path);
    if (QFile::exists(_cfg_path))     _file_watcher->addPath(_cfg_path);
    if (QFile::exists(_license_file)) _file_watcher->addPath(_license_file);

    settings.setValue(sensor_path_tag, _sens_path);
    settings.setValue(opencl_device_tag, _device_id);
    settings.setValue(config_path_tag, _cfg_path);
    settings.setValue(license_file_tag, _license_file);
    settings.sync();

    // update textboxes
    _ui->config_path_tb->setText(_cfg_path);
    _ui->sensor_path_tb->setText(_sens_path);
    _ui->license_file_tb->setText(_license_file);

    // devices from ReconstructMe SDK
    connect(_rm.get(), SIGNAL(sdk_initialized(bool)), SLOT(init_opencl_device_widget()));

    connect(_ui->browse_button_config,  SIGNAL(clicked()), SLOT(browse_config_button_clicked()));
    connect(_ui->browse_button_sensor,  SIGNAL(clicked()), SLOT(browse_sensor_button_clicked()));
    connect(_ui->browse_button_license, SIGNAL(clicked()), SLOT(browse_license_file_clicked()));

    _rm->connect(this, SIGNAL(initialize()), SLOT(initialize()));

    QAbstractButton *restore_btn = _ui->buttonBox->button(QDialogButtonBox::RestoreDefaults);
    connect(restore_btn, SIGNAL(clicked()), SLOT(create_default_settings()));
  }


  void settings_dialog::init_opencl_device_widget() {
    _ui->device_list_widget->clear();

    opencl_info ocl_info;
    _rm->get_opencl_info(ocl_info);

    ::google::protobuf::RepeatedPtrField<opencl_info_device>::const_iterator it;
    for (it = ocl_info.devices().begin(); it < ocl_info.devices().end(); it++) {
      QString device (it->name().c_str());
      _ui->device_list_widget->addItem(new QListWidgetItem(device.trimmed()));
    }
    
    if (_device_id == opencl_device_default_tag)
      _ui->ocl_device_auto_cb->setChecked(true);
    else 
      _ui->device_list_widget->setCurrentRow(_device_id);
  }

  settings_dialog::~settings_dialog() {
    delete _ui;
  }

  void settings_dialog::set_settings_path(const QString &file_path, init_t type) {
    switch(type) {
      case LICENSE:
        _ui->license_file_tb->setText(file_path);
        accept();
        break;
      case OPENCL:
        _ui->config_path_tb->setText(file_path);
        accept();
        break;
      case SENSOR:
        _ui->sensor_path_tb->setText(file_path);
        accept();
        break;
    }
  }

  void settings_dialog::accept() {
    // remove old once from filesystemwatcher
    if (QFile::exists(_sens_path))    _file_watcher->removePath(_sens_path);
    if (QFile::exists(_cfg_path))     _file_watcher->removePath(_cfg_path);
    if (QFile::exists(_license_file)) _file_watcher->removePath(_license_file);

    _cfg_path     = _ui->config_path_tb->text();
    _sens_path    = _ui->sensor_path_tb->text();
    _license_file = _ui->license_file_tb->text();

    // add to filesystemwatcher
    if (QFile::exists(_sens_path))    _file_watcher->addPath(_sens_path);
    if (QFile::exists(_cfg_path))     _file_watcher->addPath(_cfg_path);
    if (QFile::exists(_license_file)) _file_watcher->addPath(_license_file);

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, profactor_tag, reme_tag);

    // get current device selection
    if (_ui->ocl_device_auto_cb->isChecked()) {
      _device_id = opencl_device_default_tag;
    }
    else {
      QListWidgetItem* item = _ui->device_list_widget->currentItem();
      if (!item) {
        // no selection, use preferred
        _device_id = opencl_device_default_tag;
        _ui->ocl_device_auto_cb->setChecked(true);
      }
      else {
        _device_id = _ui->device_list_widget->row(item);      
      }
    }
    
    // if an option changed
    if ((settings.value(config_path_tag, config_path_default_tag).toString()   != _cfg_path)     ||
        (settings.value(opencl_device_tag, opencl_device_default_tag).toInt()  != _device_id)    ||  
        (settings.value(license_file_tag, license_file_default_tag).toString() != _license_file) ||
        (settings.value(sensor_path_tag, sensor_path_default_tag).toString()   != _sens_path)      )
      emit initialize();

    // persist changes
    settings.setValue(config_path_tag, _cfg_path);
    settings.setValue(sensor_path_tag, _sens_path); // find preferred sensor
    settings.setValue(license_file_tag, _license_file);
    settings.setValue(opencl_device_tag, _device_id); 
    settings.sync();

    hide();
  }

  void settings_dialog::reject() {
    // restore values
    _ui->config_path_tb->setText(_cfg_path);
    _ui->sensor_path_tb->setText(_sens_path);
    _ui->license_file_tb->setText(_license_file);

    if (_device_id == opencl_device_default_tag)
      _ui->ocl_device_auto_cb->setChecked(true);
    else 
      _ui->device_list_widget->setCurrentRow(_device_id);

    // and hide
    hide();
  }

  void settings_dialog::create_default_settings() {
    // update ui
    _ui->config_path_tb->setText(config_path_default_tag);
    _ui->sensor_path_tb->setText(sensor_path_default_tag);
    _ui->license_file_tb->setText(license_file_default_tag);
    _ui->ocl_device_auto_cb->setChecked(true);
  }

  QString settings_dialog::get_file_from_dialog(QString &current_path, QString &filter) {

    QFileInfo fi(current_path);
    QString file_name = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                     fi.absolutePath(),
                                                     filter);

    return file_name;
  }

  void settings_dialog::browse_config_button_clicked() {
    QString selected_file = get_file_from_dialog(_cfg_path, QString("Configruation files (*.txt);; All files (*.*)"));
    if (selected_file == "") return;

    _ui->config_path_tb->setText(selected_file); 
  }

  void settings_dialog::browse_sensor_button_clicked() {
    QString selected_file = get_file_from_dialog(_sens_path, QString("Sensor files (*.txt);; All files (*.*)"));
    if (selected_file == "") return;

    _ui->sensor_path_tb->setText(selected_file);
  }

  void settings_dialog::browse_license_file_clicked() {
    QString selected_file = get_file_from_dialog(_license_file, QString("License files (*.txt.sgn);; All files (*.*)"));
    if (selected_file == "") return;

    _ui->license_file_tb->setText(selected_file);
  }

  void settings_dialog::trigger_scanner_with_file(const QString &file_path) {
    bool do_reload = QMessageBox::Yes == QMessageBox::information(this, file_changed_tag, apply_changes_tag + file_path + "?", QMessageBox::Yes, QMessageBox::No);

    if (do_reload && (
        (file_path == _cfg_path)     ||
        (file_path == _license_file) ||
        (file_path == _sens_path)   )  )
      emit initialize();
  }
}