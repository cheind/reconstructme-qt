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
#include "reme_resource_manager.h"

#include <QCoreApplication>
#include <QSettings>
#include <qthread.h>
#include <QImage>

#include <reconstructmesdk/reme.h>

#include <sstream>
#include <iostream>

namespace ReconstructMeGUI {

  scan::scan(std::shared_ptr<reme_resource_manager> initializer) : 
    _i(initializer),
    _play_integration_time(-1)
  {
    _mode = PAUSE;
    lost_track_prev = false;
  }

  scan::~scan() {
    reme_options_destroy(_i->context(), &_o);
  }

  void scan::initialize(bool do_init) {
    if (!do_init) return;

    reme_options_create(_i->context(), &_o);
    reme_context_bind_reconstruction_options(_i->context(), _o);
  }

  void scan::process_frame() {
    if (_mode == PAUSE) return;

    // Set tracking mode to global when mode switches to play
    reme_volume_get_time(_i->context(), _i->volume(), &_current_integration_time);
    if (_play_integration_time == _current_integration_time) 
      reme_options_set(_i->context(), _o, "camera_tracking.search_mode", "GLOBAL");
    else if (_current_integration_time == _play_integration_time+1) 
      reme_options_set(_i->context(), _o, "camera_tracking.search_mode", "AUTO");

    bool success = true;

    cnt++;
    if (cnt % FPS_MODULO == 0) {
      emit current_fps((float)cnt/(((float)(clock()-c0))/CLOCKS_PER_SEC));
      c0 = clock();
      cnt = 0;
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
  }

  void scan::reset_volume() {
    bool success = true;
    success = success && REME_SUCCESS(reme_volume_reset(_i->context(), _i->volume()));
    success = success && REME_SUCCESS(reme_sensor_reset(_i->context(), _i->sensor()));
  }

  void scan::toggle_play_pause() {
    _mode = (_mode == PLAY) ? PAUSE : PLAY; // toggle mode

    // Remember times of integration for switching tracking mode to global
    if (_mode == PLAY)
      reme_volume_get_time(_i->context(), _i->sensor(), &_play_integration_time);

    int cnt = 0;
    clock_t c0 = clock();
    
    emit current_fps(0);
    emit mode_changed(_mode);
  }

  void scan::set_mode(mode_t mode) {
    _mode = mode; // toggle mode

    int cnt = 0;
    clock_t c0 = clock();
    
    emit current_fps(0);
    emit mode_changed(_mode);
  }
}