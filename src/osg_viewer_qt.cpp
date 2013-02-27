/// @file
/// @copyright Copyright (c) 2006-2012 PROFACTOR GmbH. All rights reserved. Use is subject to license terms.
///
/// @author christoph.kopf@profactor.at
#pragma once

#include "osg_viewer_qt.h"

#include <QtCore/QTimer>
#include <QtGui/QGridLayout>
#include <QMovie>
#include <QLabel>

#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <iostream>

namespace ReconstructMeGUI {

  viewer_widget::viewer_widget(QWidget *parent) : QWidget(parent) 
  {
    setThreadingModel(osgViewer::ViewerBase::SingleThreaded);

    osg::ref_ptr<osg::DisplaySettings> ds = osg::DisplaySettings::instance().get();
    traits = new osg::GraphicsContext::Traits;
    traits->windowDecoration = false;
    traits->x = 0;
    traits->y = 0;
    traits->width = 100;
    traits->height = 100;
    traits->doubleBuffer = true;
    traits->alpha = ds->getMinimumNumAlphaBits();
    traits->stencil = ds->getMinimumNumStencilBits();
    traits->sampleBuffers = ds->getMultiSamples();
    traits->samples = ds->getNumMultiSamples();

    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    window = new osgQt::GraphicsWindowQt(traits.get());
    camera->setGraphicsContext(window);    
    camera->setViewport( new osg::Viewport(0, 0, traits->width, traits->height) );
    camera->setProjectionMatrixAsPerspective(30.0f, static_cast<double>(traits->width)/static_cast<double>(traits->height), 1.0f, 10000.0f );

    grid = new QGridLayout(this);

    view = new osgViewer::View;
    view->setCamera(camera);
    addView(view);
  
    grid->addWidget(window->getGLWidget(), 0, 0);
    setLayout(grid);

    _timer = new QTimer(this);
    connect(_timer, SIGNAL(timeout()), this, SLOT(update()) );  
    this->realize();

    _movie = new QMovie(":/images/loading.gif");
    _process_label = new QLabel(this);
    _process_label->setMovie(_movie);
    _process_label->setFixedSize(65, 65); 
  }

  osg::ref_ptr<osgViewer::View> viewer_widget::osg_view() 
  {
    return view;
  }
   
  void viewer_widget::paintEvent(QPaintEvent* event) 
  { 
    frame();
  }

  void viewer_widget::start_rendering()
  {
    _timer->start(10);
  }

  void viewer_widget::stop_rendering()
  {
    _timer->stop();
  }

  void viewer_widget::start_loading_animation()
  {
    int x = (this->width()  - _process_label->width() ) / 2;
    int y = (this->height() - _process_label->height()) / 2;
    _process_label->move(x, y);
    _movie->start();
    _process_label->show();
  }

  void viewer_widget::stop_loading_animation()
  {
    _movie->stop();
    _process_label->hide();
  }

}