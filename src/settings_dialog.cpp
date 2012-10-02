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

  settings_dialog::settings_dialog(reme_context_t ctx, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::settings_dialog())
  {
    // setup this
    ui->setupUi(this);
    c = ctx;

    file_watcher = new QFileSystemWatcher(this);
    connect(file_watcher, SIGNAL(fileChanged(const QString &)), SLOT(trigger_scanner_with_file(const QString &)));

    // create default settings at first program start or restore settings
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, profactor_tag, reme_tag);

    device_id    = settings.value(opencl_device_tag, opencl_device_default_tag).toInt();
    sens_path    = settings.value(sensor_path_tag, sensor_path_default_tag).toString();
    cfg_path     = settings.value(config_path_tag, config_path_default_tag).toString();
    license_file = settings.value(license_file_tag, license_file_default_tag).toString();
    
    file_watcher->addPath(sens_path);
    file_watcher->addPath(cfg_path);
    file_watcher->addPath(license_file);

    settings.setValue(sensor_path_tag, sens_path);
    settings.setValue(opencl_device_tag, device_id);
    settings.setValue(config_path_tag, cfg_path);
    settings.setValue(license_file_tag, license_file);
    settings.sync();

    // update textboxes
    ui->config_path_tb->setText(cfg_path);
    ui->sensor_path_tb->setText(sens_path);
    ui->license_file_tb->setText(license_file);

    // devices from ReconstructMe SDK
    init_opencl_device_widget();

    connect(ui->browse_button_config,  SIGNAL(clicked()), SLOT(browse_config_button_clicked()));
    connect(ui->browse_button_sensor,  SIGNAL(clicked()), SLOT(browse_sensor_button_clicked()));
    connect(ui->browse_button_license, SIGNAL(clicked()), SLOT(browse_license_file_clicked()));

    QAbstractButton *restore = ui->buttonBox->button(QDialogButtonBox::RestoreDefaults);
    connect(restore, SIGNAL(clicked()), SLOT(create_default_settings()));
  }

  void settings_dialog::init_opencl_device_widget() {
    ui->device_list_widget->clear();

    const void *bytes;
    int length;

    reme_options_t o;
    reme_options_create(c, &o);

    reme_context_bind_opencl_info(c, o);

    reme_options_get_bytes(c, o, &bytes, &length); 

    opencl_info ocl_info;
    ocl_info.ParseFromArray(bytes, length);

    google::protobuf::RepeatedPtrField<opencl_info_device>::const_iterator it;
    for (it = ocl_info.devices().begin(); it < ocl_info.devices().end(); it++) {
      QString device (it->name().c_str());
      ui->device_list_widget->addItem(new QListWidgetItem(device.trimmed()));
    }
    
    if (device_id == opencl_device_default_tag)
      ui->ocl_device_auto_cb->setChecked(true);
    else 
      ui->device_list_widget->setCurrentRow(device_id);
  }

  settings_dialog::~settings_dialog() {
    delete ui;
  }

  void settings_dialog::accept() {
    // remove from filesystemwatcher
    file_watcher->removePath(sens_path);
    file_watcher->removePath(cfg_path);
    file_watcher->removePath(license_file);

    cfg_path     = ui->config_path_tb->text();
    sens_path    = ui->sensor_path_tb->text();
    license_file = ui->license_file_tb->text();

    // add to filesystemwatcher
    file_watcher->addPath(sens_path);
    file_watcher->addPath(cfg_path);
    file_watcher->addPath(license_file);

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, profactor_tag, reme_tag);

    // get current device selection
    if (ui->ocl_device_auto_cb->isChecked()) {
      device_id = opencl_device_default_tag;
    }
    else {
      QListWidgetItem* item = ui->device_list_widget->currentItem();
      if (!item) {
        // no selection, use preferred
        device_id = opencl_device_default_tag;
        ui->ocl_device_auto_cb->setChecked(true);
      }
      else {
        device_id = ui->device_list_widget->row(item);      
      }
    }
    
    // if an option changed
    if ((settings.value(config_path_tag, config_path_default_tag).toString()   != cfg_path)     ||
        (settings.value(license_file_tag, license_file_default_tag).toString() != license_file) ||
        (settings.value(opencl_device_tag, opencl_device_default_tag).toInt()  != device_id)      )
      emit opencl_settings_changed();
    
    if (settings.value(sensor_path_tag, sensor_path_default_tag).toString()   != sens_path)
      emit sensor_changed();

    // persist changes
    settings.setValue(config_path_tag, cfg_path);
    settings.setValue(sensor_path_tag, sens_path); // find preferred sensor
    settings.setValue(license_file_tag, license_file);
    settings.setValue(opencl_device_tag, device_id); 
    settings.sync();

    hide();
  }

  void settings_dialog::reject() {
    // restore values
    ui->config_path_tb->setText(cfg_path);
    ui->sensor_path_tb->setText(sens_path);
    ui->license_file_tb->setText(license_file);

    if (device_id == opencl_device_default_tag)
      ui->ocl_device_auto_cb->setChecked(true);
    else 
      ui->device_list_widget->setCurrentRow(device_id);

    // and hide
    hide();
  }

  void settings_dialog::create_default_settings() {
    // update settings
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, profactor_tag, reme_tag);
    settings.setValue(sensor_path_tag, sensor_path_default_tag);
    settings.setValue(opencl_device_tag, opencl_device_default_tag);
    settings.setValue(config_path_tag, config_path_default_tag);
    settings.setValue(license_file_tag, license_file_default_tag);
    settings.sync();

    // update ui
    ui->config_path_tb->setText(config_path_default_tag);
    ui->sensor_path_tb->setText(sensor_path_default_tag);
    ui->license_file_tb->setText(license_file_default_tag);
    ui->ocl_device_auto_cb->setChecked(true);
  }

  QString settings_dialog::get_file_from_dialog(QString &current_path) {

    QFileInfo fi(current_path);
    QString file_name = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                     fi.absolutePath(),
                                                     tr("Text files (*.txt);; All files (*.*)"));

    return file_name;
  }

  void settings_dialog::browse_config_button_clicked() {
    QString selected_file = get_file_from_dialog(cfg_path);
    if (selected_file == "") return;

    ui->config_path_tb->setText(selected_file); 
  }

  void settings_dialog::browse_sensor_button_clicked() {
    QString selected_file = get_file_from_dialog(sens_path);
    if (selected_file == "") return;

    ui->sensor_path_tb->setText(selected_file);
  }

  void settings_dialog::browse_license_file_clicked() {
    QString selected_file = get_file_from_dialog(sens_path);
    if (selected_file == "") return;

    ui->license_file_tb->setText(selected_file);
  }

  void settings_dialog::trigger_scanner_with_file(const QString &file_path) {
    if (QMessageBox::No == QMessageBox::information(this, file_changed_tag, apply_changes_tag + file_path + "?", QMessageBox::Yes, QMessageBox::No)) return;

    if ((file_path == cfg_path) || (file_path == license_file))
      emit opencl_settings_changed();
    else if (file_path == sens_path)
      emit sensor_changed();
  }
}




