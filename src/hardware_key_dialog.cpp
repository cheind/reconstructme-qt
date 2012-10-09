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

#include "hardware_key_dialog.h"
#include "ui_hardware_key_dialog.h"

#include "hardware.pb.h"

#include <QFileDialog>
#include <QMessageBox>

#include <fstream>
#include <sstream>

namespace ReconstructMeGUI {
  hardware_key_dialog::hardware_key_dialog(reme_context_t ctx, QWidget *parent) : 
    QDialog(parent),  
    ui(new Ui::hardware_key_dialog)
  {
    ui->setupUi(this);
    setModal(true);

    const void *bytes;
    int length;

    reme_options_t o;
    reme_options_create(ctx, &o);

    reme_license_t l;
    reme_license_create(ctx, &l);
    reme_license_bind_hardware_hashes(ctx, l, o);

    reme_options_get_bytes(ctx, o, &bytes, &length); 

    hardware hardware;
    hardware.ParseFromArray(bytes, length);
    
    std::stringstream ss;
    ::google::protobuf::RepeatedPtrField< ::std::string>::const_iterator it;

    for (it = hardware.hashes().begin(); it < hardware.hashes().end(); it++) 
      ss << *it << "\n";
    
    ui->hw_key_te->append(QString(ss.str().c_str()));


    // connections
    connect(ui->cp_clipboard_btn, SIGNAL(clicked()), SLOT(copy_keys_to_clipboard()));
    connect(ui->save_btn, SIGNAL(clicked()), SLOT(save_keys()));
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
    myfile << ui->hw_key_te->toPlainText().toStdString();
    myfile.close();

    QMessageBox::information(this, "Save", "Saved hardware keys to " + file_name, QMessageBox::Ok);
  }

  void hardware_key_dialog::copy_keys_to_clipboard() {
    ui->hw_key_te->selectAll();
    ui->hw_key_te->copy();

    QMessageBox::information(this, "Copy", "Copied hardware keys to clipboard", QMessageBox::Ok);
  }

  hardware_key_dialog::~hardware_key_dialog() 
  {
    delete ui;
  }
}