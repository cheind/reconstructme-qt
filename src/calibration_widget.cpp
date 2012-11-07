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

#include "calibration.pb.h"
#include "reme_resource_manager.h"

#include "settings.h"

#include "reconstructmesdk/reme.h"

#include <QPushButton>
#include <QThread> 
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QSettings>
#include <QString>

#include <iostream>

#define STATUS_MSG_TIME 2000
#define MIN_CALIB_IMAGES 10
#define PROGRESSBAR_LABEL_TEXT "%1/%2 Images"

namespace ReconstructMeGUI {

  calibration_widget::calibration_widget(std::shared_ptr<reme_resource_manager> initializer, QWidget *parent) : 
    QWidget(parent),
    _ui(new Ui::calibration_widget),
    _i(initializer)
  {
    _ui->setupUi(this);

    _add_next_frame = false;
    _calibrator = REME_ERROR_UNSPECIFIED;
    _calibrate_image = REME_ERROR_UNSPECIFIED;

    _ui->progressBar->setMinimum(0);
    _ui->progressBar->setMaximum(MIN_CALIB_IMAGES);

    // Create viewes and add to layout
    _aux_canvas = new QGLCanvas(QString(":/images/no_image_available.png"));
    _calib_canvas = new QGLCanvas(QString(":/images/no_image_available.png"));

    _ui->horizontalLayout->addWidget(_aux_canvas);
    _ui->horizontalLayout->addWidget(_calib_canvas);
    
    _aux_canvas->connect(_i.get(), SIGNAL(rgb_size(const QSize*)), SLOT(set_image_size(const QSize*)), Qt::BlockingQueuedConnection);
    _calib_canvas->connect(_i.get(), SIGNAL(rgb_size(const QSize*)), SLOT(set_image_size(const QSize*)), Qt::BlockingQueuedConnection);

    connect(_ui->addpicture_btn, SIGNAL(clicked()), SLOT(add_next_frame()));
    connect(_ui->calibrate_btn, SIGNAL(clicked()), SLOT(calibrate()));
    connect(_ui->reset_btn, SIGNAL(clicked()), SLOT(apply_calibration_setting()));

    connect(_ui->ich_sb, SIGNAL(valueChanged(int)), SLOT(apply_calibration_setting()));
    connect(_ui->icw_sb, SIGNAL(valueChanged(int)), SLOT(apply_calibration_setting()));
    connect(_ui->rh_sp,  SIGNAL(valueChanged(int)), SLOT(apply_calibration_setting()));
    connect(_ui->rw_sp,  SIGNAL(valueChanged(int)), SLOT(apply_calibration_setting()));
    connect(_ui->lss_sp, SIGNAL(valueChanged(double)), SLOT(apply_calibration_setting()));
    connect(_ui->zk_cb,  SIGNAL(stateChanged(int)), SLOT(apply_calibration_setting()));
    connect(_ui->zt_cb,  SIGNAL(stateChanged(int)), SLOT(apply_calibration_setting()));
  }

  calibration_widget::~calibration_widget() {
    delete _ui;
  }

  void calibration_widget::process_frame(reme_sensor_image_t type, reme_image_t img) {
    if (isVisible() && type == REME_IMAGE_AUX) {
      const void * data;
      int length;

      if (REME_SUCCESS(reme_image_get_bytes(_i->context(), img, &data, &length)))
        _aux_canvas->set_image_data(data);

      if (_add_next_frame)
      {
        _img_cnt += REME_SUCCESS(reme_calibrator_add_image(_i->context(), _calibrator, img)) ? 1 : 0;
        reme_calibrator_get_detection_image(_i->context(), _calibrator, _calibrate_image);
        
        if (REME_SUCCESS(reme_image_get_bytes(_i->context(), _calibrate_image, &data, &length)))
          _calib_canvas->set_image_data(data);

        _ui->progressBar->setValue(_img_cnt);
        _ui->progressBar_label->setText(QString(PROGRESSBAR_LABEL_TEXT).arg(_img_cnt).arg(MIN_CALIB_IMAGES));
        if (_img_cnt >= MIN_CALIB_IMAGES)
          _ui->calibrate_btn->show();

        _add_next_frame = false;
      }
    }
  }

  void calibration_widget::add_next_frame() {
    _add_next_frame = true;
  }

  void calibration_widget::calibrate() {
    // Actual calibration step

    bool success;

    float accuracy;
    success = REME_SUCCESS(reme_calibrator_calibrate(_i->context(), _calibrator, &accuracy));
    status_bar_msg(QString("Reprojection error: %1 pixels \n").arg(accuracy), STATUS_MSG_TIME);

    // Generate a new sensor config using the new intrinsic values
    reme_options_t o, o_intr, o_intr_new;
    
    success = success && REME_SUCCESS(reme_options_create(_i->context(), &o));
    success = success && REME_SUCCESS(reme_options_create(_i->context(), &o_intr));
    success = success && REME_SUCCESS(reme_options_create(_i->context(), &o_intr_new));
    // Get the current camera configuration
    success = success && REME_SUCCESS(reme_sensor_bind_camera_options(_i->context(), _i->sensor(), o));
    success = success && REME_SUCCESS(reme_options_bind_message(_i->context(), o, "depth_intrinsics", o_intr));
    // Get the result of the calibration
    success = success && REME_SUCCESS(reme_calibrator_bind_intrinsics(_i->context(), _calibrator, o_intr_new));
    
    // Copy over
    success = success && REME_SUCCESS(reme_options_copy(_i->context(), o_intr_new, o_intr));

    if (success) {
      QString sensor_file_name = QFileDialog::getSaveFileName(this, "Choose Save Filename", QDir::current().absolutePath(), QString("*.txt"));

      // Save new config to file
      success = success && REME_SUCCESS(reme_options_save_to_file(_i->context(), o, sensor_file_name.toStdString().c_str()));

      //if (QMessageBox::Yes == QMessageBox::information(this, "Calibrated Sensor Configuration", "Apply the generated sensor configuration?", QMessageBox::Yes, QMessageBox::No)) {
      //  QSettings settings(profactor_tag, reme_tag, this);
      //  settings.setValue(sensor_path_tag, sensor_file_name);
      //}
    }
    else {
      status_bar_msg("Calibration failed", STATUS_MSG_TIME);
    }
  }

  void calibration_widget::apply_camera_settings() {
    const char *support;
    int length;

    reme_options_t o;
    reme_options_create(_i->context(), &o);
    reme_sensor_bind_capture_options(_i->context(), _i->sensor(), o);

    reme_options_set(_i->context(), o, "enable_ir_projector", "false");
    reme_options_set(_i->context(), o, "ir_alpha", "40");
    reme_options_set(_i->context(), o, "enable_rgb", "false");
    reme_options_set(_i->context(), o, "enable_ir", "true");

    reme_sensor_apply_capture_options(_i->context(), _i->sensor(), o);
  }

  void calibration_widget::apply_calibration_setting() {

    calibration_options co;
    co.set_inner_count_height(_ui->ich_sb->value());
    co.set_inner_count_width(_ui->icw_sb->value());
    co.set_length_square_side(_ui->lss_sp->value());
    co.set_refine_height(_ui->rh_sp->value());
    co.set_refine_width(_ui->rw_sp->value());
    co.set_zero_tangential(_ui->zt_cb->isChecked());
    co.set_zero_k3(_ui->zk_cb->isChecked());

    reme_options_t o;
    reme_options_create(_i->context(), &o);

    if (_calibrator != REME_ERROR_UNSPECIFIED)
      _i->destroy_calibrator(_calibrator);
    _calibrator = _i->new_calibrator();
    
    reme_calibrator_bind_options(_i->context(), _calibrator, o);
    
    std::string msg;
    co.SerializeToString(&msg);
    reme_options_set_bytes(_i->context(), o, msg.c_str(), msg.size());

    // Reset
    _img_cnt = 0;
    _ui->calibrate_btn->hide();
    _calib_canvas->set_image_data(0);
    _ui->progressBar->setValue(0);
    _ui->progressBar_label->setText(QString(PROGRESSBAR_LABEL_TEXT).arg(0).arg(MIN_CALIB_IMAGES));
  }

  void calibration_widget::showEvent(QShowEvent* event) {
    apply_camera_settings();
    apply_calibration_setting();
    _calibrate_image = _i->new_image();
  }

  void calibration_widget::hideEvent(QHideEvent* event) {
    if (_calibrator != REME_ERROR_UNSPECIFIED)
      _i->destroy_calibrator(_calibrator);
    if (_calibrate_image != REME_ERROR_UNSPECIFIED)
      _i->destroy_image(_calibrate_image);
  }
}