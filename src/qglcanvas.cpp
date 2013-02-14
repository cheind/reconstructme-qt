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

#ifndef QGLCANVAS_H
#define QGLCANVAS_H

#pragma once

#include "qglcanvas.h"

#include <QSize>

#include <iostream>

namespace ReconstructMeGUI {

  QGLCanvas::QGLCanvas(QWidget* parent) : QGLWidget(parent),
    _width(1),
    _height(1)
  {
    img = new QImage(_width, _height, QImage::Format_RGB888);
  }

  void QGLCanvas::set_image_size(int width, int height) {
    if ((_width != width || _height != height) && width > 0 && height > 0) {
      _width = width;
      _height = height;
      img = new QImage(_width, _width, QImage::Format_RGB888);
    }
  }

  void QGLCanvas::set_image_data(const void *data) {
    if (data == 0)
      return;
    std::cout << img->byteCount() << std::endl;
    for (int h=0; h<_height; h++) {
    // scanLine returns a ptr to the start of the data for that row
      memcpy(img->scanLine(h), &(static_cast<const char*>(data)[h]), _width*3);
    }
    //memcpy((void*)img->bits(), data, img->byteCount());
    repaint();
  }

  void QGLCanvas::paintEvent(QPaintEvent* ev) {
    QPainter p(this);

    if (!_has_data) {
      p.fillRect(this->rect(), QColor(0, 0, 0));
    } 
    else {
      //Set the painter to use a smooth scaling algorithm.
      p.setRenderHint(QPainter::SmoothPixmapTransform, 1);
      p.drawImage(this->rect(), *img);
    }

    p.end();
  }

  void QGLCanvas::mouseReleaseEvent(QMouseEvent *event) {
    emit mouse_released();
  }

}

#endif