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
#include <QComboBox>

#include <iostream>

namespace ReconstructMeGUI {

  settings_dialog::settings_dialog(std::shared_ptr<reme_resource_manager> rm, QWidget *parent) :
    QDialog(parent),
    _rm(rm),
    _ui(new Ui::settings_dialog())
  {
    // setup this
    _ui->setupUi(this);

    _fw = new QFileSystemWatcher(this);
    connect(_fw, SIGNAL(fileChanged(const QString &)), SLOT(apply_changed_file(const QString &)));

    refresh_entries();
  }

  settings_dialog::~settings_dialog() 
  {
    delete _ui;
  }

  void settings_dialog::apply_changed_file(const QString &file) 
  {
    if (QMessageBox::Yes == QMessageBox::information(this, "File Content Changed", "File " + file + " changed. Apply Changes?", QMessageBox::Yes, QMessageBox::No))
      QMetaObject::invokeMethod(_rm.get(), "initialize", Qt::QueuedConnection);
  }

  void settings_dialog::refresh_entries() 
  {
    QSettings s(QSettings::IniFormat, QSettings::UserScope, profactor_tag, reme_tag);

    QString config = s.value(config_path_tag, config_path_default_tag).toString();
    QString sensor = s.value(sensor_path_tag, sensor_path_default_tag).toString();
    int device = s.value(devcice_id_tag, -1).toInt();

    QDir dir = QDir::current();
    dir.setFilter(QDir::Files);
    dir.setSorting(QDir::Name);

    dir.cd("cfg");
    QStringList config_list = dir.entryList();
    QComboBox &lw_config = *_ui->lw_config;
    lw_config.clear();
    lw_config.addItem("AUTO: Default Configuration", config_path_default_tag);
    lw_config.setCurrentIndex(0);
    std::for_each(config_list.begin(), config_list.end(), [&lw_config, &dir, &config](QString &c) {
      lw_config.addItem(QFileInfo(c).baseName(), dir.absoluteFilePath(c));
      if (QString::compare(QFileInfo(config).fileName(), QFileInfo(c).fileName()) == 0)
        lw_config.setCurrentIndex(lw_config.count()-1);
    });

    dir.cd("sensor");
    QStringList sensor_list = dir.entryList();
    QComboBox &lw_sensor = *_ui->lw_sensor;
    lw_sensor.clear();
    lw_sensor.addItem("AUTO: Let ReconstructMe Chosse A Sensor", sensor_path_default_tag);
    lw_sensor.setCurrentIndex(0);
    std::for_each(sensor_list.begin(), sensor_list.end(), [&lw_sensor, &dir, &sensor](QString &s) {
      lw_sensor.addItem(QFileInfo(s).baseName(), dir.absoluteFilePath(s));
      if (QString::compare(QFileInfo(sensor).fileName(), QFileInfo(s).fileName()) == 0)
        lw_sensor.setCurrentIndex(lw_sensor.count()-1);
    });

    opencl_info ocl;
    _rm->get_opencl_info(ocl);
    QComboBox &lw_device = *_ui->lw_device;
    lw_device.clear();
    lw_device.addItem("AUTO: Autoselect Best Device", -1);
    lw_device.setCurrentIndex(0);
    int cnt = 0;
    std::for_each(ocl.devices().begin(), ocl.devices().end(), [&lw_device, &device, &cnt](const opencl_info_device &dev) {
      lw_device.addItem(dev.name().c_str(), cnt);
      if (device == cnt) 
        lw_device.setCurrentIndex(lw_device.count()-1);
      cnt++;
    });
  }

  void settings_dialog::accept() 
  { 
    _fw->removePaths(_fw->files()); // Remove watched paths

    QDir dir = QDir::current();
    dir.cd("cfg");
    QString config = _ui->lw_config->itemData(_ui->lw_config->currentIndex()).value<QString>();
    QString config_path;
    if (QFileInfo(dir.absoluteFilePath(config)).exists() && QFileInfo(dir.absoluteFilePath(config)).isFile()) {
      _fw->addPath(config);
      config_path = dir.absoluteFilePath(config);
    }
    else {
      config_path = config_path_default_tag;
    }
    
    dir.cd("sensor");
    QString sensor = _ui->lw_sensor->itemData(_ui->lw_sensor->currentIndex()).value<QString>();
    QString sensor_path;
    if (QFileInfo(dir.absoluteFilePath(sensor)).exists() && QFileInfo(dir.absoluteFilePath(sensor)).isFile()) {
      _fw->addPath(sensor);
      sensor_path = dir.absoluteFilePath(sensor);
    }
    else {
      sensor_path = sensor_path_default_tag;
    }
    
    int device = _ui->lw_device->itemData(_ui->lw_device->currentIndex()).value<int>();
    
    QSettings s(QSettings::IniFormat, QSettings::UserScope, profactor_tag, reme_tag);
    s.setValue(config_path_tag, config_path);
    s.setValue(sensor_path_tag, sensor_path);
    s.setValue(devcice_id_tag, device);
    s.sync();

    QMetaObject::invokeMethod(_rm.get(), "initialize", Qt::QueuedConnection);
    hide();
  }

  void settings_dialog::reject() 
  {
    hide();
  }

}