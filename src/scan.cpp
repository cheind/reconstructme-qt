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
  
#pragma once

#define STATUS_MSG_DURATION 2000

#include "scan.h"
#include "settings.h"
#include "strings.h"
#include "reme_sdk_initializer.h"
#include "mutex.h"

#include <QCoreApplication>
#include <QSettings>
#include <QImage>

#include <reconstructmesdk/reme.h>

#include <sstream>
#include <iostream>

namespace ReconstructMeGUI {

  scan::scan(reme_sdk_initializer *initializer) {
    _mode = NOT_RUN;
    _i = initializer;
    
    connect(_i, SIGNAL(sdk_initialized()), SLOT(start()));
    connect(_i, SIGNAL(initializing_sdk()), SLOT(stop()));
  }

  scan::~scan() {
  }

  void scan::start() {
    // no recursion!
    if (_mode != NOT_RUN) return;
    
    // Get ready
    _mode = PAUSE;
    bool success = true;
    bool lost_track_prev = false;
    const void *image_bytes;

    while (_mode != NOT_RUN && success) {
      success = true;

      // Prepare image and depth data
      success = success && REME_SUCCESS(reme_sensor_grab(_i->context(), _i->sensor()));
      if (_mode == PLAY)
        success = success && REME_SUCCESS(reme_sensor_prepare_images(_i->context(), _i->sensor()));
      else if(_mode == PAUSE) {
        success = success && REME_SUCCESS(reme_sensor_prepare_image(_i->context(), _i->sensor(), REME_IMAGE_AUX));
        success = success && REME_SUCCESS(reme_sensor_prepare_image(_i->context(), _i->sensor(), REME_IMAGE_DEPTH));
      }

      success = success && REME_SUCCESS(reme_sensor_get_image(_i->context(), _i->sensor(), REME_IMAGE_AUX, &image_bytes));
      if (success) {
        QMutexLocker lock(&image_mutex);
        memcpy((void*)_rgb_image->bits(), image_bytes, _rgb_image->byteCount());
        emit new_rgb_image_bits();
      }
      success = success && REME_SUCCESS(reme_sensor_get_image(_i->context(), _i->sensor(), REME_IMAGE_DEPTH, &image_bytes));
      if (success) {
        QMutexLocker lock(&image_mutex);
        memcpy((void*)_depth_image->bits(), image_bytes, _depth_image->byteCount());
        emit new_depth_image_bits();
      }
      
      if (_mode != PLAY) {
        QCoreApplication::instance()->processEvents(); // check if something changed
        continue;
      }

      success = success && REME_SUCCESS(reme_sensor_get_image(_i->context(), _i->sensor(), REME_IMAGE_VOLUME, &image_bytes));
      if (success) {
        QMutexLocker lock(&image_mutex);
        memcpy((void*)_phong_image->bits(), image_bytes, _phong_image->byteCount());
        emit new_phong_image_bits();
      }
      

      reme_error_t track_error = reme_sensor_track_position(_i->context(), _i->sensor());
      if (REME_SUCCESS(track_error)) {
        // Track camera success (engine step)
        if (lost_track_prev) {
          // track found
          lost_track_prev = false;
          // camera_track_found_tag
        }
        // Update volume with depth data from the current sensor perspective
        success = success && REME_SUCCESS(reme_sensor_update_volume(_i->context(), _i->sensor()));
      }
      else if (track_error == REME_ERROR_INVALID_LICENSE) {
        // lost track
        if (!lost_track_prev) {
          // camera_track_lost_license_tag
        }
        lost_track_prev = true;
      }
      else if (!lost_track_prev) {
        // track lost
        lost_track_prev = true;
        // camera_track_lost_tag
      }
      
      if (!success) {
        //something_went_wrong_tag
      }
      
      QCoreApplication::instance()->processEvents(); // this has to be the last command in run
    } // while

    _mode = NOT_RUN;
  }

  void scan::reset_volume() {
    if (_mode == NOT_RUN) return;

    bool success = true;
    success = success && REME_SUCCESS(reme_volume_reset(_i->context(), _i->volume()));
    success = success && REME_SUCCESS(reme_sensor_reset(_i->context(), _i->sensor()));
    
    const void* image_bytes;
    success = success && REME_SUCCESS(reme_sensor_get_image(_i->context(), _i->sensor(), REME_IMAGE_VOLUME, &image_bytes));
    if (success) {
      QMutexLocker lock(&image_mutex);
      memcpy((void*)_phong_image->bits(), image_bytes, _phong_image->byteCount());
      emit new_phong_image_bits();
    }
  }

  void scan::save(const QString &file_name) {
    if (_mode == NOT_RUN) return;

    // Create a new surface
    reme_surface_t m;
    bool success = true;
    success = success && REME_SUCCESS(reme_surface_create(_i->context(), &m));
    success = success && REME_SUCCESS(reme_surface_generate(_i->context(), m, _i->volume()));
    success = success && REME_SUCCESS(reme_surface_save_to_file(_i->context(), m, file_name.toStdString().c_str()));
  }

  void scan::toggle_play_pause() {
    if (_mode == NOT_RUN) return;

    _mode = (_mode == PLAY) ? PAUSE : PLAY; // toggle mode

    emit mode_changed(_mode);
  }

  void scan::stop() {
    _mode = NOT_RUN;
  }

  void scan::set_rgb_image(QImage* rgb) {
    _rgb_image = rgb;
  }

  void scan::set_phong_image(QImage* phong) {
    _phong_image = phong;
  }

  void scan::set_depth_image(QImage* depth) {
    _depth_image = depth;
  }

}