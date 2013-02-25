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
#define FPS_MODULO 15

#include "reme_resource_manager.h"
#include "settings.h"
#include "strings.h"

#include <QDebug>
#include <QCoreApplication>
#include <QSettings>
#include <QImage>
#include <QtConcurrentRun>

#include <reconstructmesdk/reme.h>

#include <sstream>
#include <iostream>

namespace ReconstructMeGUI {

  void reme_log(reme_log_severity_t sev, const char *message, void *user_data)  {
    reme_resource_manager *rm = static_cast<reme_resource_manager*>(user_data);
    rm->new_log_message(sev, QString(message));
  }

  reme_resource_manager::reme_resource_manager() : 
    _has_valid_license(false),
    _c(0)
  {
  }

  reme_resource_manager::~reme_resource_manager() {
    if (_c != 0)
      reme_context_destroy(&_c);
  }

  void reme_resource_manager::new_log_message(reme_log_severity_t sev, const QString &log) {
    emit log_message(sev, log);
  }

  void reme_resource_manager::initialize() {
    bool success = false;

    _has_sensor = false;
    _has_compiled_context = false;
    _has_volume = false;

    emit initializing_sdk();

    if (_c != 0)
      reme_context_destroy(&_c);

    reme_context_create(&_c);
    reme_context_set_log_callback(_c, reme_log, this);
    
    emit initializing(LICENSE);
    success = apply_license();
    emit initialized(LICENSE, success);
    
    emit initializing(OPENCL);
    success = compile_context();
    emit initialized(OPENCL, success);
    
    emit initializing(SENSOR);
    success = open_sensor();
    emit initialized(SENSOR, success);
    
    success = _has_compiled_context && _has_sensor && _has_volume;

    if (success) {
      reme_surface_create(_c, &_p);
    }
    emit sdk_initialized(success);
  }

  bool reme_resource_manager::open_sensor() {
    bool success = true;
    
    // can not initialize sensor is no compiled context is available
    if (!_has_compiled_context) success = false;

    // create and open a sensor from settings
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, profactor_tag, reme_tag);
    QString sensor_path = settings.value(sensor_path_tag, sensor_path_default_tag).toString();
    success = success && REME_SUCCESS(reme_sensor_create(_c, sensor_path.toStdString().c_str(), true, &_s));
    success = success && REME_SUCCESS(reme_sensor_open(_c, _s));
   
    if (success)
    {
      int w, h;
      bool supports_depth, supports_aux;

      reme_options_t o;
      reme_options_create(_c, &o);
      reme_sensor_bind_capture_options(_c, _s, o);

      // AUXILIARY image
      reme_options_get_bool(_c, o, "frame_info.supports_aux", &supports_aux);
      if (supports_aux) {
        reme_options_get_int(_c, o, "frame_info.aux_size.width", &w);
        reme_options_get_int(_c, o, "frame_info.aux_size.height", &h);
      }
      
      // DEPTH image
      reme_options_get_bool(_c, o, "frame_info.supports_depth", &supports_depth);
      if (supports_depth) {
        reme_options_get_int(_c, o, "frame_info.depth_size.width", &w);
        reme_options_get_int(_c, o, "frame_info.depth_size.height", &h);
      }
    }

    _has_sensor = success;
    return _has_sensor;
  }

  bool reme_resource_manager::apply_license() {
    bool success;

    reme_license_t l;
    success = REME_SUCCESS(reme_license_create(_c, &l));
    
    // Set licence
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, profactor_tag, reme_tag);
    QString licence_file = settings.value(license_file_tag, license_file_default_tag).toString();    
    reme_error_t error = reme_license_authenticate(_c, l, licence_file.toStdString().c_str());
    if (error == REME_ERROR_INVALID_LICENSE)
      success = false;
    else if (error == REME_ERROR_UNSPECIFIED) 
      success = false;
    
    _has_valid_license = success;

    return success;
  }

  bool reme_resource_manager::compile_context()
  {
    bool success = true;

    // reload settings
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, profactor_tag, reme_tag);

    // Create empty options binding
    reme_options_t o;
    success = success && REME_SUCCESS(reme_options_create(_c, &o));

    success = success && REME_SUCCESS(reme_context_bind_reconstruction_options(_c, o));

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

  void reme_resource_manager::set_frame_grabber(std::shared_ptr<frame_grabber> fg) {
    _fg = std::shared_ptr<frame_grabber>(fg);
  }

  void reme_resource_manager::start_scanning() {
    reme_sensor_set_trackhint(_c, _s, REME_SENSOR_TRACKHINT_USE_GLOBAL);

    _fg->request(REME_IMAGE_DEPTH);
    connect(_fg.get(), SIGNAL(frames_updated()), SLOT(scan()));

    _lost_track_prev = true;
    _cnt = 0;
    _c0 = clock();
  }

  void reme_resource_manager::stop_scanning() {
    _fg->release(REME_IMAGE_DEPTH);
    disconnect(_fg.get(), SIGNAL(frames_updated()), this, SLOT(scan()));
    current_fps(0);
  }

  void reme_resource_manager::scan() {
    bool success = true;

    _cnt++;
    if (_cnt % FPS_MODULO == 0) {
      emit current_fps((float)_cnt/(((float)(clock()-_c0))/CLOCKS_PER_SEC));
      _c0 = clock();
      _cnt = 0;
    }

    reme_error_t track_error = reme_sensor_track_position(_c, _s);
    if (REME_SUCCESS(track_error)) {
      // Track camera success (engine step)
      if (_lost_track_prev) {
        // track found
        _lost_track_prev = false;
      }
      // Update volume with depth data from the current sensor perspective
      success = success && REME_SUCCESS(reme_sensor_update_volume(_c, _s));
    }
    else if (!_lost_track_prev) {
      // track lost
      _lost_track_prev = true;
    }
  }

  void reme_resource_manager::generate_surface(float face_decimation)
  {
    std::string msg;
    
    // options
    reme_options_t o;
    reme_options_create(_c, &o);

    reme_context_bind_reconstruction_options(_c, o);    
    
    // volume information
    int res_x, res_y, res_z;
    int min_x, min_y, min_z;
    int max_x, max_y, max_z;
    reme_options_get_int(_c, o, "volume.resolution.x", &res_x);
    reme_options_get_int(_c, o, "volume.resolution.y", &res_y);
    reme_options_get_int(_c, o, "volume.resolution.z", &res_z);
    reme_options_get_int(_c, o, "volume.minimum_corner.x", &min_x);
    reme_options_get_int(_c, o, "volume.minimum_corner.y", &min_y);
    reme_options_get_int(_c, o, "volume.minimum_corner.z", &min_z);
    reme_options_get_int(_c, o, "volume.maximum_corner.x", &max_x);
    reme_options_get_int(_c, o, "volume.maximum_corner.y", &max_y);
    reme_options_get_int(_c, o, "volume.maximum_corner.z", &max_z);
    
    float vox_x, vox_y, vox_z;
    vox_x = std::abs(max_x - min_x) / (float)res_x;
    vox_y = std::abs(max_y - min_y) / (float)res_y;
    vox_z = std::abs(max_z - min_z) / (float)res_z;

    float min_vox = std::min(vox_x, std::min(vox_y, vox_z));
    float merge_radius = min_vox * 0.1f;

    // generation options
    generation_options go;
    go.set_merge_duplicate_vertices(true);
    go.set_merge_radius(merge_radius);
    go.SerializeToString(&msg);
    reme_surface_bind_generation_options(_c, _p, o);
    reme_options_set_bytes(_c, o, msg.c_str(), msg.size());

    const unsigned *faces;
    const float *points, *normals;
    int num_point_coordinates, num_normals_coordinates, num_triangle_indices;
    int num_points, num_normals, num_faces;
    
    bool has_surface = REME_SUCCESS(reme_surface_generate(_c, _p, _v));

    if(has_surface)
    {
      // decimation options
      if (0.f < face_decimation && face_decimation < 1.f) 
      {
        reme_surface_get_triangles(_c, _p, &faces, &num_triangle_indices);
        num_faces = num_triangle_indices / 3;
        
        decimation_options deco;
        deco.set_maximum_faces(num_faces * face_decimation);
        deco.SerializeToString(&msg);
        
        reme_surface_bind_decimation_options(_c, _p, o);
        reme_options_set_bytes(_c, o, msg.c_str(), msg.size());
        has_surface = REME_SUCCESS(reme_surface_decimate(_c, _p));
      }
    }
    
    if (has_surface)
    {
      // retrieve data
      reme_surface_get_points(_c, _p, &points, &num_point_coordinates);
      reme_surface_get_normals(_c, _p, &normals, &num_normals_coordinates);
      reme_surface_get_triangles(_c, _p, &faces, &num_triangle_indices);
    
      num_points = num_point_coordinates / 4;
      num_normals = num_normals_coordinates / 4;
      num_faces  = num_triangle_indices / 3;
    }
    emit surface(has_surface, points, num_points, normals, num_normals, faces, num_faces);
  }

  void reme_resource_manager::save(const QString &filename) {
    // Transform the mesh from world space to CAD space, so external viewers
    // can cope better with the result.
    float mat[16];
    reme_transform_set_predefined(_c, REME_TRANSFORM_WORLD_TO_CAD, mat);
    reme_surface_transform(_c, _p, mat);

    reme_surface_save_to_file(_c, _p, filename.toStdString().c_str());
  }

  void reme_resource_manager::reset_volume() {
    reme_volume_reset(_c, _v);
    reme_sensor_reset(_c, _s);
  }

  void reme_resource_manager::get_version(std::string& version) {
    int length;
    const char* data;
    reme_context_get_version(_c, &data, &length);

    version = data;
  }

  void reme_resource_manager::get_opencl_info(opencl_info &ocl) {
    const void *bytes;
    int length;

    reme_options_t o;
    reme_options_create(_c, &o);

    reme_context_bind_opencl_info(_c, o);

    reme_options_get_bytes(_c, o, &bytes, &length); 

    ocl.ParseFromArray(bytes, length);
  }

  void reme_resource_manager::get_hardware_hashes(hardware &hashes) {
    const void *bytes;
    int length;

    reme_options_t o;
    reme_options_create(_c, &o);

    reme_license_t l;
    reme_license_create(_c, &l);
    reme_license_bind_hardware_hashes(_c, l, o);

    reme_options_get_bytes(_c, o, &bytes, &length); 

    hashes.ParseFromArray(bytes, length);
  }

  bool reme_resource_manager::has_valid_license() const {
    return _has_valid_license;
  }

  reme_calibrator_t reme_resource_manager::new_calibrator() const {
    if (_has_compiled_context && _has_sensor) {
      reme_calibrator_t calib;
      reme_calibrator_create(_c, &calib);
      return calib;
    }
    else 
      return REME_ERROR_UNSPECIFIED;
  }

  void reme_resource_manager::destroy_calibrator(reme_calibrator_t calib) {
    if (_has_compiled_context)
      reme_calibrator_destroy(_c, &calib);
  }

  reme_image_t reme_resource_manager::new_image() const {
    if (_has_compiled_context) {
      reme_image_t img;
      reme_image_create(_c, &img);
      return img;
    }
    else 
      return REME_ERROR_UNSPECIFIED;
  }

  void reme_resource_manager::destroy_image(reme_image_t img) {
    reme_image_destroy(_c, &img);
  }

  const reme_context_t reme_resource_manager::context() const{
    return _c;
  }

  const reme_sensor_t reme_resource_manager::sensor() const{
    return _s;
  }

  const reme_volume_t reme_resource_manager::volume() const{
    return _v;
  }
}