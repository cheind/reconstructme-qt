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

#include "settings.h"
#include "strings.h"
#include "settings_dialog.h"
#include "scan.h"
#include "qglcanvas.h"
#include "logging_dialog.h"
#include "hardware_key_dialog.h"
#include "defines.h"

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

#include <sstream>
#include <iostream>

#define STATUSBAR_TIME 1500
#define SPLASH_MSG_ALIGNMENT Qt::AlignBottom | Qt::AlignLeft

namespace ReconstructMeGUI {
  
  void reme_log(reme_log_severity_t sev, const char *message, void *user_data)  {
    logging_dialog *l = static_cast<logging_dialog*>(user_data);
    l->append_log_message(QString(message));
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
    
    // shortcuts
    ui->play_button->setShortcut(QKeySequence("Ctrl+P"));
    ui->reset_button->setShortcut(QKeySequence("Ctrl+R"));
    ui->actionSave->setShortcut(QKeySequence("Ctrl+S"));
    ui->actionLog->setShortcut(QKeySequence("Ctrl+L"));
    ui->actionSettings->setShortcut(QKeySequence("Ctrl+E"));

     // Logger
    log_dialog = new logging_dialog(this, Qt::Dialog);
    ui->actionLog->connect(log_dialog, SIGNAL(close_clicked()), SLOT(toggle()));

    reme_context_create(&c);
    reme_context_set_log_callback(c, reme_log, log_dialog);

    QRect r = geometry();
    r.moveCenter(QApplication::desktop()->availableGeometry().center());
    setGeometry(r);

    dialog_settings = 0;
    splash_wait = 0;
    save_dialog = 0;
    hw_key_dialog = 0;

    QPixmap titleBarPix (":/images/icon.ico");
    QIcon titleBarIcon(titleBarPix);
    setWindowIcon(titleBarIcon);

    splash->showMessage(create_views_tag, SPLASH_MSG_ALIGNMENT);
    create_views();    // three views
    splash->showMessage(reload_settings_tag, SPLASH_MSG_ALIGNMENT);
    create_settings(); // load settings
    splash->showMessage(init_scanner_tag, SPLASH_MSG_ALIGNMENT);
    create_scanner();  // init scanner -> called by slot
    create_filedialog();
    current_mode = PAUSE;

    // Define connections
    connect(ui->actionInstallation, SIGNAL(triggered()), SLOT(action_installation_clicked()));
    connect(ui->actionDevice, SIGNAL(triggered()), SLOT(action_device_clicked()));
    connect(ui->actionUsage, SIGNAL(triggered()), SLOT(action_usage_clicked()));
    connect(ui->actionFAQ, SIGNAL(triggered()), SLOT(action_faq_clicked()));
    connect(ui->actionForum, SIGNAL(triggered()), SLOT(action_forum_clicked()));
    connect(ui->actionSettings, SIGNAL(triggered()),SLOT(action_settings_clicked()));
    connect(ui->actionAbout, SIGNAL(triggered()), SLOT(action_about_clicked()));
    connect(ui->actionLog, SIGNAL(toggled(bool)), SLOT(action_log_toggled(bool)));
    connect(ui->actionSave, SIGNAL(triggered()), SLOT(save_button_clicked()));
    connect(ui->actionGenerate_hardware_key, SIGNAL(triggered()), SLOT(action_hardware_key_clicked()));

    connect(ui->play_button, SIGNAL(clicked()), SLOT(play_button_clicked()));
    connect(ui->save_button, SIGNAL(clicked()), SLOT(save_button_clicked()));
    connect(ui->reset_button, SIGNAL(clicked()), SLOT(reset_button_clicked()));
    
    scanner->connect(dialog_settings, SIGNAL(opencl_settings_changed()), SLOT(initialize()));
    scanner->connect(dialog_settings, SIGNAL(sensor_changed()), SLOT(create_sensor()));
    
    connect(scanner, SIGNAL(initialized(bool)), SLOT(hide_splash(bool)));
    connect(scanner, SIGNAL(sensor_created(bool)), SLOT(hide_splash(bool)));

    scanner->connect(this, SIGNAL(initialized(bool)), SLOT(run(bool)));
    emit initialized(true);

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

  void reconstructme::set_image_references(bool has_sensor) {
    // this is called, when a new sensor was created.
    if (!has_sensor) return;
    
    rgb_canvas->setImage(scanner->get_rgb_image());
    phong_canvas->setImage(scanner->get_phong_image());
    depth_canvas->setImage(scanner->get_depth_image());
  }

  void reconstructme::create_settings() {
    // create dialog
    dialog_settings = new settings_dialog(c,this);
    
    // create splashscreen for loading
    QPixmap splash_pix(":/images/wait_splash.png");
    splash_wait = new QSplashScreen(this, splash_pix, Qt::WindowStaysOnTopHint);
    splash_wait->showMessage(loading_settings_tag);
    splash_wait->connect(dialog_settings, SIGNAL(opencl_settings_changed()), SLOT(show()));
    splash_wait->connect(dialog_settings, SIGNAL(sensor_changed()), SLOT(show()));
  }

  void reconstructme::create_scanner() {
    scanner = new scan(c);
    
    // set connections to scanner
    connect(scanner, SIGNAL(licence_error_code(int)), SLOT(hanlde_licence_error(int)));
    connect(scanner, SIGNAL(sensor_created(bool)), SLOT(set_image_references(bool)));
    
    // logger
    log_dialog->connect(scanner, SIGNAL(log_message(const QString&)), SLOT(append_log_message(const QString &)));
        
    // button handler
    scanner->connect(ui->play_button, SIGNAL(clicked()), SLOT(toggle_play_pause()));
    scanner->connect(ui->reset_button, SIGNAL(clicked()), SLOT(reset_volume()));
    scanner->connect(this, SIGNAL(save_mesh_to_file(const QString &)), SLOT(save(const QString &)));

    // NOTE: Qt::QueuedConnection has to be defined explicity here. 
    // If Qt::AutoConnections will be used, a direct connection is choosen by Qt, 
    // since receiver object and emitter object are in the same thread.
    // http://qt-project.org/doc/qt-4.8/qt.html#ConnectionType-enum
    scanner->connect(scanner, SIGNAL(sensor_created(bool)), SLOT(run(bool)), Qt::QueuedConnection); 
    scanner->connect(scanner, SIGNAL(initialized(bool)), SLOT(run(bool)), Qt::QueuedConnection);
    
    // feedback from scanner
    statusBar()->connect(scanner, SIGNAL(status_string(const QString &, const int)), SLOT(showMessage(const QString &, const int)));

    // views update
    rgb_canvas->connect(scanner, SIGNAL(new_rgb_image_bits()), SLOT(update()));
    depth_canvas->connect(scanner, SIGNAL(new_depth_image_bits()), SLOT(update()));
    phong_canvas->connect(scanner, SIGNAL(new_phong_image_bits()), SLOT(update()));

    // start initializing sensor
    splash->showMessage(init_opencl_tag, SPLASH_MSG_ALIGNMENT);
    scanner->initialize();

    // find a sensor
    splash->showMessage(init_sensor_tag, SPLASH_MSG_ALIGNMENT);
    bool sensor_found = scanner->create_sensor();
    while (!sensor_found && QMessageBox::Retry == QMessageBox::information(this, no_sensor_found_tag, no_sensor_found_msg_tag, QMessageBox::Ok, QMessageBox::Retry))
      sensor_found = scanner->create_sensor();
    
    // scan thread
    scanner_thread = new QThread(this);
    scanner->moveToThread(scanner_thread);
    scanner_thread->start();
  }

  void reconstructme::create_filedialog() {
    save_dialog = new QFileDialog(this, Qt::Dialog);
    
    save_dialog->setAcceptMode(QFileDialog::AcceptSave);
    save_dialog->setFileMode(QFileDialog::AnyFile);
    save_dialog->setDirectory(QDir::currentPath());
    save_dialog->setFilter("*.ply;; *.obj;; *.3ds;; Text based *.stl");
    save_dialog->setNameFilter(".ply;; .obj;; .3ds;; .stl");
  }

  reconstructme::~reconstructme()
  {
    scanner->request_stop();
    scanner_thread->quit();
    scanner_thread->wait();
    
    delete scanner;
    delete scanner_thread;

    delete ui;

    reme_context_destroy(&c);
  }

  void reconstructme::save_button_clicked()
  {
    QStringList selected_files;
    if(save_dialog->exec()) 
      selected_files = save_dialog->selectedFiles();
    
    if (selected_files.empty())
      return;
    
    QString file_name = selected_files[0];
    if(!file_name.endsWith(save_dialog->selectedFilter()))
      file_name += save_dialog->selectedFilter();

    emit save_mesh_to_file(file_name);
    ui->reconstruct_satus_bar->showMessage(saving_to_tag + file_name, STATUSBAR_TIME);
  }

  void reconstructme::play_button_clicked()
  {
    QPushButton* playPause_b = ui->play_button;
    
    // toggle mode
    current_mode = (current_mode == PLAY) ? PAUSE : PLAY;
    
    if (current_mode == PAUSE) {
      ui->save_button->setDisabled(false);
      QPixmap changePixPlay (":/images/record-button.png");
      QIcon newIconPlay (changePixPlay);
      playPause_b->setIcon(newIconPlay);
      ui->reconstruct_satus_bar->showMessage(mode_pause_tag, STATUSBAR_TIME);
    }
    else if (current_mode == PLAY) {
      QPixmap changePixPause (":/images/pause-button.png");
      QIcon newIconPause (changePixPause);
      playPause_b->setIcon(newIconPause);
      ui->reconstruct_satus_bar->showMessage(mode_play_tag, STATUSBAR_TIME);
      ui->save_button->setDisabled(true);
    }
  }

  void reconstructme::reset_button_clicked() {
    ui->reconstruct_satus_bar->showMessage(volume_resetted_tag, STATUSBAR_TIME);
  }

  void reconstructme::action_settings_clicked() {
    dialog_settings->show();
    dialog_settings->raise();
    dialog_settings->activateWindow();
  }

  void reconstructme::hanlde_licence_error(int error) {
    if (error == REME_ERROR_INVALID_LICENSE)
      QMessageBox::information(this, license_info_tag, invalid_license_tag, QMessageBox::Ok);
    else if (error == REME_ERROR_UNSPECIFIED)
      QMessageBox::information(this, license_info_tag, license_unspecified_tag, QMessageBox::Ok);
  }

  void reconstructme::hide_splash(bool unused) {
    splash_wait->hide();
  }

  void reconstructme::action_log_toggled(bool checked) {
    if (checked) {
      log_dialog->align_to_parent();
      log_dialog->show();
    }
    else
      log_dialog->hide();
  }

  void reconstructme::action_hardware_key_clicked() {
    if (!hw_key_dialog)
      hw_key_dialog = new hardware_key_dialog(c, this);
    hw_key_dialog->show();
  }

  void reconstructme::action_about_clicked() {
    QMessageBox msgbox;
    msgbox.setText(application_about_tag);
                          
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

    std::stringstream ss_about;
    ss_about << "Version ReconstructMeQT: " << ss_gui_v.str() << "\n";
    ss_about << "Build Version ReconstructMeSDK: " << ss_sdk_v.str() << "\n";
    ss_about << "Runtime Version ReconstructMeSDK: " << ss_sdk_v.str() << "\n";
    ss_about << "http://reconstructme.net/ ";
    ss_about << "\nChristoph Heindl";
    ss_about << "\nChristoph Kopf";
    ss_about << "\nFlorian Eckerstorfer";

    QString qstr = QString::fromStdString(ss_about.str());

    msgbox.setInformativeText(qstr);
    QPixmap boxpix(":/images/reme_typo.png");
    msgbox.setIconPixmap(boxpix);
    msgbox.exec();
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
