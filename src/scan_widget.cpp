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

#include "scan_widget.h"
#include "ui_scan_widget.h"

#include "scan.h"
#include "reme_sdk_initializer.h"

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
#include <QSignalMapper>

#include <iostream>

#define STATUSBAR_TIME 1500
#define SPLASH_MSG_ALIGNMENT Qt::AlignBottom | Qt::AlignLeft

namespace ReconstructMeGUI {
  
  scan_widget::scan_widget(std::shared_ptr<reme_sdk_initializer> initializer, QWidget *parent) : 
    QWidget(parent),  
    _ui(new Ui::scan_widget),
    _initializer(initializer)
  {
    _ui->setupUi(this);

    // Create 
    create_views();    // three views
    create_scanner();  // init _scanner -> called by slot

    // interaction with _scanner
    qRegisterMetaType<init_t>( "mode_t" );
    _scanner->connect(_ui->play_button, SIGNAL(clicked()), SLOT(toggle_play_pause()));
    connect(_scanner, SIGNAL(mode_changed(mode_t)), SLOT(apply_mode(mode_t)));
    _scanner->connect(_ui->reset_button, SIGNAL(clicked()), SLOT(reset_volume()));
    connect(_ui->reset_button, SIGNAL(clicked()), SLOT(reset_button_clicked()));
    _scanner->connect(this, SIGNAL(save_mesh_to_file(const QString &)), SLOT(save(const QString &)));
    connect(_ui->save_button, SIGNAL(clicked()), SLOT(save_button_clicked()));

    // views
    _rgb_canvas->connect(_initializer.get(),   SIGNAL(rgb_size(const QSize*)),   SLOT(set_image_size(const QSize*)), Qt::BlockingQueuedConnection);
    _depth_canvas->connect(_initializer.get(), SIGNAL(depth_size(const QSize*)), SLOT(set_image_size(const QSize*)), Qt::BlockingQueuedConnection);
    _phong_canvas->connect(_initializer.get(), SIGNAL(phong_size(const QSize*)), SLOT(set_image_size(const QSize*)), Qt::BlockingQueuedConnection);

    _rgb_canvas->connect(_scanner,   SIGNAL(new_rgb_image_bits(const void*)),   SLOT(set_image_data(const void*)));
    _depth_canvas->connect(_scanner, SIGNAL(new_depth_image_bits(const void*)), SLOT(set_image_data(const void*)));
    _phong_canvas->connect(_scanner, SIGNAL(new_phong_image_bits(const void*)), SLOT(set_image_data(const void*)));

    // Shortcuts
    _ui->play_button->setShortcut(QKeySequence("Ctrl+P"));
    _ui->reset_button->setShortcut(QKeySequence("Ctrl+R"));
    _ui->save_button->setShortcut(QKeySequence("Ctrl+S"));
  }

  void scan_widget::create_views() {
    // Create viewes and add to layout
    QString def_img(":/images/no_image_available.png");
    _rgb_canvas   = new QGLCanvas(def_img);
    _phong_canvas = new QGLCanvas(def_img);
    _depth_canvas = new QGLCanvas(def_img);

    //                                         r  c rs cs
    _ui->view_layout->addWidget(_phong_canvas, 0, 0, 2, 1);
    _ui->view_layout->addWidget(_rgb_canvas,   0, 1, 1, 1);
    _ui->view_layout->addWidget(_depth_canvas, 1, 1, 1, 1);
    _ui->view_layout->setColumnStretch(0, 2);
    _ui->view_layout->setColumnStretch(1, 1);
  }

  void scan_widget::create_scanner() {
    // do not change order, due to a connect in the scan 
    _scanner = new scan(_initializer);
    // scan thread
    _scanner_thread = new QThread(this);
    _scanner->moveToThread(_scanner_thread);
    _scanner_thread->start();
  }

  scan_widget::~scan_widget()
  {
    _scanner->stop();
    _scanner_thread->quit();
    _scanner_thread->wait();
    
    delete _scanner;
    delete _scanner_thread;

    delete _ui;
  }

  void scan_widget::save_button_clicked()
  {
    QString file_name = QFileDialog::getSaveFileName(this, tr("Save 3D Model"),
                                                 QDir::currentPath(),
                                                 tr("PLY files (*.ply);;OBJ files (*.obj);;3DS files (*.3ds);;STL files (*.stl)"),
                                                 0);
    if (file_name.isEmpty())
      return;

    emit save_mesh_to_file(file_name);
    status_bar_msg(saving_to_tag + file_name, STATUSBAR_TIME);
  }

  void scan_widget::apply_mode(mode_t current__scanner_mode) {
    QPushButton* playPause_b = _ui->play_button;
    QPixmap pixmap;
    if (current__scanner_mode != PLAY) {
      _ui->save_button->setEnabled(true);
      pixmap.load(":/images/record-button.png");
      status_bar_msg(mode_pause_tag, STATUSBAR_TIME);
    }
    else {
      _ui->save_button->setDisabled(true);
      pixmap.load(":/images/pause-button.png");
      status_bar_msg(mode_play_tag, STATUSBAR_TIME);
    }
    QIcon icon(pixmap);
    playPause_b->setIcon(icon);
  }

  void scan_widget::reset_button_clicked() {
    status_bar_msg(volume_resetted_tag, STATUSBAR_TIME);
  }

  const scan *scan_widget::scanner() const {
    return _scanner;
  }
}
