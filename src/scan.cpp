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
#define FPS_MODULO 10

#include "scan.h"
#include "settings.h"
#include "strings.h"
#include "reme_sdk_initializer.h"
#include "mutex.h"

#include <QCoreApplication>
#include <QSettings>
#include <qthread.h>
#include <QImage>

#include <reconstructmesdk/reme.h>

#include <sstream>
#include <iostream>
#include <ctime>

namespace ReconstructMeGUI {

  scan::scan(std::shared_ptr<reme_sdk_initializer> initializer) :
    _i(initializer)
  {
    _mode = NOT_RUN;
    
    connect(_i.get(), SIGNAL(sdk_initialized(bool)), SLOT(start(bool)));
    connect(_i.get(), SIGNAL(initializing_sdk()), SLOT(stop()), Qt::BlockingQueuedConnection);
  }

  scan::~scan() {
  }

  void scan::start(bool do_start) {
    
    // avoid recursion!
    if (!do_start || _mode != NOT_RUN) return;
    
    // Get ready
    _mode = PAUSE;
    bool success = true;
    bool lost_track_prev;
    const void *image_bytes;
    int length;
    int cnt = 1;
    clock_t c0 = clock();
    success = success && REME_SUCCESS(reme_sensor_update_image(_i->context(), _i->sensor(), REME_IMAGE_VOLUME, _i->volume())); 
    success = success && REME_SUCCESS(reme_image_get_bytes(_i->context(), _i->volume(), &image_bytes, &length));
    if (success) {
      QMutexLocker lock(&image_mutex);
      memcpy((void*)_phong_image->bits(), image_bytes, _phong_image->byteCount());
      emit new_phong_image_bits();
    }

    while (_mode != NOT_RUN && success) {
      // frames per second
      cnt++;
      if (cnt % FPS_MODULO == 0) {
        emit current_fps((float)cnt/(((float)(clock()-c0))/CLOCKS_PER_SEC));
        c0 = clock();
        cnt = 0;
      }

      success = true;

      // Prepare image and depth data
      success = success && REME_SUCCESS(reme_sensor_grab(_i->context(), _i->sensor()));
      //reme_sensor_s
      success = success && REME_SUCCESS(reme_sensor_prepare_images(_i->context(), _i->sensor()));

      success = success && REME_SUCCESS(reme_sensor_update_image(_i->context(), _i->sensor(), REME_IMAGE_AUX, _i->rgb()));
      if (_i->rgb_size() != 0 && success && REME_SUCCESS(reme_image_get_bytes(_i->context(), _i->rgb(), &image_bytes, &length))) {
        QMutexLocker lock(&image_mutex);
        memcpy((void*)_rgb_image->bits(), image_bytes, _rgb_image->byteCount());
        emit new_rgb_image_bits();
      }

      success = success && REME_SUCCESS(reme_sensor_update_image(_i->context(), _i->sensor(), REME_IMAGE_DEPTH, _i->depth()));
      if (_i->depth_size() != 0 && success && REME_SUCCESS(reme_image_get_bytes(_i->context(), _i->depth(), &image_bytes, &length))) {
        QMutexLocker lock(&image_mutex);
        memcpy((void*)_depth_image->bits(), image_bytes, _depth_image->byteCount());
        emit new_depth_image_bits();
      }
      
      if (_mode != PLAY) {
        QCoreApplication::instance()->processEvents(); // check if something changed
        continue;
      }

      success = success && REME_SUCCESS(reme_sensor_update_image(_i->context(), _i->sensor(), REME_IMAGE_VOLUME, _i->volume()));
      if (_i->depth_size() != 0 && success && REME_SUCCESS(reme_image_get_bytes(_i->context(), _i->volume(), &image_bytes, &length))) {
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
          emit status_bar_msg(camera_track_found_tag, STATUS_MSG_DURATION);
        }
        // Update volume with depth data from the current sensor perspective
        success = success && REME_SUCCESS(reme_sensor_update_volume(_i->context(), _i->sensor()));
      }
      else if (track_error == REME_ERROR_INVALID_LICENSE) {
        // lost track
        if (!lost_track_prev) {
          emit status_bar_msg(camera_track_lost_license_tag);
        }
        lost_track_prev = true;
      }
      else if (!lost_track_prev) {
        // track lost
        lost_track_prev = true;
        emit status_bar_msg(camera_track_lost_tag);
      }
      
      if (!success) {
        emit status_bar_msg(something_went_wrong_tag, STATUS_MSG_DURATION);
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
    int length;

    success = success && REME_SUCCESS(reme_sensor_update_image(_i->context(), _i->sensor(), REME_IMAGE_VOLUME, _i->volume()));
    success = success && REME_SUCCESS(reme_image_get_bytes(_i->context(), _i->volume(), &image_bytes, &length));
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

    // Transform the mesh from world space to CAD space, so external viewers
    // can cope better with the result.
    float mat[16];
    success = success && REME_SUCCESS(reme_transform_set_predefined(_i->context(), REME_TRANSFORM_WORLD_TO_CAD, mat));
    success = success && REME_SUCCESS(reme_surface_transform(_i->context(), m, mat));

    success = success && REME_SUCCESS(reme_surface_save_to_file(_i->context(), m, file_name.toStdString().c_str()));
  }

  void scan::toggle_play_pause() {
    if (_mode == NOT_RUN) return;

    _mode = (_mode == PLAY) ? PAUSE : PLAY; // toggle mode

    emit current_fps(0);
    emit mode_changed(_mode);
  }

  void scan::stop() {
    emit current_fps(0);
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