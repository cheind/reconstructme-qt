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

#include "log.pb.h"

#include <QDateTime>
#include <QFile>
#include <QFileDialog>

#include <fstream>

namespace ReconstructMeGUI {

  logging_dialog::logging_dialog(QWidget *parent, Qt::WindowFlags f) : 
    window_dialog(parent, f),  
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

    connect(ui->btnClear, SIGNAL(clicked()), SLOT(clear_log()));
    connect(ui->btnSave, SIGNAL(clicked()), SLOT(save_log()));
  }

  logging_dialog::~logging_dialog() 
  {
    delete ui;
  }

  void logging_dialog::add_log_message(reme_log_severity_t sev, const QString &log) {
     
    QStyle *style = this->style();

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

    QList< QStandardItem *> items;
    
    QStandardItem *sev_item = new QStandardItem(sev_icon, sev_str);
    QStandardItem *date_item = new QStandardItem(QDateTime::currentDateTime().toString());
    QStandardItem *message_item = new QStandardItem(log);

    sev_item->setEditable(false);
    date_item->setEditable(false);
    message_item->setEditable(false);

    items.push_back(sev_item);
    items.push_back(date_item);
    items.push_back(message_item);

    _log_model->insertRow(0, items);
  }

  void logging_dialog::clear_log() {
    _log_model->removeRows(0, _log_model->rowCount());
  }
  
  void logging_dialog::save_log() {
    QString file_name = QFileDialog::getSaveFileName(this, tr("Save log"),
                                                 QDir::currentPath(),
                                                 tr("Text File (*.log)"),
                                                 0);
    if (file_name.isEmpty())
      return;

    logging_info log;
    logging_info_log_entry *log_entry;
    
    for(int row = 0; row < _log_model->rowCount(); row++) {
      log_entry = log.add_logs();

      logging_info::severity sev;

      QString txt = _log_model->item(row, 0)->text();
      if (txt == "Info")
        sev = logging_info::INFO;
      else if (txt == "Warning")
        sev = logging_info::WARNING;
      else 
        sev = logging_info::ERROR;

      log_entry->set_sev(sev);
      log_entry->set_date(_log_model->item(row, 1)->text().toStdString());
      log_entry->set_log(_log_model->item(row, 2)->text().toStdString());
    }

    std::ofstream ost;
    ost.open(file_name.toStdString());
    ost << log.SerializePartialAsString();
    ost.close();
  }
}