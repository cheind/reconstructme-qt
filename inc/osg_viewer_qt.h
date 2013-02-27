/// @file
/// @copyright Copyright (c) 2013 PROFACTOR GmbH. All rights reserved. 
///
/// @author christoph.kopf@profactor.at
#pragma once

#include <QWidget>


#include <osgViewer/CompositeViewer>
#include <osgQt/GraphicsWindowQt>
#include <QTimer>

// Forward declaration
class QGridLayout;
class QLabel;

namespace ReconstructMeGUI {

  /** Use a OSG view, and render it in a QWidget */
  class viewer_widget : public QWidget, public osgViewer::CompositeViewer {
  public:

    viewer_widget(QWidget *parent = 0);

    osg::ref_ptr<osgViewer::View> osg_view();
    
    void start_rendering();
    void stop_rendering();

    virtual void paintEvent(QPaintEvent* event);

    void start_loading_animation();
    void stop_loading_animation();

  private:
    struct data;
    QGridLayout *grid;
    osg::ref_ptr<osgQt::GraphicsWindowQt> window;
    osg::ref_ptr<osg::GraphicsContext::Traits> traits;
    osg::ref_ptr<osgViewer::View> view;
    QTimer *_timer;
    QLabel *_process_label;
    QMovie *_movie;
  };
}