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
  
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once

#include <QMainWindow>

#include <reconstructmesdk/reme.h>

// Forward declarations
class QImage;
class QThread;
class QLabel;
class QSplashScreen;
class QFileDialog;
class QProgressDialog;
namespace Ui {
  class reconstructme_mw;
}
namespace ReconstructMeGUI {
  class scan;
  class QGLCanvas;
  class about_dialog;
  class settings_dialog;
  class logging_dialog;
  class hardware_key_dialog;
}

namespace ReconstructMeGUI {

  /** Main UI element of the ReconstructMeQT application 
  *
  * \note This is the topmost element of the UI-tree. It holds references to 
  *       all other UI elements, such as the necessary dialogs.
  */
  class reconstructme : public QMainWindow
  {
    Q_OBJECT
    
  public:
    explicit reconstructme(QWidget *parent = 0);
    ~reconstructme();

  private slots:
    /** Write a message to the status bar */
    void write_to_status_bar(const QString &msg, const int msecs = 0);

    /** Hide splashscreen when new settings are applied */
    void hide_splash(bool unused);
    /** Set image references from scanner */
    void set_image_references(bool has_sensor);

    // Online help
    void action_installation_clicked();
    void action_usage_clicked();
    void action_faq_clicked();
    void action_forum_clicked();  
    void action_device_clicked(); 

    /** Settings dialog */
    void action_settings_clicked();
    /** About dialog */
    void action_about_clicked();
    /** Show/Hide log dialog */
    void action_log_toggled(bool checked);
    /** Show hardware key dialog */
    void action_hardware_key_clicked();

    /** Handle save button clicked event. Trigger scanner to save current mesh */
    void save_button_clicked();
    /** Handle play button clicked event. Trigger scanner to toggle PLAY/PAUSE */
    void play_button_clicked();
    /** Handle reset button clicked event. Trigger scanner to reset current volume */
    void reset_button_clicked();

    /** Handling wait_splash hiding */
    void apply_new_sensor();
    /** Handling wait_splash hiding */
    void apply_new_context();
    /** Handling wait_splash hiding */
    void new_sensor(bool success);
    /** Handling wait_splash hiding */
    void new_context(bool success);

    /** show message box */
    void show_message_box(
      int icon,
      QString message, 
      int btn_1 = 1024, // QMessageBox::Ok
      int btn_2 = 0);   // QMessageBox::NoButton
signals:
    /** Trigger scanner to save current mesh */
    void save_mesh_to_file(const QString &s);
    /** This signal is emited when this objects constructor finished */
    void initialized(bool);

  private:
    void open_url_in_std_browser(const QString &url_string);
    void create_views();
    void create_settings();
    void create_scanner();
    void create_statusbar();

    enum Mode {PLAY, PAUSE};

    // Members
    Ui::reconstructme_mw *ui;

    // Dialogs
    settings_dialog *dialog_settings;
    logging_dialog *log_dialog;
    hardware_key_dialog *hw_key_dialog;
    about_dialog *app_about_dialog;

    // Splash screens
    QSplashScreen *splash;
    QProgressDialog *splash_wait;

    // Scanner utils
    scan *scanner;
    QThread* scanner_thread;
    Mode current_mode;

    // Context
    reme_context_t c;

    // Images & Widget
    QImage *rgb_image;
    QImage *phong_image;
    QImage *depth_image;
    QGLCanvas *rgb_canvas;
    QGLCanvas *phong_canvas;
    QGLCanvas *depth_canvas;

    // Splash wait handling
    bool wait_for_sensor;
    bool wait_for_context;
  };

}

#endif // MAINWINDOW_H
