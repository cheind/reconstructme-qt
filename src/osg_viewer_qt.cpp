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

namespace ReconstructMeGUI {

  viewer_widget::viewer_widget(QWidget *parent) : QWidget(parent) {
    setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
    grid = new QGridLayout(this);

    osg::ref_ptr<osg::DisplaySettings> ds = osg::DisplaySettings::instance().get();
    traits = new osg::GraphicsContext::Traits;
    traits->windowDecoration = false;
    traits->x = 0;
    traits->y = 0;
    traits->doubleBuffer = true;
    traits->alpha = ds->getMinimumNumAlphaBits();
    traits->stencil = ds->getMinimumNumStencilBits();
    traits->sampleBuffers = ds->getMultiSamples();
    traits->samples = ds->getNumMultiSamples();

    window = new osgQt::GraphicsWindowQt(traits.get());
  }

  void viewer_widget::set_view(osg::ref_ptr<osgViewer::View> view) {
    osg::ref_ptr<osg::Camera> camera = view->getCamera();
    camera->setGraphicsContext(window);
    camera->setViewport( new osg::Viewport(0, 0, traits->width, traits->height) );

    this->addView(view);

    grid->addWidget(window->getGLWidget(), 0, 0);
    setLayout(grid);
  }
   
  void viewer_widget::paintEvent(QPaintEvent* event) { 
    std::cout << "test" << std::endl;
    frame(); // trigger render content
  }

}