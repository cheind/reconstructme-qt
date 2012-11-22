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

#include "osg_widget.h"
#include "ui_osg_widget.h"

#include "osg_viewer_qt.h"

#include <osgDB/ReadFile>
#include <osg/Node>
#include <osg/ref_ptr>
#include <osgViewer/View>

#include <iostream>

namespace ReconstructMeGUI {

  osg_widget::osg_widget(std::shared_ptr<reme_resource_manager> initializer, QWidget *parent) : 
    QWidget(parent),
    _ui(new Ui::osg_widget),
    _i(initializer)
  {
    _ui->setupUi(this);

    _osg = new viewer_widget(this);
    _ui->gridLayout->addWidget(_osg, 0, 0);


    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry();

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    vertices->push_back(osg::Vec3f(0, 0, 0));
    vertices->push_back(osg::Vec3f(1, 0, 0));
    vertices->push_back(osg::Vec3f(0, 0, 1));
    geometry->setVertexArray(vertices);

    osg::ref_ptr<osg::DrawElementsUInt> triangle_def = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
    triangle_def->push_back(0);
    triangle_def->push_back(1);
    triangle_def->push_back(2);
    geometry->addPrimitiveSet(triangle_def);

    osg::ref_ptr<osg::Group> root = new osg::Group();
    osg::ref_ptr<osg::Geode> geode = new osg::Geode();

    osg::ref_ptr<osg::Node> cow = osgDB::readNodeFile("D:/reconstruct_me_qt_build/Release/cow.osg");
    
    osg::ref_ptr<osgViewer::View> view = new ::osgViewer::View;
    view->setSceneData(root);

    _osg->set_view(view);
    _osg->repaint();
  }

  osg_widget::~osg_widget() {
    delete _ui;
  }

  void osg_widget::showEvent(QShowEvent* ev) {
  }

  void osg_widget::hideEvent(QHideEvent* event) {
  }
}