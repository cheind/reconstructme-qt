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

#include "reme_sdk_initializer.h"
#include "settings.h"
#include "strings.h"
#include "mutex.h"

#include <QDebug>
#include <QCoreApplication>
#include <QSettings>
#include <QImage>
#include <QtConcurrentRun>

#include <reconstructmesdk/reme.h>

#include <sstream>
#include <iostream>

namespace ReconstructMeGUI {

  reme_sdk_initializer::reme_sdk_initializer(reme_context_t c) {
    _c = c;
    _has_compiled_context = false;
    _has_sensor = false;
    _has_volume = false;
  }

  reme_sdk_initializer::~reme_sdk_initializer() {
    if (_has_sensor) {
      reme_sensor_close(_c, _s);
      reme_sensor_destroy(_c, &_s);
    }
    if (_has_volume) {
      reme_volume_destroy(_c, &_v);
    }
  }

  void reme_sdk_initializer::_initialize() {
    bool success = false;

    emit initializing_sdk();

    emit initializing(LICENSE);
    success = apply_license();
    emit initialized(LICENSE, success);
    
    emit initializing(OPENCL);
    success = compile_context();
    emit initialized(OPENCL, success);

    emit initializing(SENSOR);
    success = open_sensor();
    emit initialized(SENSOR, success);
    
    if (_has_compiled_context && _has_sensor && _has_volume) {
      emit sdk_initialized();
    }
  }

  void _init(reme_sdk_initializer *initializer) {
    return initializer->_initialize();
  }

  void reme_sdk_initializer::initialize() {
    QtConcurrent::run(_init, this);
  }

  bool reme_sdk_initializer::open_sensor() {
    bool success = true;
    
    // can not initialize sensor is no compiled context is available
    if (!_has_compiled_context) success = false;

    // destroy sensor object, if a sensor is already in use
    if (_has_sensor) {
      success = success && REME_SUCCESS(reme_sensor_close(_c, _s));
      success = success && REME_SUCCESS(reme_sensor_destroy(_c, &_s));
    }

    // create and open a sensor from settings
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, profactor_tag, reme_tag);
    QString sensor_path = settings.value(sensor_path_tag).toString();
    success = success && REME_SUCCESS(reme_sensor_create(_c, sensor_path.toStdString().c_str(), true, &_s));
    success = success && REME_SUCCESS(reme_sensor_open(_c, _s));
    
    if (success)
    {
      int width, height;
      reme_error_t error;
      
      error = reme_sensor_get_image_size(_c, _s, REME_IMAGE_AUX, &width, &height);
      _rgb_size = REME_SUCCESS(error) ? new QSize(width, height) : 0;
 
      error = reme_sensor_get_image_size(_c, _s, REME_IMAGE_VOLUME, &width, &height);
      _phong_size = REME_SUCCESS(error) ? new QSize(width, height) : 0;
 
      error = reme_sensor_get_image_size(_c, _s, REME_IMAGE_DEPTH, &width, &height);
      _depth_size = REME_SUCCESS(error) ? new QSize(width, height) : 0;

      emit initialized_images();
    }

    _has_sensor = success;
    return _has_sensor;
  }

  bool reme_sdk_initializer::apply_license() {
    bool success = false;

    reme_license_t l;
    reme_license_create(_c, &l);
    
    // Set licence
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, profactor_tag, reme_tag);
    QString licence_file = settings.value(license_file_tag, license_file_default_tag).toString();
    if (licence_file != license_file_default_tag) {      
      reme_error_t error = reme_license_authenticate(_c, l, licence_file.toStdString().c_str());
      if (error == REME_ERROR_INVALID_LICENSE)
        success = false;
      else if (error == REME_ERROR_UNSPECIFIED) 
        success = false;
    }
    
    return success;
  }

  bool reme_sdk_initializer::compile_context()
  {
    bool success = true;

    // reload settings
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, profactor_tag, reme_tag);

    // Create empty options binding
    reme_options_t o;
    success = success && REME_SUCCESS(reme_options_create(_c, &o));
    success = success && REME_SUCCESS(reme_context_bind_compile_options(_c, o));

    // load options if config_path already set
    std::string path = settings.value(config_path_tag, config_path_default_tag).toString().toStdString();
    if (path != config_path_default_tag) {
      success = success && REME_SUCCESS(reme_options_load_from_file(_c, o, path.c_str()));
    }

    // apply selected opencl_device
    int device_id = settings.value(opencl_device_tag, opencl_device_default_tag).toInt();
    std::stringstream str_stream;
    str_stream << device_id;
    success = success && REME_SUCCESS(reme_options_set(_c, o, devcice_id_tag, str_stream.str().c_str()));

    // Compile for OpenCL device using modified options
    success = success && REME_SUCCESS(reme_context_compile(_c));
    if (!_has_volume) {
      success = success && REME_SUCCESS(reme_volume_create(_c, &_v));
      _has_volume = true;
    }
    
    _has_compiled_context = success;
    return _has_compiled_context;
  }

  const reme_context_t reme_sdk_initializer::context() const{
    return _c;
  }

  const reme_sensor_t reme_sdk_initializer::sensor() const{
    return _s;
  }

  const reme_volume_t reme_sdk_initializer::volume() const{
    return _v;
  }

  const QSize *reme_sdk_initializer::rgb_size() const {
    return _rgb_size;
  }

  const QSize *reme_sdk_initializer::phong_size() const {
    return _phong_size;
  }

  const QSize *reme_sdk_initializer::depth_size() const {
    return _depth_size;
  }

  QSize *reme_sdk_initializer::rgb_size() {
    return _rgb_size;
  }

  QSize *reme_sdk_initializer::phong_size() {
    return _phong_size;
  }

  QSize *reme_sdk_initializer::depth_size() {
    return _depth_size;
  }
}