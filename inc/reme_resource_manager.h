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
  
#ifndef REME_RESOURCE_MANAGER_H
#define REME_RESOURCE_MANAGER_H

#pragma once

#include "types.h"
#include "frame_grabber.h"

#include "opencl_info.pb.h"
#include "surface.pb.h"
#include "hardware.pb.h"

#include <ctime>

#include <QObject>
#include <QPair>
#include <QFuture>
#include <QFutureWatcher>
#include <QSharedPointer>

#include <reconstructmesdk/types.h>

// FoWrward declarations
class QImage;

namespace ReconstructMeGUI {

  /** This class provides scanning utilities */
  class reme_resource_manager : public QObject 
  {  
    Q_OBJECT;
    
  public:
    reme_resource_manager();
    ~reme_resource_manager();

  public slots:
    void initialize();

    const reme_context_t context() const;
    const reme_sensor_t sensor() const;
    const reme_volume_t volume() const;
    
    void get_version(std::string& version);
    void get_hardware_hashes(hardware& hashes);
    void get_opencl_info(opencl_info &ocl);
    void new_log_message(reme_log_severity_t sev, const QString &log);

    bool has_valid_license() const;

    reme_calibrator_t new_calibrator() const;
    void destroy_calibrator(reme_calibrator_t calib);

    reme_image_t new_image() const;
    void destroy_image(reme_image_t image);

    void set_frame_grabber(std::shared_ptr<frame_grabber> fg);
    void start_scanning();
    void stop_scanning();
    void scan();
    void reset_volume();
     
    void generate_surface(float face_decimation);

    void save(const QString &filename);

  signals:
    void surface(bool has_surface,
      const float *points, int num_points,
      const float *normals, int num_normals,
      const unsigned *faces, int num_faces);

    void initializing(init_t what);
    void initialized(init_t what, bool success);
    void sdk_initialized(bool success);
    void initializing_sdk();
    void log_message(reme_log_severity_t sev, const QString &log);

    void current_fps(const float);
   
  private:
    bool try_open_sensor(const char *driver);
    
    bool open_sensor();
    bool compile_context();
    bool apply_license();

    reme_context_t _c;
    reme_sensor_t _s;
    reme_volume_t _v;
    reme_surface_t _p;

    bool _has_compiled_context;
    bool _has_sensor;
    bool _has_volume;
    bool _has_valid_license;

    std::shared_ptr<frame_grabber> _fg;

    bool _lost_track_prev;

    clock_t _c0;
    int _cnt;
  };
}

#endif // REME_RESOURCE_MANAGER_H