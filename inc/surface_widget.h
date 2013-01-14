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
  */

#ifndef SURFACE_WIDGET_H
#define SURFACE_WIDGET_H

#include "types.h"

#include <QWidget>
#include <QTimer>
#include <QFutureWatcher>

#include <reconstructmesdk/types.h>

#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Group>
#include <osgViewer/View>
#include <osgGA/TrackballManipulator>
#include <osg/PolygonMode>

// Forward declarations
namespace Ui {
  class surface_widget;
}
namespace ReconstructMeGUI {
  class reme_resource_manager;
  class calibrate;
}

namespace ReconstructMeGUI {

  class surface_widget : public QWidget
  {
    Q_OBJECT;

  public:
    surface_widget(std::shared_ptr<reme_resource_manager> initializer, QWidget *parent = 0);
    ~surface_widget();

    virtual void showEvent(QShowEvent* event);
    virtual void hideEvent(QHideEvent* event);
    
    void update_surface_concurrent();

  protected slots:
    void update_surface();
    void save();
    
  private slots:
    void render_polygon(bool do_apply);
    void render_wireframe(bool do_apply);
    void render();
    
  private:
    osg::ref_ptr<osg::PolygonMode> poly_mode();

    Ui::surface_widget *_ui;
    std::shared_ptr<reme_resource_manager> _i;
    osg::ref_ptr<osgViewer::View> _view;
    osg::ref_ptr<osg::Group> _root;
    osg::ref_ptr<osg::Geometry> _geom;
    osg::ref_ptr<osg::Geode> _geode;
    osg::ref_ptr<osgGA::CameraManipulator> _manip;
    reme_surface_t _s;

    QFutureWatcher<void> _fw;
    QFuture<void> _future;
    bool _has_surface;
  };
} 

#endif // CALIBRATION_WIDGET_H