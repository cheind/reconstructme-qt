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
  * @authors christoph.heindl@profactor.at
  */
  
#include "log_table_model.h"

namespace ReconstructMeGUI {

  LogTableModel::LogTableModel(QObject *parent)
    :QAbstractTableModel(parent)
  {}

  int LogTableModel::rowCount(const QModelIndex &parent) const
  {
    Q_UNUSED(parent);
    return _messages.size();
  }

  int LogTableModel::columnCount(const QModelIndex &parent) const
  {
    Q_UNUSED(parent);
    return 3;
  }

  QVariant LogTableModel::data(const QModelIndex &index, int role) const
  {
    if (!index.isValid())
      return QVariant();

    if (index.row() >= _messages.size() || index.row() < 0)
      return QVariant();

    const LogMessage &m = _messages.at(index.row());

    if (role == Qt::DisplayRole) {

      if (index.column() == 0) {
        return m.timestamp;
      } else if (index.column() == 1) {
        // Severity
        switch (m.severity) {
          case REME_LOG_SEVERITY_INFO:
            return tr("Info");
        
          case REME_LOG_SEVERITY_WARNING:
            return tr("Warning");

          case REME_LOG_SEVERITY_ERROR:
            return tr("Error");
        }

      } else if (index.column() == 2) {
        // Message itself
        return m.message;
      } 
    }


    return QVariant();
  }

  QVariant LogTableModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
      return QVariant();

    switch (section) {
      case 0:
        return tr("Timestamp");

      case 1:
        return tr("Severity");

      case 2:
        return tr("Message");

      default:
        return QVariant();
    }
  }

  void LogTableModel::prepend(reme_log_severity_t sev, const QString &msg)
  {
   
    beginInsertRows(QModelIndex(), 0, 1);

     LogMessage m;
     m.timestamp = QDateTime::currentDateTime();
     m.severity = sev;
     m.message = msg;
     _messages.insert(0, m);

     endInsertRows();
  }

}




