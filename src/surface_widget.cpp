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

#include "surface_widget.h"
#include "ui_surface_widget.h"
#include "osg_viewer_qt.h"
#include "reme_resource_manager.h"
#include "surface.pb.h"

#include <reconstructmesdk/reme.h>

#include <QFileDialog>

#include <osgViewer/View>
#include <osg/Node>
#include <osg/ref_ptr>
#include <osg/StateSet>
#include <osg/Geode>
#include <osg/Material>
#include <osg/LightModel>
#include <osg/PolygonMode>
#include <iostream>

namespace ReconstructMeGUI {

  surface_widget::surface_widget(std::shared_ptr<reme_resource_manager> initializer, QWidget *parent) : 
    QWidget(parent),
    _ui(new Ui::surface_widget),
    _i(initializer),
    _has_surface(false)
  {
    _ui->setupUi(this);

    connect(_ui->btnGenerate, SIGNAL(clicked()), SLOT(update_surface()));
    connect(_ui->saveButton, SIGNAL(clicked()), SLOT(save()));
    connect(_ui->polygonRB, SIGNAL(toggled(bool)), SLOT(render_polygon(bool)));
    connect(_ui->wireframeRB, SIGNAL(toggled(bool)), SLOT(render_wireframe(bool)));

    _root = new osg::Group();
    _geode = new osg::Geode();
    _geom = new osg::Geometry();
    _view = _ui->viewer->osg_view();
    _manip = new osgGA::TrackballManipulator;
    
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

  surface_widget::~surface_widget() {
    delete _ui;
  }

  void surface_widget::showEvent(QShowEvent* ev) {
    reme_surface_create(_i->context(), &_s);
    _has_surface = true;

    update_surface();    
    _manip->computeHomePosition();
    _manip->home(0);
    _ui->viewer->start_rendering();
  }

  void surface_widget::hideEvent(QHideEvent* event) {
    if(!_has_surface) {
      return;
    }

    _ui->viewer->stop_rendering();
    reme_surface_destroy(_i->context(), &_s);
    _has_surface = false;
  }

  void surface_widget::update_surface()
  {
    // Assumes that the timer is stopped.
    // Remove old geometry
    const unsigned int n = _geode->getNumDrawables();             
    _geode->removeDrawables(0, _geode->getNumDrawables());

    reme_options_t o;
    reme_options_create(_i->context(), &o);
    std::string msg;

    // Generation
    generation_options go;
    go.set_merge_duplicate_vertices(true);
    go.set_merge_radius((float)_ui->spMergeRadius->value());
    
    reme_surface_bind_generation_options(_i->context(), _s, o);
    go.SerializeToString(&msg);
    reme_options_set_bytes(_i->context(), o, msg.c_str(), msg.size());

    reme_surface_generate(_i->context(), _s, _i->volume());

    // Decimate
    if (_ui->gbDecimation->isChecked()) {
      decimation_options deco;
      deco.set_maximum_error((float)_ui->spMaximumError->value());
      deco.set_maximum_faces(_ui->spMaximumFaces->value());
      deco.set_maximum_vertices(_ui->spMaximumVertices->value());
      
      reme_surface_bind_decimation_options(_i->context(), _s, o);
      deco.SerializeToString(&msg);
      reme_options_set_bytes(_i->context(), o, msg.c_str(), msg.size());

      reme_surface_decimate(_i->context(), _s);
    }

    // Convert to OSG
    const float *points, *normals;
    const unsigned *faces;
    int num_point_coordinates, num_triangle_indices;

    // Transform the mesh from world space to CAD space, so external viewers
    // can cope better with the result.
    //float mat[16];
    //reme_transform_set_predefined(_i->context(), REME_TRANSFORM_WORLD_TO_CAD, mat);
    //reme_surface_transform(_i->context(), s, mat);

    reme_surface_get_points(_i->context(), _s, &points, &num_point_coordinates);
    reme_surface_get_normals(_i->context(), _s, &normals, &num_point_coordinates);
    reme_surface_get_triangles(_i->context(), _s, &faces, &num_triangle_indices);
    
    int num_triangles = num_triangle_indices / 3;
    int num_points = num_point_coordinates / 4;

    _ui->numTrianglesLE->setText(QString::number(num_triangles));
    _ui->numVerticesLE->setText(QString::number(num_points));

    _geom = new osg::Geometry();
    _geom->setUseDisplayList(true);
    _geom->setUseVertexBufferObjects(false);

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

    // Wireframe
    if (_ui->wireframeRB->isChecked())
      render_wireframe(true);
    else
      render_polygon(true);

    _geode->addDrawable(_geom);
    _root->dirtyBound();
  }

  osg::ref_ptr<osg::PolygonMode> surface_widget::poly_mode() {
    osg::ref_ptr<osg::StateSet> state = _geom->getOrCreateStateSet();
    osg::ref_ptr<osg::PolygonMode> polygon_mode;
    polygon_mode = dynamic_cast< osg::PolygonMode* >( state->getAttribute( osg::StateAttribute::POLYGONMODE ));
    if ( !polygon_mode ) {
      polygon_mode = new osg::PolygonMode;
      state->setAttribute( polygon_mode );
    }
    return polygon_mode;
  }

  void surface_widget::render_polygon(bool do_apply) {
    osg::ref_ptr<osg::PolygonMode> polygon_mode = poly_mode();
    if (do_apply)
      polygon_mode->setMode( osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL );
  }

  void surface_widget::render_wireframe(bool do_apply) {
    osg::ref_ptr<osg::PolygonMode> polygon_mode = poly_mode();
    if (do_apply)
      polygon_mode->setMode( osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE );
  }

  void surface_widget::save() {
    QString file_name = QFileDialog::getSaveFileName(this, tr("Save 3D Model"),
                                                 QDir::currentPath(),
                                                 tr("PLY files (*.ply);;OBJ files (*.obj);;3DS files (*.3ds);;STL files (*.stl)"),
                                                 0);
    if (file_name.isEmpty())
      return;

    // Transform the mesh from world space to CAD space, so external viewers
    // can cope better with the result.
    float mat[16];
    reme_transform_set_predefined(_i->context(), REME_TRANSFORM_WORLD_TO_CAD, mat);
    reme_surface_transform(_i->context(), _s, mat);

    reme_surface_save_to_file(_i->context(), _s, file_name.toStdString().c_str());
  }
}