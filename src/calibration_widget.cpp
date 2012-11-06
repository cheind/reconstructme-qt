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

#include "calibration_widget.h"
#include "ui_calibration_widget.h"

#include "qglcanvas.h"

#include "calibrate.h"
#include "reme_resource_manager.h"

#include "settings.h"

#include "reconstructmesdk/reme.h"

#include <QPushButton>
#include <QThread> 
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QSettings>

#include <iostream>

#define STATUS_MSG_TIME 2000

namespace ReconstructMeGUI {

  calibration_widget::calibration_widget(std::shared_ptr<reme_resource_manager> initializer, QWidget *parent) : 
    QWidget(parent),
    _ui(new Ui::calibration_widget),
    _i(initializer)
  {
    _ui->setupUi(this);

    _add_next_frame = false;

    // Create viewes and add to layout
    _aux_canvas = new QGLCanvas(QString(":/images/no_image_available.png"));
    _calib_canvas = new QGLCanvas(QString(":/images/no_image_available.png"));

    _ui->horizontalLayout->addWidget(_aux_canvas);
    _ui->horizontalLayout->addWidget(_calib_canvas);
    
    _aux_canvas->connect(_i.get(), SIGNAL(rgb_size(const QSize*)), SLOT(set_image_size(const QSize*)), Qt::BlockingQueuedConnection);
    _calib_canvas->connect(_i.get(), SIGNAL(rgb_size(const QSize*)), SLOT(set_image_size(const QSize*)), Qt::BlockingQueuedConnection);

    connect(_ui->addpicture_btn, SIGNAL(clicked()), SLOT(add_next_frame()));
    connect(_ui->calibrate_btn, SIGNAL(clicked()), SLOT(calibrate()));
  }

  calibration_widget::~calibration_widget() {
    delete _ui;
  }

  void calibration_widget::initialize(bool success) {
    reme_calibrator_create(_i->context(), &_calibrator);
    reme_image_create(_i->context(), &_calibrate_image);
  }

  void calibration_widget::process_frame(reme_sensor_image_t type, reme_image_t img) {
    if (isVisible() && type == REME_IMAGE_AUX) {
      const void * data;
      int length;

      reme_image_get_bytes(_i->context(), img, &data, &length);
      _aux_canvas->set_image_data(data);

      if (_add_next_frame)
      {
        reme_calibrator_add_image(_i->context(), _calibrator, img);
        reme_calibrator_update_detection_image(_i->context(), _calibrator, _calibrate_image);

        reme_image_get_bytes(_i->context(), _calibrate_image, &data, &length);
        _calib_canvas->set_image_data(data);

        _add_next_frame = false;
      }
    }
  }

  void calibration_widget::add_next_frame() {
    _add_next_frame = true;
    _img_cnt++;
    if (_img_cnt > 10)
      _ui->calibrate_btn->setEnabled(true);
  }

  void calibration_widget::calibrate() {
    // Actual calibration step
    float accuracy;
    if (REME_SUCCESS(reme_calibrator_calibrate(_i->context(), _calibrator, &accuracy))) {
      status_bar_msg(QString("Reprojection error: %1 pixels \n").arg(accuracy), STATUS_MSG_TIME);

      // Generate a new sensor config using the new intrinsic values
      reme_options_t o, o_intr, o_intr_new;
    
      reme_options_create(_i->context(), &o);
      reme_options_create(_i->context(), &o_intr);
      reme_options_create(_i->context(), &o_intr_new);
      // Get the current camera configuration
      reme_sensor_bind_camera_options(_i->context(), _i->sensor(), o);
      reme_options_bind_message(_i->context(), o, "depth_intrinsics", o_intr);
      // Get the result of the calibration
      reme_calibrator_bind_intrinsics(_i->context(), _calibrator, o_intr_new);
    
      // Copy over
      reme_options_copy(_i->context(), o_intr_new, o_intr);

      QString sensor_file_name = QFileDialog::getSaveFileName(this, "Choose Save Filename", QDir::current().absolutePath(), QString("*.txt"));

      // Save new config to file
      reme_options_save_to_file(_i->context(), o, sensor_file_name.toStdString().c_str());

      //if (QMessageBox::Yes == QMessageBox::information(this, "Calibrated Sensor Configuration", "Apply the generated sensor configuration?", QMessageBox::Yes, QMessageBox::No)) {
      //  QSettings settings(profactor_tag, reme_tag, this);
      //  settings.setValue(sensor_path_tag, sensor_file_name);
      //}
    } else {
      status_bar_msg("Calibration failed", STATUS_MSG_TIME);
    }
  }

  void calibration_widget::showEvent(QShowEvent* event) {
    _img_cnt = 0;
    _calibrator = _i->new_calibrator();
    _calibrate_image = _i->new_image();
  }

  void calibration_widget::hideEvent(QHideEvent* event) {
    //if (_calibrator != REME_ERROR_UNSPECIFIED)
    //  _i->destroy_calibrator(_calibrator);
    //if (_calibrate_image != REME_ERROR_UNSPECIFIED)
    //  _i->destroy_image(_calibrate_image);
  }
}