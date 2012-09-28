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
  
#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#pragma once

#include <QDialog>

#include <reconstructmesdk/reme.h>

// Forward Declaration
class QFileDialog;
class QFileSystemWatcher;
namespace Ui {
  class settings_dialog;
}

namespace ReconstructMeGUI {

  class settings_dialog : public QDialog
  {
    Q_OBJECT
    
  public:
    settings_dialog(reme_context_t ctx, QWidget *parent = 0);
    ~settings_dialog();

  public slots:
    virtual void accept();
    virtual void reject();

  signals:
    void sensor_changed();
    void opencl_settings_changed();
    
  private slots:
    void browse_config_button_clicked();
    void browse_sensor_button_clicked();
    void browse_license_file_clicked();
    void create_default_settings();
    void init_opencl_device_widget();
    void trigger_scanner_with_file(const QString &file_path);
  
  private:
    QString get_file_from_dialog(QString &current_path);

    // Members
    Ui::settings_dialog *ui;
    
    reme_context_t c;

    // Paths
    QString cfg_path;
    QString sens_path;
    QString license_file;
    // Paths utils
    QFileDialog* file_dialog;
    QFileSystemWatcher *file_watcher;

    // Selected device
    int device_id;
  };

}

#endif // SETTINGS_DIALOG_H
