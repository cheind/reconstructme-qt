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
  
#ifndef SCAN_H
#define SCAN_H

#pragma once

#include "types.h"

#include <QObject>

#include <reconstructmesdk/types.h>

// Forward declarations
class QImage;
namespace ReconstructMeGUI {
  class reme_sdk_initializer;
}

namespace ReconstructMeGUI {

  /** This class provides scanning utilities */
  class scan : public QObject 
  {  
    Q_OBJECT;
    
  public:
    scan(reme_sdk_initializer *initializer);
    ~scan();

    void set_rgb_image(QImage* rgb);
    void set_phong_image(QImage* phong);
    void set_depth_image(QImage* depth);

  public slots:
    /** Main loop. Retrieve Images, update volume */
    void start(bool);
    /** Exit run(bool) */
    void stop();
    /** Toggles current mode */
    void toggle_play_pause();
    /** Sets the volume to empty */
    void reset_volume();
    
    /** Save current volume content as polygonzied 3D model to file_name */
    void save(const QString &file_name);

  signals:
    /** Emit log information */
    void log_message(reme_log_severity_t sev, const QString &msg);

    /** Provide status information */
    void status_bar_msg(const QString &msg, const int msecs = 0);

    /** Is emitted, when a new RGB image is available */
    void new_rgb_image_bits();
    /** Is emitted, when a new Phong image is available */
    void new_phong_image_bits();
    /** Is emitted, when a new Depth image is available */
    void new_depth_image_bits();
    
    void mode_changed(mode_t);

  private:
    void initialize_images();

    QImage* _rgb_image;
    QImage* _phong_image;
    QImage* _depth_image;

    const reme_sdk_initializer *_i;
    
    mode_t _mode;
  };
}

#endif // SCAN_H