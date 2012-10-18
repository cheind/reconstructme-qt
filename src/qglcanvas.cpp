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
#include "mutex.h"

#include <QSize>

namespace ReconstructMeGUI {
  QGLCanvas::QGLCanvas(QString &default_img_path, QString &logo_image, QWidget* parent) : QGLWidget(parent), 
    default_img(default_img_path), 
    logo(logo_image) {
    
    set_image_size(0);
  }

  void QGLCanvas::set_image_size(const QSize* size) {
    if (size != 0) 
      img = new QImage(*size, QImage::Format_RGB888);
    else 
      img = &default_img;
    
    repaint();
  }

  QImage* QGLCanvas::image() {
    return img;
  }

  void QGLCanvas::resizeEvent(QResizeEvent*event) {
    img_rect = this->rect();
    QSize s = img->size();
    s.scale(img_rect.size(), Qt::KeepAspectRatio);
    
    img_rect.setLeft(img_rect.left() + ((img_rect.width()  - s.width ()) /2));
    img_rect.setTop(img_rect.top()   + ((img_rect.height() - s.height()) /2));
    
    img_rect.setSize(s);
    
    
    logo_rect = img_rect;
    s = logo.size();
    s.scale(logo_rect.size(), Qt::KeepAspectRatio);
    logo_rect.setTop(logo_rect.bottom()-s.height());
    logo_rect.setSize(s);
  }

  void QGLCanvas::paintEvent(QPaintEvent* ev) {
    QMutexLocker lock(&image_mutex);
    
    QPainter p(this);
    p.fillRect(this->rect(), QColor(60, 60, 60));
    //Set the painter to use a smooth scaling algorithm.
    p.setRenderHint(QPainter::SmoothPixmapTransform, 1);
    p.drawImage(img_rect, *img);
    
    p.drawImage(logo_rect, logo);

    p.end();
  }
}

#endif