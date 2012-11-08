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

#ifndef CALIBRATION_WIDGET_H
#define CALIBRATION_WIDGET_H

#include "types.h"

#include <QDialog>

#include <reconstructmesdk/types.h>

// Forward declarations
class QThread;
namespace Ui {
  class calibration_widget;
}
namespace ReconstructMeGUI {
  class reme_resource_manager;
  class QGLCanvas;
  class calibrate;
}

namespace ReconstructMeGUI {

  /** This is dialog provides status information of the scanner*/
  class calibration_widget : public QWidget
  {
    Q_OBJECT;

  public:
    calibration_widget(std::shared_ptr<reme_resource_manager> initializer, QWidget *parent = 0);
    ~calibration_widget();

    virtual void showEvent(QShowEvent* event);
    virtual void hideEvent(QHideEvent* event);

  public slots:
    void process_frame(reme_sensor_image_t type, reme_image_t img);
    void add_next_frame();
    void calibrate();
    void apply_calibration_setting();
    void apply_camera_settings();

  signals:
    void status_bar_msg(const QString &msg, const int msecs = 0);
    void new_setting_file(const QString &file_path, init_t type);

  private:

    Ui::calibration_widget *_ui;
    std::shared_ptr<reme_resource_manager> _i;

    reme_image_t _calibrate_image;
    reme_calibrator_t _calibrator;
    reme_options_t _cap_o;

    bool _applied_new_sensor_config;

    bool _add_next_frame;
    int _img_cnt;

    QGLCanvas* _aux_canvas;
    QGLCanvas* _calib_canvas;
  };
} 

#endif // CALIBRATION_WIDGET_H