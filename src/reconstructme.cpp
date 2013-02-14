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

#pragma once

#include "reconstructme.h"
#include "ui_reconstructmeqt.h"

#include "scan.h"
#include "reme_resource_manager.h"
#include "frame_grabber.h"

#include "settings.h"
#include "strings.h"
#include "defines.h"

#include "qglcanvas.h"
#include "logging_dialog.h"
#include "hardware_key_dialog.h"
#include "about_dialog.h"
#include "settings_dialog.h"
#include "status_dialog.h"

#include <stdlib.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QImage>
#include <QThread>
#include <QLabel>
#include <QFileDialog>
#include <QSettings>
#include <QApplication>
#include <QDesktopWidget>
#include <QKeySequence>
#include <QFont>
#include <QRegExp>
#include <QProgressBar>
#include <QProgressDialog>
#include <QMetaType>
#include <QPushButton>
#include <QSignalMapper>
#include <QWidget>
#include <QCloseEvent>
#include <QMovie>

#include <iostream>

#define STATUSBAR_TIME 1500

namespace ReconstructMeGUI {
  
  reconstructme::reconstructme(QWidget *parent) : 
    QMainWindow(parent),
    _ui(new Ui::reconstructmeqt),
    _rm(new reme_resource_manager())
  {
    // ui's setup
    _ui->setupUi(this);
    _ui->stackedWidget->setCurrentWidget(_ui->scanPage);

    // move to center
    QRect r = geometry();
    r.moveCenter(QApplication::desktop()->availableGeometry().center());
    setGeometry(r);
    
    // Create dialogs
    _dialog_log = new logging_dialog(_rm, this, Qt::Dialog);
    _dialog_settings = new settings_dialog(_rm, this);
    _dialog_license = new hardware_key_dialog(_rm, this);
    _dialog_about = new about_dialog(this);
    _dialog_state = new status_dialog(_rm, this);

    _dialog_license->connect(_ui->actionGenerate_hardware_key, SIGNAL(triggered()), SLOT(show()));
    _dialog_log->connect(_ui->actionLog, SIGNAL(triggered()), SLOT(show()));
    _dialog_about->connect(_ui->actionAbout, SIGNAL(triggered()), SLOT(show()));
    _dialog_settings->connect(_ui->actionSettings, SIGNAL(triggered()), SLOT(show()));

    // application icon
    QPixmap titleBarPix (":/images/icon.ico");
    QIcon titleBarIcon(titleBarPix);
    setWindowIcon(titleBarIcon);

    // Create 
    create_url_mappings();

    // Trigger concurrent initialization
    _fg = std::shared_ptr<frame_grabber>(new frame_grabber(_rm));
    _rm->set_frame_grabber(_fg);
    connect(_fg.get(), SIGNAL(frame(reme_sensor_image_t, const void*, int, int, int, int, int, int)), SLOT(show_frame(reme_sensor_image_t, const void*, int, int, int, int, int, int)));
    _rm->connect(this, SIGNAL(initialize()), SLOT(initialize()));
    _rm_thread = new QThread(this);
    _rm->moveToThread(_rm_thread);
    _fg->moveToThread(_rm_thread);
    _rm_thread->start();

    _fg->request(REME_IMAGE_AUX);
    _fg->request(REME_IMAGE_DEPTH);
    _fg->request(REME_IMAGE_VOLUME);

    _rm->connect(_ui->play_button, SIGNAL(clicked()), SLOT(start_scanning()));
    _rm->connect(_ui->reset_button, SIGNAL(clicked()), SLOT(reset_volume()));

    emit initialize();
  }

  void reconstructme::show_frame(reme_sensor_image_t type, const void* data, int length, int width, int height, int channels, int num_bytes_per_channel, int row_stride) {
    switch(type) {
    case REME_IMAGE_AUX:
      _ui->rgb_canvas->set_image_size(width, height);
      _ui->rgb_canvas->set_image_data(data, length);
      break;
    case REME_IMAGE_DEPTH:
      _ui->depth_canvas->set_image_size(width, height);
      _ui->depth_canvas->set_image_data(data, length);
      break;
    case REME_IMAGE_VOLUME:
      _ui->rec_canvas->set_image_size(width, height);
      _ui->rec_canvas->set_image_data(data, length);
      break;
    }
  }

  void reconstructme::create_url_mappings() {
    _url_mapper = new QSignalMapper(this);
    _url_mapper->setMapping(_ui->actionDevice, QString(url_device_matrix_tag));
    _url_mapper->setMapping(_ui->actionFAQ_2, QString(url_faq_tag));
    _url_mapper->setMapping(_ui->actionForum, QString(url_forum_tag));
    _url_mapper->setMapping(_ui->actionInstallation, QString(url_install_tag));
    _url_mapper->setMapping(_ui->actionProjectHome, QString(url_reconstructme_qt));
    _url_mapper->setMapping(_ui->actionSDKDocumentation, QString(url_sdk_doku_tag));
    _url_mapper->setMapping(_ui->actionUsage, QString(url_usage_tag));
    _url_mapper->setMapping(_dialog_state->onlineHelpBtn(), QString(url_faq_tag));
    
    _url_mapper->connect(_ui->actionDevice, SIGNAL(triggered()), SLOT(map()));
    _url_mapper->connect(_ui->actionFAQ_2, SIGNAL(triggered()), SLOT(map()));
    _url_mapper->connect(_ui->actionForum, SIGNAL(triggered()), SLOT(map()));
    _url_mapper->connect(_ui->actionInstallation, SIGNAL(triggered()), SLOT(map()));
    _url_mapper->connect(_ui->actionProjectHome, SIGNAL(triggered()), SLOT(map()));
    _url_mapper->connect(_ui->actionSDKDocumentation, SIGNAL(triggered()), SLOT(map()));
    _url_mapper->connect(_ui->actionUsage, SIGNAL(triggered()), SLOT(map()));
    _url_mapper->connect(_dialog_state->onlineHelpBtn(), SIGNAL(clicked()), SLOT(map()));

    connect(_url_mapper, SIGNAL(mapped(const QString&)), SLOT(open_url(const QString&)));
  }

  // ==================== closing ====================
  void reconstructme::closeEvent(QCloseEvent *ev) {
    if (_fg->is_grabbing()) {
      // 1. stop frame grabber before closing
      _fg->connect(this, SIGNAL(closing()), SLOT(stop()), Qt::QueuedConnection);
      this->connect(_fg.get(), SIGNAL(stopped_grabbing()), SLOT(really_close()), Qt::QueuedConnection);
      emit closing();
      ev->ignore();
    } else {
      // 3. close QMainWindow
      ev->accept();
      QMainWindow::closeEvent(ev);
    }
  }

  void reconstructme::really_close() {
    // 2. frame grabber stopped, so call close event again
    this->close();
  }

  reconstructme::~reconstructme() {
    // 4. Since the close event is accepted, destruct this
    _rm_thread->quit();
    _rm_thread->wait();

    delete _ui;
  }

  // ==================== closing ====================

  void reconstructme::show_message_box(
      int icon, 
      QString message, 
      int btn_1,
      int btn_2) {
    
    switch (icon) {
      case QMessageBox::Warning:
        QMessageBox::warning(this, warning_tag, message, btn_1, btn_2);
        break;
      case QMessageBox::Critical:
        QMessageBox::critical(this, critical_tag, message, btn_1, btn_2);
        break;
      case QMessageBox::Information:
        QMessageBox::information(this, information_tag, message, btn_1, btn_2);
        break;
      case QMessageBox::Question:
        QMessageBox::question(this, question_tag, message, btn_1, btn_2);
        break;
    }
  }

  void reconstructme::action_settings_clicked() {
    _dialog_settings->show();
    _dialog_settings->raise();
    _dialog_settings->activateWindow();
  }

  void reconstructme::open_url(const QString &url_string) {
    QUrl url (url_string);
    QDesktopServices::openUrl(url);
    QString msg = open_url_tag + url_string;
    _ui->reconstruct_satus_bar->showMessage(msg, STATUSBAR_TIME);
  }

  void reconstructme::status_bar_msg(const QString &msg, const int msecs) {
    statusBar()->showMessage(msg, msecs);
  }

  void reconstructme::show_fps(const float fps) {
    //if (fps > 20) 
    //  _label_fps_color->setStyleSheet("background-color: green;");
    //else if (fps > 10)
    //  _label_fps_color->setStyleSheet("background-color: orange;");
    //else
    //  _label_fps_color->setStyleSheet("background-color: #FF4848;");
    //
    //_label_fps->setText(QString().sprintf("%.2f fps", fps));
  }
}
