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

#include "hardware_key_dialog.h"
#include "ui_hardware_key_dialog.h"

#include "hardware.pb.h"

#include "settings.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QDir>

#include <fstream>
#include <sstream>
#include <iostream>

namespace ReconstructMeGUI {
  hardware_key_dialog::hardware_key_dialog(std::shared_ptr<reme_resource_manager> rm, QWidget *parent) : 
    QDialog(parent),
    _rm(rm),
    _ui(new Ui::hardware_key_dialog)
  {
    _ui->setupUi(this);
    setModal(true);
    
    QSettings s(QSettings::IniFormat, QSettings::UserScope, profactor_tag, reme_tag);
    QString license = s.value(license_file_tag, license_file_default_tag).value<QString>();
    _ui->le_license->setText(license);

    // connections
    connect(_ui->cp_clipboard_btn, SIGNAL(clicked()), SLOT(copy_keys_to_clipboard()));
    connect(_ui->save_btn, SIGNAL(clicked()), SLOT(save_keys()));
    connect(rm.get(), SIGNAL(sdk_initialized(bool)), SLOT(set_hashes()));
    connect(_ui->pb_apply, SIGNAL(clicked()), SLOT(apply_license()));
    connect(_ui->pb_browse, SIGNAL(clicked()), SLOT(browse_license()));
  }

  hardware_key_dialog::~hardware_key_dialog() 
  {
    delete _ui;
  }

  void hardware_key_dialog::browse_license() 
  {
    QSettings s(QSettings::IniFormat, QSettings::UserScope, profactor_tag, reme_tag);
    QString license = s.value(license_file_tag, license_file_default_tag).value<QString>();
    
    QString fd_path = QDir::currentPath();
    if (QString::compare(license, license_file_default_tag) != 0)
      if (QFileInfo(license).exists())
        fd_path = license;
      
    QString license_path = QFileDialog::getOpenFileName(
      this, "Open License File", fd_path, "License File (*.sgn)");

    if (license_path.isEmpty())
      return;

    _ui->le_license->setText(license_path);
  }

  void hardware_key_dialog::apply_license() 
  {
    QString license_path = _ui->le_license->text();
    
    QSettings s(QSettings::IniFormat, QSettings::UserScope, profactor_tag, reme_tag);
    s.setValue(license_file_tag, license_path);

    QMetaObject::invokeMethod(_rm.get(), "initialize", Qt::QueuedConnection);
    hide();
  }

  void hardware_key_dialog::set_hashes()
  {
    hardware hardware;
    _rm->get_hardware_hashes(hardware);

    std::stringstream ss;
    ::google::protobuf::RepeatedPtrField< ::std::string>::const_iterator it;

    _ui->hw_key_te->clear();
    for (it = hardware.hashes().begin(); it < hardware.hashes().end(); it++) 
      ss << "hashes: \"" << *it << "\"\n";
    
    _ui->hw_key_te->append(QString(ss.str().c_str()));
  }
 

  void hardware_key_dialog::save_keys() {
    QString file_name = QFileDialog::getSaveFileName(this, tr("Save hardware keys"),
                                                 QDir::currentPath(),
                                                 tr("Text File (*.txt)"),
                                                 0);
    if (file_name.isEmpty())
      return;

    std::ofstream myfile;
    myfile.open(file_name.toStdString());
    myfile << _ui->hw_key_te->toPlainText().toStdString();
    myfile.close();
  }

  void hardware_key_dialog::copy_keys_to_clipboard() 
  {
    _ui->hw_key_te->selectAll();
    _ui->hw_key_te->copy();

    QMessageBox::information(this, "Copy", "Copied hardware keys to clipboard", QMessageBox::Ok);
  }
}