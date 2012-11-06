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
  
#ifndef SCANWIDGET_H
#define SCANWIDGET_H

#pragma once

#include <QWidget>
#include <QMap>

#include "types.h"

#include <reconstructmesdk/types.h>

// Forward declarations
class QImage;
class QThread;
class QLabel;
class QFileDialog;
namespace Ui {
  class scan_widget;
}
namespace ReconstructMeGUI {
  class scan;
  class QGLCanvas;
  class reme_resource_manager;
}

namespace ReconstructMeGUI {

  /** Main UI element of the ReconstructMeQT application 
  *
  * \note This is the topmost element of the UI-tree. It holds references to 
  *       all other UI elements, such as the necessary dialogs.
  */
  class scan_widget : public QWidget
  {
    Q_OBJECT
    
  public:
    scan_widget(std::shared_ptr<reme_resource_manager>, QWidget *parent = 0);
    ~scan_widget();

    const scan *scanner() const;
    virtual void showEvent(QShowEvent* event);
    virtual void hideEvent(QHideEvent* event);

  public slots:
    void process_frame(reme_sensor_image_t, reme_image_t);

    void reconstruct();

  private slots:
    /** Handle save button clicked event. Trigger scanner to save current mesh */
    void save_button_clicked();
    /** Handle reset button clicked event. Trigger scanner to reset current volume */
    void reset_button_clicked();
    /** Handle play button clicked event. Trigger scanner to toggle PLAY/PAUSE */
    void apply_mode(mode_t mode);

  signals:
    /** Trigger scanner to save current mesh */
    void save_mesh_to_file(const QString &s);
    /** This signal is emited when this objects constructor finished */
    void initialize();
    /** Provide status information */
    void status_bar_msg(const QString &msg, const int msecs = 0);

  private:
    Ui::scan_widget *_ui;

    // Scanner
    scan *_scanner;
    
    // Images & Widget
    QMap<reme_sensor_image_t, QGLCanvas*> _canvas_map; 
    std::shared_ptr<reme_resource_manager> _i;
  };

}

#endif // SCANWIDGET_H
