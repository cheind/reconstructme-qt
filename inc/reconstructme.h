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
  */
  
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once

#include "types.h"

#include <QMainWindow>

#include <reconstructmesdk/types.h>

// Forward declarations
class QImage;
class QThread;
class QLabel;
class QFileDialog;
class QProgressDialog;
class QSignalMapper;
namespace Ui {
  class reconstructmeqt;
}
namespace ReconstructMeGUI {
  class scan;
  class QGLCanvas;
  class about_dialog;
  class settings_dialog;
  class logging_dialog;
  class status_dialog;
  class hardware_key_dialog;
  class reme_resource_manager;
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
    Q_OBJECT;
    
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

    /** show message box */
    void show_message_box(
      int icon,
      QString message, 
      int btn_1 = 1024, // QMessageBox::Ok
      int btn_2 = 0);   // QMessageBox::NoButton

    void toggle_mode();

    void really_close();
    void show_frame(reme_sensor_image_t type, const void* data, int length, int width, int height, int channels, int num_bytes_per_channel, int row_stride);

  signals:
    /** This signal is emited when this objects constructor finished */
    void initialize();
    void closing();
    void start_scanning();
    void stop_scanning();

  protected:
     void	closeEvent(QCloseEvent *event);

  private:
    void create_url_mappings();

    QSignalMapper *_url_mapper;

    // Members
    Ui::reconstructmeqt *_ui;
    
    QLabel *_label_fps;
    QLabel *_label_fps_color;

    // Dialogs
    settings_dialog *_dialog_settings;
    logging_dialog *_dialog_log;
    hardware_key_dialog *_dialog_license;
    about_dialog *_dialog_about;
    status_dialog *_dialog_state;

    // utils
    std::shared_ptr<reme_resource_manager> _rm;
    std::shared_ptr<frame_grabber> _fg;
    QThread* _rm_thread;
   
    mode_t _mode;
  };
}

#endif // MAINWINDOW_H
