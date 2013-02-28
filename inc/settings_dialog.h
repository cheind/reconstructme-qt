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
  *     * Neither the name of Profactor GmbH nor the names of its
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

#include "types.h"
#include "reme_resource_manager.h"

#include <QDialog>


// Forward Declaration
class QFileDialog;
class QFileSystemWatcher;
namespace Ui {
  class settings_dialog;
}

namespace ReconstructMeGUI {
  /** This dialog manages the settings of reconstructme 
   *
   *  \note The settings are application wide available via QSettings. 
   */
  class settings_dialog : public QDialog
  {
    Q_OBJECT
    
  public:
    settings_dialog(std::shared_ptr<reme_resource_manager> rm, QWidget *parent = 0);
    ~settings_dialog();

  public slots:
    /** Syncronize current settings */
    virtual void accept();
    /** Discard changes */
    virtual void reject();

  private slots:
    void refresh_entries();

    void save_settings();

    void apply_changed_file(const QString &);

  private:
    Ui::settings_dialog *_ui;
    std::shared_ptr<reme_resource_manager> _rm;

    QFileSystemWatcher *_fw;
  };

}

#endif // SETTINGS_DIALOG_H
