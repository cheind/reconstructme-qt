/// @file
/// @copyright Copyright (c) 2006-2012 PROFACTOR GmbH. All rights reserved. Use is subject to license terms.
///
/// @author christoph.kopf@profactor.at
#pragma once

#include "osg_viewer_qt.h"

#include <QtCore/QTimer>
#include <QtGui/QGridLayout>

#include <osgViewer/ViewerEventHandlers>

#include <osgGA/TrackballManipulator>

#include <osgDB/ReadFile>

namespace ReconstructMeUI {
  namespace UI {

    viewer_widget::viewer_widget(osgViewer::View *view) : QWidget()
    {
      timer = new QTimer(this);
      grid = new QGridLayout(this);

      setThreadingModel(osgViewer::ViewerBase::SingleThreaded);

      osg::Camera *camera = view->getCamera();

      osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
      osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
      traits->windowDecoration = false;
      traits->x = 0;
      traits->y = 0;
      //traits->width = 640;
      //traits->height = 480;
      traits->doubleBuffer = true;
      traits->alpha = ds->getMinimumNumAlphaBits();
      traits->stencil = ds->getMinimumNumStencilBits();
      traits->sampleBuffers = ds->getMultiSamples();
      traits->samples = ds->getNumMultiSamples();

      osgQt::GraphicsWindowQt *window = new osgQt::GraphicsWindowQt(traits.get());
      camera->setGraphicsContext(window);
      camera->setViewport( new osg::Viewport(0, 0, traits->width, traits->height) );

      this->addView(view);

      grid->addWidget(window->getGLWidget(), 0, 0);
      setLayout(grid);
        
      //connect(timer, SIGNAL(timeout()), this, SLOT(update()));
      //timer->start(10);
    }

   
    void viewer_widget::paintEvent( QPaintEvent* event )
    { 
      this->frame(); 
    }

  }
}