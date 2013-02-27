/** @file
  * @copyright Copyright (c) 2013 PROFACTOR GmbH. All rights reserved. 
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
  
#ifndef FRAMEGRABBER_H
#define FRAMEGRABBER_H

#pragma once

#include <QObject>  
#include <QSet>

#include <reconstructmesdk/types.h>

// Forward declarations
namespace ReconstructMeGUI {
  class reme_resource_manager;
}

namespace ReconstructMeGUI {

  /** Main UI element of the ReconstructMeQT application 
  *
  * \note This is the topmost element of the UI-tree. It holds references to 
  *       all other UI elements, such as the necessary dialogs.
  */
  class frame_grabber : public QObject
  {
    Q_OBJECT
    
  public:
    frame_grabber(std::shared_ptr<reme_resource_manager> rm);
    ~frame_grabber();

    bool is_grabbing();
    void request(reme_image_t image);
    void release(reme_image_t image);
    
  private slots:
    void start(bool);
  
  public slots:
    void stop();

  signals:
    void frame(reme_sensor_image_t type, const void* data, int length=0, int width=0, int height=0, int channels=0, int num_bytes_per_channel=0, int row_stride=0); 
    void frames_updated();
    void stopped_grabbing();

  private:
    std::shared_ptr<reme_resource_manager> _rm;
    bool _do_grab;

    reme_image_t _rgb;
    reme_image_t _phong;
    reme_image_t _depth;

    int _req_count[3];    
  };
}

#endif // FRAMEGRABBER_H
