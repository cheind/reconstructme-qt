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

#include "reconstructme.h"
#include "ui_reconstructme.h"

#include "scan.h"
#include "reme_sdk_initializer.h"

#include "settings.h"
#include "strings.h"
#include "defines.h"
#include "types.h"

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

#define STATUSBAR_TIME 1500
#define SPLASH_MSG_ALIGNMENT Qt::AlignBottom | Qt::AlignLeft

namespace ReconstructMeGUI {
  
  void reme_log(reme_log_severity_t sev, const char *message, void *user_data)  {
    logging_dialog *l = static_cast<logging_dialog*>(user_data);
    l->add_log_message(sev, QString(message));
  }
  
  reconstructme::reconstructme(QWidget *parent) : 
    QMainWindow(parent),  
    ui(new Ui::reconstructme_mw)
  {
    // Splashscreen
    QPixmap splashPix(":/images/splash_screen.png");
    splash = new QSplashScreen(this, splashPix);
    splash->setAutoFillBackground(false);
    splash->showMessage(welcome_tag, SPLASH_MSG_ALIGNMENT);
    splash->show();

    ui->setupUi(this);
    
    // move to center
    QRect r = geometry();
    r.moveCenter(QApplication::desktop()->availableGeometry().center());
    setGeometry(r);

    reme_context_create(&c);
    
     // logger
    log_dialog = new logging_dialog(this, Qt::Dialog);
    reme_context_set_log_callback(c, reme_log, log_dialog);
    
    dialog_settings = new settings_dialog(c,this);
    hw_key_dialog = new hardware_key_dialog(c, this);
    app_about_dialog = new about_dialog(c, this);
    init_status_dialog = new status_dialog(this);

    QPixmap titleBarPix (":/images/icon.ico");
    QIcon titleBarIcon(titleBarPix);
    setWindowIcon(titleBarIcon);

    splash->showMessage(create_views_tag, SPLASH_MSG_ALIGNMENT);
    QApplication::processEvents();
    create_views();    // three views
    splash->showMessage(init_scanner_tag, SPLASH_MSG_ALIGNMENT);
    QApplication::processEvents();
    create_scanner();  // init scanner -> called by slot
    
    //// Define connections
    connect(ui->actionInstallation, SIGNAL(triggered()), SLOT(action_installation_clicked()));
    connect(ui->actionDevice, SIGNAL(triggered()), SLOT(action_device_clicked()));
    connect(ui->actionUsage, SIGNAL(triggered()), SLOT(action_usage_clicked()));
    connect(ui->actionFAQ, SIGNAL(triggered()), SLOT(action_faq_clicked()));
    connect(ui->actionForum, SIGNAL(triggered()), SLOT(action_forum_clicked()));
    connect(ui->actionSettings, SIGNAL(triggered()),SLOT(action_settings_clicked()));
    connect(ui->actionAbout, SIGNAL(triggered()), SLOT(action_about_clicked()));
    connect(ui->actionSave, SIGNAL(triggered()), SLOT(save_button_clicked()));
    connect(ui->actionGenerate_hardware_key, SIGNAL(triggered()), SLOT(action_hardware_key_clicked()));
    connect(ui->actionLog, SIGNAL(toggled(bool)), SLOT(action_log_toggled(bool)));
    ui->actionLog->connect(log_dialog, SIGNAL(close_clicked()), SLOT(toggle()));

    
    // button handler
    scanner->connect(ui->play_button, SIGNAL(clicked()), SLOT(toggle_play_pause()));
    connect(ui->play_button, SIGNAL(clicked()), SLOT(play_button_clicked()));
    scanner->connect(ui->reset_button, SIGNAL(clicked()), SLOT(reset_volume()));
    connect(ui->reset_button, SIGNAL(clicked()), SLOT(reset_button_clicked()));
    connect(ui->save_button, SIGNAL(clicked()), SLOT(save_button_clicked()));
    scanner->connect(this, SIGNAL(save_mesh_to_file(const QString &)), SLOT(save(const QString &)));
    
    // views update
    connect(initializer, SIGNAL(initialized_images()), SLOT(set_image_references()));
    rgb_canvas->connect(scanner, SIGNAL(new_rgb_image_bits()), SLOT(update()));
    depth_canvas->connect(scanner, SIGNAL(new_depth_image_bits()), SLOT(update()));
    phong_canvas->connect(scanner, SIGNAL(new_phong_image_bits()), SLOT(update()));

    qRegisterMetaType<init_t>( "init_t" );
    init_status_dialog->connect(initializer, SIGNAL(initializing(init_t)), SLOT(initializing(init_t)));
    init_status_dialog->connect(initializer, SIGNAL(initialized(init_t, bool)), SLOT(initialized(init_t, bool)));

    initializer->connect(dialog_settings, SIGNAL(initialize(init_t)), SLOT(initialize(init_t)));
    initializer->connect(this, SIGNAL(initialize(init_t)), SLOT(initialize(init_t)));
    
    emit initialize(OPENCL);
    emit initialize(LICENSE);
    emit initialize(SENSOR);

    // shortcuts
    ui->play_button->setShortcut(QKeySequence("Ctrl+P"));
    ui->reset_button->setShortcut(QKeySequence("Ctrl+R"));
    ui->actionSave->setShortcut(QKeySequence("Ctrl+S"));
    ui->actionLog->setShortcut(QKeySequence("Ctrl+L"));
    ui->actionSettings->setShortcut(QKeySequence("Ctrl+E"));

    splash->finish(this);
  }

  void reconstructme::create_views() {
    // Create viewes and add to layout
    rgb_canvas   = new QGLCanvas();
    phong_canvas = new QGLCanvas();
    depth_canvas = new QGLCanvas();

    //                                       r  c rs cs
    ui->view_layout->addWidget(phong_canvas, 0, 0, 2, 1);
    ui->view_layout->addWidget(rgb_canvas,   0, 1, 1, 1);
    ui->view_layout->addWidget(depth_canvas, 1, 1, 1, 1);
    ui->view_layout->setColumnStretch(0, 2);
    ui->view_layout->setColumnStretch(1, 1);
  }

  void reconstructme::set_image_references() {
    rgb_canvas->setImage(initializer->rgb_image());
    phong_canvas->setImage(initializer->phong_image());
    depth_canvas->setImage(initializer->depth_image());
  }

  void reconstructme::create_scanner() {
    // do not change order, due to a connect in the scan constructor
    initializer = new reme_sdk_initializer(c);
    
    scanner = new scan(initializer);
    // scan thread
    scanner_thread = new QThread(this);
    scanner->moveToThread(scanner_thread);    
    initializer->moveToThread(scanner_thread);
    scanner_thread->start();
  }

  reconstructme::~reconstructme()
  {
    scanner->stop();
    scanner_thread->quit();
    scanner_thread->wait();
    
    delete scanner;
    delete scanner_thread;
    delete initializer;

    delete ui;

    reme_context_destroy(&c);
  }

  void reconstructme::save_button_clicked()
  {
    QString file_name = QFileDialog::getSaveFileName(this, tr("Save 3D Model"),
                                                 QDir::currentPath(),
                                                 tr("PLY files (*.ply);;OBJ files (*.obj);;3DS files (*.3ds);;STL files (*.stl)"),
                                                 0);
    if (file_name.isEmpty())
      return;

    emit save_mesh_to_file(file_name);
    ui->reconstruct_satus_bar->showMessage(saving_to_tag + file_name, STATUSBAR_TIME);
  }

  void reconstructme::play_button_clicked()
  {
    const mode_t current_scanner_mode = scanner->get_current_mode();
    
    if (current_scanner_mode == NOT_RUN) return;

    QPushButton* playPause_b = ui->play_button;
    QPixmap pixmap;
    if (current_scanner_mode == PAUSE) {
      ui->save_button->setEnabled(true);
      ui->actionSave->setEnabled(true);
      pixmap.load(":/images/record-button.png");
      ui->reconstruct_satus_bar->showMessage(mode_pause_tag, STATUSBAR_TIME);
    }
    else if (current_scanner_mode == PLAY) {
      ui->save_button->setDisabled(true);
      ui->actionSave->setDisabled(true);
      pixmap.load(":/images/pause-button.png");
      ui->reconstruct_satus_bar->showMessage(mode_play_tag, STATUSBAR_TIME);
    }
    QIcon icon(pixmap);
    playPause_b->setIcon(icon);
  }

  void reconstructme::reset_button_clicked() {
    ui->reconstruct_satus_bar->showMessage(volume_resetted_tag, STATUSBAR_TIME);
  }

  void reconstructme::action_settings_clicked() {
    dialog_settings->show();
    dialog_settings->raise();
    dialog_settings->activateWindow();
  }

  void reconstructme::action_log_toggled(bool checked) {
    if (checked) 
      log_dialog->show();
    else
      log_dialog->hide();
  }

  void reconstructme::action_hardware_key_clicked() {
    if (!hw_key_dialog)
      hw_key_dialog = new hardware_key_dialog(c, this);
    hw_key_dialog->show();
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

  void reconstructme::action_about_clicked() {
    app_about_dialog->show();
  }

  void reconstructme::open_url_in_std_browser(const QString &url_string) {
    QUrl url (url_string);
    QDesktopServices::openUrl(url);
    QString msg = open_url_tag + url_string;
    ui->reconstruct_satus_bar->showMessage(msg, STATUSBAR_TIME);
  }

  void reconstructme::action_installation_clicked() {
    open_url_in_std_browser(url_install_tag);
  }

  void reconstructme::action_device_clicked() {
    open_url_in_std_browser(url_device_matrix_tag);
  }

  void reconstructme::action_usage_clicked() {
    open_url_in_std_browser(url_usage_tag);
  }

  void reconstructme::action_faq_clicked() {
    open_url_in_std_browser(url_faq_tag);
  }

  void reconstructme::action_forum_clicked() {
    open_url_in_std_browser(url_forum_tag);
  }

  void reconstructme::write_to_status_bar(const QString &msg, const int msecs) {
    statusBar()->showMessage(msg, msecs);
  }
}
