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

#include "scan_widget.h"
#include "calibration_widget.h"

#include "scan.h"
#include "reme_sdk_initializer.h"
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

#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QSplashScreen>
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

#include <iostream>

#define STATUSBAR_TIME 1500
#define _splash_MSG_ALIGNMENT Qt::AlignBottom | Qt::AlignLeft

namespace ReconstructMeGUI {
  
  reconstructme::reconstructme(QWidget *parent) : 
    QMainWindow(parent),
    _ui(new Ui::reconstructmeqt),
    _initializer(new reme_sdk_initializer())
  {
    // _splashscreen
    QPixmap splashPix(":/images/_splash_screen.png");
    _splash = new QSplashScreen(this, splashPix);
    _splash->setAutoFillBackground(false);
    _splash->showMessage(welcome_tag, _splash_MSG_ALIGNMENT);
    _splash->show();

    _frame_grabber = new frame_grabber(_initializer);
    _frame_grabber_thread = new QThread(this);
    _frame_grabber->moveToThread(_frame_grabber_thread);
    _frame_grabber_thread->start();

    // ui's setup
    _ui->setupUi(this);
    _scan_ui = new scan_widget(_initializer, this);
    _calibration_ui = new calibration_widget(_initializer, this);

    _ui->scan_page = _scan_ui;
    _ui->calibration_page = _calibration_ui;
    _ui->stackedWidget->insertWidget(0, _ui->scan_page);
    _ui->stackedWidget->insertWidget(1, _ui->calibration_page);
    _ui->stackedWidget->setCurrentWidget(_ui->scan_page);

    // Status bar
    _fps_label = new QLabel();
    _fps_label->setStyleSheet("qproperty-alignment: AlignRight; margin-right: 0px; padding-right: 0px;");
    _fps_label->setMaximumWidth(100);
    _fps_label->setToolTip(tool_tip_fps_label_tag);
    
    _fps_color_label = new QLabel();
    _fps_color_label->setMinimumWidth(10);
    _fps_color_label->setMaximumWidth(10);
    _fps_color_label->setStyleSheet("margin-left: 0px; padding-left: 0px;");
    _fps_color_label->setAutoFillBackground(true);
    _fps_color_label->setToolTip(tool_tip_fps_color_label_tag);

    statusBar()->addPermanentWidget(_fps_label, 0);
    statusBar()->addPermanentWidget(_fps_color_label, 0);

    // action views
    QActionGroup *view_ag = new QActionGroup(this);
    view_ag->addAction(_ui->actionScan);
    view_ag->addAction(_ui->actionCalibration);
    _ui->actionScan->setChecked(true);

    QSignalMapper *view_sm = new QSignalMapper(this);
    view_sm->setMapping(_ui->actionScan, _ui->scan_page);
    view_sm->setMapping(_ui->actionCalibration, _ui->calibration_page);
    
    view_sm->connect(_ui->actionScan, SIGNAL(triggered()), SLOT(map()));
    view_sm->connect(_ui->actionCalibration, SIGNAL(triggered()), SLOT(map()));

    _ui->stackedWidget->connect(view_sm, SIGNAL(mapped(QWidget *)), SLOT(setCurrentWidget(QWidget *)));

    // move to center
    QRect r = geometry();
    r.moveCenter(QApplication::desktop()->availableGeometry().center());
    setGeometry(r);

    reme_context_t c;
    reme_context_create(&c);
    
     // Create dialogs
    _logging_dialog = new logging_dialog(this, Qt::Dialog);
    _settings_dialog = new settings_dialog(c, this);
    _hardware_key_dialog = new hardware_key_dialog(c, this);
    _about_dialog = new about_dialog(c, this);
    _status_dialog = new status_dialog(this);

    reme_context_destroy(&c);

    // application icon
    QPixmap titleBarPix (":/images/icon.ico");
    QIcon titleBarIcon(titleBarPix);
    setWindowIcon(titleBarIcon);

    // Create 
    create_mappings();

    // ui connections
    connect(_scan_ui, SIGNAL(status_bar_msg(const QString&, const int)), SLOT(status_bar_msg(const QString&, const int)));
    connect(_scan_ui, SIGNAL(status_bar_msg(const QString&, const int)), SLOT(status_bar_msg(const QString&, const int)));
    _scan_ui->connect(_frame_grabber, SIGNAL(frame(reme_sensor_image_t, reme_image_t)), SLOT(process_frame(reme_sensor_image_t, reme_image_t)), Qt::BlockingQueuedConnection);
    _scan_ui->scanner()->connect(_frame_grabber, SIGNAL(frames_updated()), SLOT(process_frame()), Qt::BlockingQueuedConnection);
    connect(_scan_ui->scanner(), SIGNAL(current_fps(const float)), SLOT(show_fps(const float)));
    connect(_scan_ui->scanner(), SIGNAL(current_fps(const float)), SLOT(show_fps(const float)));


    // Dialog connections
    _settings_dialog->connect(_ui->actionSettings, SIGNAL(triggered()),SLOT(show()));
    _about_dialog->connect(_ui->actionAbout, SIGNAL(triggered()), SLOT(show()));
    _hardware_key_dialog->connect(_ui->actionGenerate_hardware_key, SIGNAL(triggered()), SLOT(show()));
    
    
    connect(_ui->actionLog, SIGNAL(toggled(bool)), SLOT(action_log_toggled(bool)));
    _ui->actionLog->connect(_logging_dialog, SIGNAL(close_clicked()), SLOT(toggle()));

    // sdk _initializer
    qRegisterMetaType<init_t>( "init_t" );
    _status_dialog->connect(_initializer.get(), SIGNAL(initializing(init_t)), SLOT(initializing(init_t)), Qt::BlockingQueuedConnection);
    _status_dialog->connect(_initializer.get(), SIGNAL(initialized(init_t, bool)), SLOT(initialized(init_t, bool)), Qt::BlockingQueuedConnection);

    _status_dialog->connect(_status_dialog->closeBtn(), SIGNAL(clicked()), SLOT(hide()));
    _status_dialog->connect(_status_dialog->logBtn(), SIGNAL(clicked()), SLOT(hide()));
    _logging_dialog->connect(_status_dialog->logBtn(), SIGNAL(clicked()), SLOT(show()));
    _status_dialog->connect(_initializer.get(), SIGNAL(initializing_sdk()), SLOT(reset()));
    _status_dialog->connect(_initializer.get(), SIGNAL(initializing_sdk()), SLOT(show()));
    _status_dialog->closeBtn()->connect(_initializer.get(), SIGNAL(initializing_sdk()), SLOT(hide()));
    _status_dialog->closeBtn()->connect(_initializer.get(), SIGNAL(sdk_initialized(bool)), SLOT(show()));
    _status_dialog->logBtn()->connect(_initializer.get(), SIGNAL(initializing_sdk()), SLOT(hide()));
    _status_dialog->logBtn()->connect(_initializer.get(), SIGNAL(sdk_initialized(bool)), SLOT(show()));

    _frame_grabber->connect(this, SIGNAL(closing()), SLOT(stop()), Qt::BlockingQueuedConnection);
    _initializer->connect(_settings_dialog, SIGNAL(initialize()), SLOT(initialize()));
    
    qRegisterMetaType<init_t>( "reme_log_severity_t" );
    _logging_dialog->connect(_initializer.get(), SIGNAL(log_message(reme_log_severity_t, const QString &)), SLOT(add_log_message(reme_log_severity_t, const QString &)));

    // Trigger concurrent initialization
    _initializer->initialize();

    // Shortcuts
    _ui->actionLog->setShortcut(QKeySequence("Ctrl+L"));
    _ui->actionSettings->setShortcut(QKeySequence("Ctrl+E"));

    // CLose _splashscreen
    _splash->finish(this);
  }

  void reconstructme::create_mappings() {
    _url_mapper = new QSignalMapper(this);
    _url_mapper->setMapping(_ui->actionDevice, QString(url_device_matrix_tag));
    _url_mapper->setMapping(_ui->actionFAQ, QString(url_faq_tag));
    _url_mapper->setMapping(_ui->actionForum, QString(url_forum_tag));
    _url_mapper->setMapping(_ui->actionInstallation, QString(url_install_tag));
    _url_mapper->setMapping(_ui->actionProjectHome, QString(url_reconstructme_qt));
    _url_mapper->setMapping(_ui->actionSDKDocumentation, QString(url_sdk_doku_tag));
    _url_mapper->setMapping(_ui->actionUsage, QString(url_usage_tag));
    
    _url_mapper->connect(_ui->actionDevice, SIGNAL(triggered()), SLOT(map()));
    _url_mapper->connect(_ui->actionFAQ, SIGNAL(triggered()), SLOT(map()));
    _url_mapper->connect(_ui->actionForum, SIGNAL(triggered()), SLOT(map()));
    _url_mapper->connect(_ui->actionInstallation, SIGNAL(triggered()), SLOT(map()));
    _url_mapper->connect(_ui->actionProjectHome, SIGNAL(triggered()), SLOT(map()));
    _url_mapper->connect(_ui->actionSDKDocumentation, SIGNAL(triggered()), SLOT(map()));
    _url_mapper->connect(_ui->actionUsage, SIGNAL(triggered()), SLOT(map()));

    connect(_url_mapper, SIGNAL(mapped(const QString&)), SLOT(open_url(const QString&)));
  }

  reconstructme::~reconstructme()
  {
    emit closing();
    _frame_grabber_thread->quit();
    _frame_grabber_thread->wait();
    
    delete _ui;

    delete _frame_grabber;

  }

  void reconstructme::action_log_toggled(bool checked) {
    if (checked) 
      _logging_dialog->show();
    else
      _logging_dialog->hide();
  }

  void reconstructme::action_status_toggled(bool checked) {
    if (checked) 
      _status_dialog->show();
    else
      _status_dialog->hide();
  }

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
    _settings_dialog->show();
    _settings_dialog->raise();
    _settings_dialog->activateWindow();
  }

  void reconstructme::action_about_clicked() {
    _about_dialog->show();
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
    if (fps > 20) 
      _fps_color_label->setStyleSheet("background-color: green;");
    else if (fps > 10)
      _fps_color_label->setStyleSheet("background-color: orange;");
    else
      _fps_color_label->setStyleSheet("background-color: #FF4848;");
    
    _fps_label->setText(QString().sprintf("%.2f fps", fps));
  }
}
