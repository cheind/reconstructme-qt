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

#ifndef HARDWARE_KEY_DIALOG_H
#define HARDWARE_KEY_DIALOG_H

#include "types.h"
#include "reme_resource_manager.h"

#include <QDialog>

// Forward declarations
namespace Ui {
  class hardware_key_dialog;
}

namespace ReconstructMeGUI {

  /** This dialog provides anonymized information about hardware keys */
  class hardware_key_dialog : public QDialog
  {
    Q_OBJECT;

  public:
    hardware_key_dialog(std::shared_ptr<reme_resource_manager> rm, QWidget *parent = 0);
    ~hardware_key_dialog();

  private slots:
    /** Saves the keys to a text file */
    void save_keys();
    /** Copies the keys to the clipboard */
    void copy_keys_to_clipboard();

    void set_hashes();

  private:
    Ui::hardware_key_dialog *_ui;
    std::shared_ptr<reme_resource_manager> _rm;
  };

} 

#endif // HARDWARE_KEY_DIALOG_H