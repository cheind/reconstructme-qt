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
#include "reme_resource_manager.h"

#include <reconstructmesdk/reme.h>

#include <osgViewer/View>
#include <osg/Node>
#include <osg/ref_ptr>
#include <osg/StateSet>
#include <osg/Geode>
#include <osg/Material>
#include <osg/LightModel>
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

    _root = new osg::Group();
    _geode = new osg::Geode();
    _geom = new osg::Geometry();
    _view = _osg->osg_view();
    _manip = new osgGA::TrackballManipulator();

    _root->addChild(_geode);
    _view->setSceneData(_root);
    _view->setCameraManipulator(_manip);
    _view->getCamera()->setClearColor(osg::Vec4(50/255.f, 50/255.f, 50/255.f, 1.0));

    osg::ref_ptr<osg::Material> mat = new osg::Material();
    
    // Front side
    mat->setDiffuse(osg::Material::FRONT,  osg::Vec4(200/255.f, 214/255.f, 230/255.f, 1.0));
    mat->setSpecular(osg::Material::FRONT, osg::Vec4(0.2f, 0.2f, 0.2f, 1.0));
    mat->setAmbient(osg::Material::FRONT,  osg::Vec4(0.0f, 0.0f, 0.0f, 1.0));
    mat->setEmission(osg::Material::FRONT, osg::Vec4(0.0, 0.0, 0.0, 1.0));
    mat->setShininess(osg::Material::FRONT, 10);
    
    // Back side
    mat->setDiffuse(osg::Material::BACK,  osg::Vec4(255/255.f, 217/255.f, 228/255.f, 1.0));
    mat->setSpecular(osg::Material::BACK, osg::Vec4(0.2f, 0.2f, 0.2f, 1.0));
    mat->setAmbient(osg::Material::BACK,  osg::Vec4(0.0f, 0.0f, 0.0f, 1.0));
    mat->setEmission(osg::Material::BACK, osg::Vec4(0.0, 0.0, 0.0, 1.0));
    mat->setShininess(osg::Material::BACK, 2);

    osg::ref_ptr<osg::LightModel> lightmodel = new osg::LightModel();
    lightmodel->setTwoSided(true);

    _geode->getOrCreateStateSet()->setAttributeAndModes(mat, osg::StateAttribute::ON);
    _geode->getOrCreateStateSet()->setAttributeAndModes(lightmodel, osg::StateAttribute::ON);
  }

  osg_widget::~osg_widget() {
    delete _ui;
  }

  void osg_widget::showEvent(QShowEvent* ev) {
    update_surface();
    _osg->start_rendering();
  }

  void osg_widget::hideEvent(QHideEvent* event) {
    _osg->stop_rendering();
  }

  void osg_widget::update_surface()
  {
    // Assumes that the timer is stopped.

    // Remove old geometry
    const unsigned int n = _geode->getNumDrawables();             
    _geode->removeDrawables(0, _geode->getNumDrawables());

    // Create surface using SDK
    reme_surface_t s;
    reme_surface_create(_i->context(), &s);
    reme_surface_generate(_i->context(), s, _i->volume());

    // Convert to OSG
    const float *points, *normals;
    const unsigned *faces;
    int num_points, num_triangles;

    reme_surface_get_points(_i->context(), s, &points, &num_points);
    reme_surface_get_normals(_i->context(), s, &normals, &num_points);
    reme_surface_get_triangles(_i->context(), s, &faces, &num_triangles);

    num_triangles /= 3;

    _geom = new osg::Geometry();
    _geom->setUseDisplayList(false);
    _geom->setUseVertexBufferObjects(true);

    typedef osg::TemplateIndexArray<unsigned int, osg::Array::UIntArrayType, 24, 4> index_array_type;

    osg::ref_ptr<osg::Vec3Array> vertex_coords = new osg::Vec3Array();
    osg::ref_ptr<osg::Vec3Array> vertex_normals = new osg::Vec3Array();
    osg::ref_ptr<osg::DrawElementsUInt> face_to_vertex = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
    osg::ref_ptr<index_array_type> normal_to_vertex = new index_array_type();

    for (int i = 0; i < num_points; ++i) {
      vertex_coords->push_back(osg::Vec3(points[i*4+0], points[i*4+1], points[i*4+2]));
      vertex_normals->push_back(osg::Vec3(normals[i*4+0], normals[i*4+1], normals[i*4+2]));
      normal_to_vertex->push_back(i);
    }

    face_to_vertex->insert(face_to_vertex->begin(), faces, faces + num_triangles * 3);

    _geom->setVertexArray(vertex_coords);
    _geom->setNormalArray(vertex_normals);
    _geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    _geom->setNormalIndices(normal_to_vertex);
    _geom->addPrimitiveSet(face_to_vertex);
   
    _geode->addDrawable(_geom);
    _geode->dirtyBound();
    _root->dirtyBound();

    _manip->setNode(_root);
    _manip->computeHomePosition();
    _manip->home(0);

    reme_surface_destroy(_i->context(), &s);
  }
}