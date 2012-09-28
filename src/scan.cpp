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

#include <QDebug>
#include <QCoreApplication>
#include <QSettings>
#include <QImage>

#include <sstream>
#include <iostream>

namespace ReconstructMeGUI {

  struct scan::data {

    enum Mode {PLAY, PAUSE, STOP};

    reme_context_t c;
    reme_sensor_t s;
    reme_volume_t v;

    bool has_compiled_context;
    bool has_sensor;
    bool has_volume;

    Mode mode;

    QImage* rgb_image;
    QImage* phong_image;
    QImage* depth_image;

    // initialize data
    data() {
      has_compiled_context = false;
      has_sensor = false;
      has_volume = false;
      mode = STOP;
    }
  };

  scan::scan(reme_context_t c) : _data(new data()) {
    _data->c = c;
  }

  scan::~scan() {
    if (_data->has_sensor) {
      reme_sensor_close(_data->c, _data->s);
      reme_sensor_destroy(_data->c, &_data->s);
    }
    delete _data;
  }

  bool scan::create_sensor()
  {
    if (!_data->has_compiled_context) 
      return false;
    bool success = true;
    // delete sensor, if a sensor is already in use
    if (_data->has_sensor) {
      success &= REME_SUCCESS(reme_sensor_close(_data->c, _data->s));
      success &= REME_SUCCESS(reme_sensor_destroy(_data->c, &_data->s));
    }
    _data->has_sensor = false;
    // get settings
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, profactor_tag, reme_tag);
    QString sensor_path = settings.value(sensor_path_tag).toString();
    // create a sensor from settings
    success &= REME_SUCCESS(reme_sensor_create(_data->c, sensor_path.toStdString().c_str(), true, &_data->s));
    success &= REME_SUCCESS(reme_sensor_open(_data->c, _data->s));
    
    if (success) { 
      _data->has_sensor = true;
      
      int width, height;
      
      if (REME_SUCCESS(reme_sensor_get_image_size(_data->c, _data->s, REME_IMAGE_AUX, &width, &height)))
        _data->rgb_image   = new QImage(width, height, QImage::Format_RGB888);
      else 
        _data->rgb_image   = 0;

      if (REME_SUCCESS(reme_sensor_get_image_size(_data->c, _data->s, REME_IMAGE_VOLUME, &width, &height)))
        _data->phong_image = new QImage(width, height, QImage::Format_RGB888);
      else
        _data->phong_image = 0;

      if (REME_SUCCESS(reme_sensor_get_image_size(_data->c, _data->s, REME_IMAGE_DEPTH, &width, &height)))
        _data->depth_image = new QImage(width, height, QImage::Format_RGB888);
      else 
        _data->depth_image = 0;
    }
    else {
      emit status_string(something_went_wrong_tag);
    }

    emit sensor_created(success);

    return _data->has_sensor;
  }

  bool scan::initialize() 
  {
    _data->has_compiled_context = false;
    bool success = true;

    // reload settings
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, profactor_tag, reme_tag);

    // Set licence
    QString licence_file = settings.value(license_file_tag, license_file_default_tag).toString();
    if (licence_file != license_file_default_tag) {
      reme_error_t error = reme_context_set_license(_data->c, licence_file.toStdString().c_str());
      if (error == REME_ERROR_INVALID_LICENSE) {
        emit log_message(invalid_license_tag);
      }
      else if (error == REME_ERROR_UNSPECIFIED) {
        emit log_message(license_unspecified_tag);
      }
      else {
        emit log_message(license_applied_tag);
        emit status_string(license_applied_tag, STATUS_MSG_DURATION);
      }
      emit licence_error_code(error);
    }

    // Create empty options binding
    reme_options_t o;
    success &= REME_SUCCESS(reme_options_create(_data->c, &o));

    // Bind to compile time options
    success &= REME_SUCCESS(reme_context_bind_compile_options(_data->c, o));

    // load options if config_path already set
    std::string path = settings.value(config_path_tag, config_path_default_tag).toString().toStdString();
    if (path != config_path_default_tag) {
      success &= REME_SUCCESS(reme_options_load_from_file(_data->c, o, path.c_str()));
    }

    // Modify the 'device_id' field through a reflection like syntax
    int device_id = settings.value(opencl_device_tag, opencl_device_default_tag).toInt();
    std::stringstream str_stream;
    str_stream << device_id;
    success &= REME_SUCCESS(reme_options_set(_data->c, o, devcice_id_tag, str_stream.str().c_str()));

    // Compile for OpenCL device using modified options
    success &= REME_SUCCESS(reme_context_compile(_data->c));
    if (success) {
      _data->has_compiled_context = true;
      if (!_data->has_volume) {
        success &= REME_SUCCESS(reme_volume_create(_data->c, &_data->v));
        _data->has_volume = true;
      }
    }
    else
      emit status_string(something_went_wrong_tag);

    emit initialized(success);
    return success;
  }

  void scan::run(bool unused) {
    
    // check requirements for run!
    if ((!_data->has_compiled_context && !_data->has_sensor) ||
        _data->rgb_image   == 0 ||
        _data->phong_image == 0 ||
        _data->depth_image == 0 ||
        _data->mode != data::STOP) 
      return;

    _data->mode = data::PAUSE;
    // Get ready
    bool success = true;
    bool lost_track_prev = false;
    const void *image_bytes;

    while (_data->mode != data::STOP && success) {
      success = true;
      // Prepare image and depth data
      success &= REME_SUCCESS(reme_sensor_grab(_data->c, _data->s));
      success &= REME_SUCCESS(reme_sensor_retrieve(_data->c, _data->s));

      success &= REME_SUCCESS(reme_sensor_get_image(_data->c, _data->s, REME_IMAGE_AUX, &image_bytes));
      if (success) {
        memcpy((void*)_data->rgb_image->bits(), image_bytes, _data->rgb_image->byteCount());
        emit new_rgb_image_bits();
      }
      success &= REME_SUCCESS(reme_sensor_get_image(_data->c, _data->s, REME_IMAGE_VOLUME, &image_bytes));
      if (success) {
        memcpy((void*)_data->phong_image->bits(), image_bytes, _data->phong_image->byteCount());
        emit new_phong_image_bits();
      }
      success &= REME_SUCCESS(reme_sensor_get_image(_data->c, _data->s, REME_IMAGE_DEPTH, &image_bytes));
      if (success) {
        memcpy((void*)_data->depth_image->bits(), image_bytes, _data->depth_image->byteCount());
        emit new_depth_image_bits();
      }
      
      if (_data->mode != data::PLAY) {
        QCoreApplication::instance()->processEvents(); // check if something changed
        continue;
      }

      reme_error_t track_error = reme_sensor_track_position(_data->c, _data->s);
      if (REME_SUCCESS(track_error)) {
        // Track camera success (engine step)
        if (lost_track_prev) {
          // track found
          lost_track_prev = false;
          emit status_string(camera_track_found_tag, STATUS_MSG_DURATION);
        }
        // Update volume with depth data from the current sensor perspective
        success &= REME_SUCCESS(reme_sensor_update_volume(_data->c, _data->s));
      }
      else if (track_error == REME_ERROR_INVALID_LICENSE) {
        // lost track
        if (!lost_track_prev)
          emit status_string(camera_track_lost_license_tag);
        lost_track_prev = true;
      }
      else if (!lost_track_prev) {
        // track lost
        lost_track_prev = true;
        emit status_string(camera_track_lost_tag);
      }
      
      if (!success)
        emit status_string(something_went_wrong_tag);
      
      QCoreApplication::instance()->processEvents(); // this has to be the last command in run
    } // while

    _data->mode = data::STOP;
  }

  void scan::reset_volume()
  {
    if (_data->has_volume)
      reme_volume_reset(_data->c, _data->v);

    if (_data->has_sensor)
      reme_sensor_reset(_data->c, _data->s);
  }

  void scan::save(const QString &file_name) {
    if (!_data->has_compiled_context) return;

    // Create a new surface
    reme_surface_t m;
    bool success = true;
    success &= REME_SUCCESS(reme_surface_create(_data->c, &m));
    success &= REME_SUCCESS(reme_surface_generate(_data->c, m, _data->v));
    success &= REME_SUCCESS(reme_surface_save_to_file(_data->c, m, file_name.toStdString().c_str()));

    QString msg;
    if (!success)
      msg = something_went_wrong_tag;
    else 
      msg = saved_file_to_tag + file_name;
    
    emit status_string(msg);
  }

  void scan::toggle_play_pause() {
    _data->mode = (_data->mode == data::PLAY) ? data::PAUSE : data::PLAY; // toggle mode
  }

  void scan::new_log(const char* msg) 
  {
    QString log (msg);
    emit log_message(log);
  }

  void scan::request_stop() {
    _data->mode = data::STOP;
  }

  QImage *scan::get_rgb_image() {
    return _data->rgb_image;
  }

  QImage *scan::get_phong_image() {
    return _data->phong_image;
  }

  QImage *scan::get_depth_image() {
    return _data->depth_image;
  }
}