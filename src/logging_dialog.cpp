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

#include <QDateTime>

namespace ReconstructMeGUI {

  logging_dialog::logging_dialog(QWidget *parent, Qt::WindowFlags f) : 
    QDialog(parent, f),  
    ui(new Ui::logging_widget)
  {
    ui->setupUi(this);

    _log_model = new QStandardItemModel(0, 3, parent);
    _log_model->setHeaderData(0, Qt::Horizontal, tr("Severity"));
    _log_model->setHeaderData(1, Qt::Horizontal, tr("Timestamp"));
    _log_model->setHeaderData(2, Qt::Horizontal, tr("Message"));

    _proxy_model = new QSortFilterProxyModel(parent);
    _proxy_model->setSourceModel(_log_model);
    _proxy_model->setDynamicSortFilter(true);

    ui->logtableview->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->logtableview->horizontalHeader()->setStretchLastSection(true);
    ui->logtableview->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);    
   
    ui->logtableview->setModel(_proxy_model);
    ui->logtableview->horizontalHeader()->setSortIndicator(1, Qt::AscendingOrder);

    setModal(false);
  }

  logging_dialog::~logging_dialog() 
  {
    delete ui;
  }

  void logging_dialog::add_log_message(reme_log_severity_t sev, const QString &log) {
     
    QStyle *style = this->style();

    _log_model->insertRow(0);

    QString sev_str;
    QIcon sev_icon;


    switch (sev) {
      case REME_LOG_SEVERITY_INFO:
        sev_str = tr("Info");
        sev_icon = style->standardIcon(QStyle::SP_MessageBoxInformation);
        break;

      case REME_LOG_SEVERITY_WARNING:
        sev_str = tr("Warning");
        sev_icon = style->standardIcon(QStyle::SP_MessageBoxWarning);
        break;

      case REME_LOG_SEVERITY_ERROR:
        sev_str = tr("Error");
        sev_icon = style->standardIcon(QStyle::SP_MessageBoxCritical);
        break;
     }
     
     _log_model->setData(_log_model->index(0, 0), sev_str, Qt::DisplayRole);
     _log_model->setData(_log_model->index(0, 0), sev_icon, Qt::DecorationRole);
     _log_model->setData(_log_model->index(0, 1), QDateTime::currentDateTime(), Qt::DisplayRole);
     _log_model->setData(_log_model->index(0, 2), log, Qt::DisplayRole);
  }

  void logging_dialog::closeEvent (QCloseEvent *e) {
    emit close_clicked();
  }
}