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

#include "about_dialog.h"
#include "ui_about_dialog.h"

#include "defines.h"

#include <QPixmap>

#include <sstream>

namespace ReconstructMeGUI {
  about_dialog::about_dialog(reme_context_t c, QWidget *parent, Qt::WindowFlags f) : 
    QDialog(parent, f),  
    ui(new Ui::about_dialog)
  {
    ui->setupUi(this);

    std::stringstream ss_gui_v;
    ss_gui_v << RECONSTRUCTMEQT_VERSION_MAJOR << ".";
    ss_gui_v << RECONSTRUCTMEQT_VERSION_MINOR << ".";
    ss_gui_v << RECONSTRUCTMEQT_VERSION_BUILD;

    std::stringstream ss_sdk_v;
    ss_sdk_v << REME_VERSION_MAJOR << ".";
    ss_sdk_v << REME_VERSION_MINOR << ".";
    ss_sdk_v << REME_VERSION_BUILD << "-";
    ss_sdk_v << REME_VERSION_REVISION;

    int length;               
    const char* runtime_sdk_version;
    reme_context_get_version(c, &runtime_sdk_version, &length);

    std::stringstream ss_version;
    ss_version << "Version ReconstructMeQT: " << ss_gui_v.str() << "\n";
    ss_version << "Build Version ReconstructMeSDK: " << ss_sdk_v.str() << "\n";
    ss_version << "Runtime Version ReconstructMeSDK: " << ss_sdk_v.str();
    
    ui->version_label->setText(QString::fromStdString(ss_version.str()));
    
    std::stringstream ss_authors;
    ss_authors << "Christoph Heindl";
    ss_authors << "\nChristoph Kopf";
    ss_authors << "\nFlorian Eckerstorfer";

    ui->authors_label->setText(QString::fromStdString(ss_authors.str()));

    ui->logo_label->setPixmap(QPixmap(":/images/reme_typo.png"));
    ui->logo_label->setOpenExternalLinks(true);
  }

  about_dialog::~about_dialog()
  {
    delete ui;
  }
}