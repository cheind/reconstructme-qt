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

#include "macros.h"

#include <QObject>
#include <QString>

#include <reconstructmesdk/reme.h>

// Forward declarations
class QImage;

namespace ReconstructMeGUI {

  /** This class provides scanning utilities */
  class scan : public QObject 
  {  
    Q_OBJECT;

  public:
    scan(reme_context_t c);
    ~scan();

    /** Getter for correct sized RGB QImage */
    QImage *get_rgb_image();
    /** Getter for correct sized Phong QImage */
    QImage *get_phong_image();
    /** Getter for correct sized Depth QImage */
    QImage *get_depth_image();

  public slots:
    /** Toggles current mode */
    void toggle_play_pause();
    /** Sets the volume to empty */
    void reset_volume();
    /** Main loop. Retrieve Images, update volume */
    void run(bool);
    /** Exit run(bool) */
    void request_stop();
    
    /** Save current volume content as polygonzied 3D model to file_name */
    void save(const QString &file_name);
	  /** Create a sensor from QSettings */
    bool create_sensor();
    /** Initialized opencl utitliy with configruations from QSettings */    
    bool initialize();

  signals:
    /** Emits the success of intitialization process */
    void initialized(bool);
    /** Emits the success of the sensor creation process */
    void sensor_created(bool);

    /** Emit status information */
    void status_string(const QString &msg, const int msecs = 0);
    /** Emit log information */
    void log_message(const QString &msg);
    /** Provide information about error when license is applied */
    void licence_error_code(int);

    /** Is emitted, when a new RGB image is available */
    void new_rgb_image_bits();
    /** Is emitted, when a new Phong image is available */
    void new_phong_image_bits();
    /** Is emitted, when a new Depth image is available */
    void new_depth_image_bits();
    
  private:
    bool try_open_sensor(const char *driver);
    
    // Members
    struct data;
    NO_WARNING_DLL_INTERFACE(data*, _data);
  };
}

#endif // SCAN_H