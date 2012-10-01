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

#include "logging_dialog.h"
#include "ui_logging_dialog.h"

#include <iostream>
#include <QDateTime>

namespace ReconstructMeGUI {
  logging_dialog::logging_dialog(QWidget *parent, Qt::WindowFlags f) : 
    QDialog(parent, f),  
    ui(new Ui::logging_widget)
  {
    ui->setupUi(this);
    setModal(false);
  }

  logging_dialog::~logging_dialog() 
  {
    delete ui;
  }

  void logging_dialog::append_log_message(const QString &log) {
    if (log == "") return;

    QString new_log = 
      "=== " + QDateTime::currentDateTime().toString() + " ===\n" + 
      log;

    ui->log_te->append(new_log);
  }

  void logging_dialog::closeEvent (QCloseEvent *e) {
    emit close_clicked();
  }

  void logging_dialog::align_to_parent() {
    QRect pos = parentWidget()->geometry();
    pos.setY(pos.y() + pos.height() + 37); // 37 is the height of the titlebar on Windows platform :-)
    setGeometry(pos);
  }
}