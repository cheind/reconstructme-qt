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

#ifndef STATUS_DIALOG_H
#define STATUS_DIALOG_H

#include "types.h"

#include <QDialog>

#include <reconstructmesdk/types.h>

// Forward declarations
class QStandardItemModel;
class QStandardItem;
class QPushButton;
namespace Ui {
  class status_dialog;
}

namespace ReconstructMeGUI {

  /** This is dialog provides status information of the scanner*/
  class status_dialog : public QDialog
  {
    Q_OBJECT;

  public:
    status_dialog(QWidget *parent = 0, Qt::WindowFlags f = Qt::FramelessWindowHint);
    ~status_dialog();
    
    QPushButton *closeBtn();
    QPushButton *logBtn();

  public slots:
    void reset();
    void initializing(init_t what);
    void initialized(init_t what, bool success);

  protected:
    /** Since there is no signal emitted for a close event, this method is overwritten */

  private:
    void create_content();

    Ui::status_dialog *ui;
    QStandardItemModel *_status_model;

    QStandardItem *_lic_obj_item;
    QStandardItem *_lic_status_item;
    QStandardItem *_lic_message_item;
                   
    QStandardItem *_sen_obj_item;
    QStandardItem *_sen_status_item;
    QStandardItem *_sen_message_item;
                   
    QStandardItem *_dev_obj_item;
    QStandardItem *_dev_status_item;
    QStandardItem *_dev_message_item;
  };
} 

#endif // STATUS_DIALOG_H