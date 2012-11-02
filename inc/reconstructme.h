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

#include "types.h"

#include <reconstructmesdk/types.h>

// Forward declarations
class QImage;
class QThread;
class QLabel;
class QSplashScreen;
class QFileDialog;
class QProgressDialog;
class QSignalMapper;
namespace Ui {
  class reconstructmeqt;
}
namespace ReconstructMeGUI {
  class scan_widget;
  class calibration_widget;
  class scan;
  class QGLCanvas;
  class about_dialog;
  class settings_dialog;
  class logging_dialog;
  class status_dialog;
  class hardware_key_dialog;
  class reme_sdk_initializer;
  class frame_grabber;
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
    void status_bar_msg(const QString &msg, const int msecs = 0);
    void show_fps(const float fps);

    // Online help
    void open_url(const QString &url_string);

    /** Settings dialog */
    void action_settings_clicked();
    /** About dialog */
    void action_about_clicked();
    /** Show/Hide log dialog */
    void action_log_toggled(bool checked);
    /** Show/Hide log dialog */
    void action_status_toggled(bool checked);

    /** show message box */
    void show_message_box(
      int icon,
      QString message, 
      int btn_1 = 1024, // QMessageBox::Ok
      int btn_2 = 0);   // QMessageBox::NoButton
  signals:
    /** This signal is emited when this objects constructor finished */
    void initialize();
    void closing();

  private:
    void create_mappings();

    QSignalMapper *_url_mapper;

    // Members
    Ui::reconstructmeqt *_ui;
    scan_widget *_scan_ui;
    calibration_widget *_calibration_ui;

    QLabel *_fps_label;
    QLabel *_fps_color_label;

    // Dialogs
    settings_dialog *_settings_dialog;
    logging_dialog *_logging_dialog;
    hardware_key_dialog *_hardware_key_dialog;
    about_dialog *_about_dialog;
    status_dialog *_status_dialog;

    // Splash screens
    QSplashScreen *_splash;

    // utils
    std::shared_ptr<reme_sdk_initializer> _initializer;
    frame_grabber* _frame_grabber;
    QThread* _frame_grabber_thread;
  };
}

#endif // MAINWINDOW_H
