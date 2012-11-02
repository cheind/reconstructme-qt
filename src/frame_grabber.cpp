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

#include "frame_grabber.h"
#include "reme_sdk_initializer.h"

#include <reconstructmesdk/reme.h>

#include <QCoreApplication>
#include <qdebug.h>

#include <iostream>


namespace ReconstructMeGUI {
  frame_grabber::frame_grabber(std::shared_ptr<reme_sdk_initializer> initializer) : 
    _i(initializer) 
  {
    qRegisterMetaType<reme_sensor_image_t>("reme_sensor_image_t");

    connect(_i.get(), SIGNAL(initializing_sdk()), SLOT(stop()), Qt::BlockingQueuedConnection);
    connect(_i.get(), SIGNAL(sdk_initialized(bool)), SLOT(start(bool)));
  }
    
  frame_grabber::~frame_grabber() {
    std::cout << "~frame_grabber" << std::endl; 
  }


  void frame_grabber::start(bool initialization_success) {
    if (!initialization_success) return;

    // Image creation
    reme_image_create(_i->context(), &_rgb);
    reme_image_create(_i->context(), &_depth);
    reme_image_create(_i->context(), &_phong);

    // Grabbing utils
    _do_grab = true;
    bool success = true;

    std::cout << "start" << std::endl;
    while (_do_grab && success)
    {
      // Prepare image and depth data
      success = success && REME_SUCCESS(reme_sensor_grab(_i->context(), _i->sensor()));
      success = success && REME_SUCCESS(reme_sensor_prepare_images(_i->context(), _i->sensor()));
      
      if (_i->rgb_size() && REME_SUCCESS(reme_sensor_update_image(_i->context(), _i->sensor(), REME_IMAGE_AUX, _rgb))) 
        emit frame(REME_IMAGE_AUX, _rgb);
      
      if (_i->depth_size() && REME_SUCCESS(reme_sensor_update_image(_i->context(), _i->sensor(), REME_IMAGE_DEPTH, _depth))) 
        emit frame(REME_IMAGE_DEPTH, _depth);

      if (_i->phong_size() && REME_SUCCESS(reme_sensor_update_image(_i->context(), _i->sensor(), REME_IMAGE_VOLUME, _phong))) 
        emit frame(REME_IMAGE_VOLUME, _phong);

      emit frames_updated();
      QCoreApplication::processEvents();
    }

    std::cout << "end_while" << std::endl;
  }

  void frame_grabber::stop() {
    _do_grab = false;
    std::cout << "stop" << std::endl;
  }
}