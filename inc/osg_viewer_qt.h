/// @file
/// @copyright Copyright (c) 2006-2012 PROFACTOR GmbH. All rights reserved. Use is subject to license terms.
///
/// @author christoph.kopf@profactor.at
#pragma once

#include <QWidget>


#include <osgViewer/CompositeViewer>
#include <osgQt/GraphicsWindowQt>

// Forward declaration
class QTimer;
class QGridLayout;

namespace ReconstructMeGUI {

  /** Use a OSG view, and render it in a QWidget */
  class viewer_widget : public QWidget, public osgViewer::CompositeViewer {
  public:
      viewer_widget(QWidget *parent = 0);

      /**  */
      void set_view(osg::ref_ptr<osgViewer::View> view);

      /** repaint scene, eg. this->frame() */
      virtual void paintEvent(QPaintEvent* event);

  private:
    struct data;
    QGridLayout *grid;
    osg::ref_ptr<osgQt::GraphicsWindowQt> window;
    osg::ref_ptr<osg::GraphicsContext::Traits> traits;
  };
}