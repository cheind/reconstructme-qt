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

#pragma once

#include "reconstructme.h"
#include "ui_reconstructmeqt.h"

#include "scan.h"
#include "reme_resource_manager.h"
#include "frame_grabber.h"

#include "settings.h"
#include "strings.h"
#include "defines.h"

#include "qglcanvas.h"
#include "logging_dialog.h"
#include "hardware_key_dialog.h"
#include "about_dialog.h"
#include "settings_dialog.h"
#include "status_dialog.h"

#include <stdlib.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QImage>
#include <QThread>
#include <QLabel>
#include <QFileDialog>
#include <QSettings>
#include <QApplication>
#include <QDesktopWidget>
#include <QKeySequence>
#include <QFont>
#include <QRegExp>
#include <QProgressBar>
#include <QProgressDialog>
#include <QMetaType>
#include <QPushButton>
#include <QSignalMapper>
#include <QWidget>
#include <QCloseEvent>
#include <QMovie>

#include <osg/PolygonMode>
#include <osgUtil/Optimizer>

#include <iostream>

#define STATUSBAR_TIME 1500

namespace ReconstructMeGUI {
  
  reconstructme::reconstructme(QWidget *parent) : 
    QMainWindow(parent),
    _ui(new Ui::reconstructmeqt),
    _rm(new reme_resource_manager()),
    _mode(PAUSE)
  {
    // ui's setup
    _ui->setupUi(this);
    _ui->stackedWidget->setCurrentWidget(_ui->scanPage);

    // move to center
    QRect r = geometry();
    r.moveCenter(QApplication::desktop()->availableGeometry().center());
    setGeometry(r);
    
    // Status bar
    _label_fps = new QLabel();
    _label_fps->setStyleSheet("qproperty-alignment: AlignRight; margin-right: 0px; padding-right: 0px;");
    _label_fps->setMaximumWidth(100);
    _label_fps->setToolTip(tool_tip_fps_label_tag);
    
    _label_fps_color = new QLabel();
    _label_fps_color->setMinimumWidth(10);
    _label_fps_color->setMaximumWidth(10);
    _label_fps_color->setStyleSheet("margin-left: 0px; padding-left: 0px;");
    _label_fps_color->setAutoFillBackground(true);
    _label_fps_color->setToolTip(tool_tip_fps_color_label_tag);

    statusBar()->addPermanentWidget(_label_fps, 0);
    statusBar()->addPermanentWidget(_label_fps_color, 0);

    // Create dialogs
    _dialog_log = new logging_dialog(_rm, this, Qt::Dialog);
    _dialog_settings = new settings_dialog(_rm, this);
    _dialog_license = new hardware_key_dialog(_rm, this);
    _dialog_about = new about_dialog(this);
    _dialog_state = new status_dialog(_rm, this);

    _dialog_license->connect(_ui->actionGenerate_hardware_key, SIGNAL(triggered()), SLOT(show()));
    _dialog_log->connect(_ui->actionLog, SIGNAL(triggered()), SLOT(show()));
    _dialog_about->connect(_ui->actionAbout, SIGNAL(triggered()), SLOT(show()));
    _dialog_settings->connect(_ui->actionSettings, SIGNAL(triggered()), SLOT(show()));

    // application icon
    QPixmap titleBarPix (":/images/icon.ico");
    QIcon titleBarIcon(titleBarPix);
    setWindowIcon(titleBarIcon);

    // Create 
    create_url_mappings();

    // surface rendering
    _root = new osg::Group();
    _geode_group = new osg::Group();
    _view = _ui->viewer->osg_view();
    _manip = new osgGA::TrackballManipulator;
    
    _root->addChild(_geode_group);
    _view->setSceneData(_root);
    _view->setCameraManipulator(_manip);
    _view->getCamera()->setClearColor(osg::Vec4(50/255.f, 50/255.f, 50/255.f, 1.0));

    _mat = new osg::Material();
    // Front side
    _mat->setDiffuse(osg::Material::FRONT,  osg::Vec4(200/255.f, 214/255.f, 230/255.f, 1.0));
    _mat->setSpecular(osg::Material::FRONT, osg::Vec4(0.2f, 0.2f, 0.2f, 1.0));
    _mat->setAmbient(osg::Material::FRONT,  osg::Vec4(0.0f, 0.0f, 0.0f, 1.0));
    _mat->setEmission(osg::Material::FRONT, osg::Vec4(0.0, 0.0, 0.0, 1.0));
    _mat->setShininess(osg::Material::FRONT, 10);
    // Back side
    _mat->setDiffuse(osg::Material::BACK,  osg::Vec4(255/255.f, 217/255.f, 228/255.f, 1.0));
    _mat->setSpecular(osg::Material::BACK, osg::Vec4(0.2f, 0.2f, 0.2f, 1.0));
    _mat->setAmbient(osg::Material::BACK,  osg::Vec4(0.0f, 0.0f, 0.0f, 1.0));
    _mat->setEmission(osg::Material::BACK, osg::Vec4(0.0, 0.0, 0.0, 1.0));
    _mat->setShininess(osg::Material::BACK, 2);

    _lightmodel = new osg::LightModel();
    _lightmodel->setTwoSided(true);

    // Trigger concurrent initialization
    _fg = std::shared_ptr<frame_grabber>(new frame_grabber(_rm));
    _rm->set_frame_grabber(_fg);
    connect(_fg.get(), SIGNAL(frame(reme_sensor_image_t, const void*, int, int, int, int, int, int)), SLOT(show_frame(reme_sensor_image_t, const void*, int, int, int, int, int, int)));
    _rm->connect(this, SIGNAL(initialize()), SLOT(initialize()));
    _rm_thread = new QThread(this);
    _rm->moveToThread(_rm_thread);
    _fg->moveToThread(_rm_thread);
    _rm_thread->start();

    _fg->request(REME_IMAGE_AUX);
    _fg->request(REME_IMAGE_DEPTH);
    _fg->request(REME_IMAGE_VOLUME);

    _rm->connect(this, SIGNAL(start_scanning()), SLOT(start_scanning()));
    _rm->connect(this, SIGNAL(stop_scanning()), SLOT(stop_scanning()));

    connect(_ui->play_button, SIGNAL(clicked()), SLOT(toggle_mode()));
    _rm->connect(_ui->reset_button, SIGNAL(clicked()), SLOT(reset_volume()));
    connect(_rm.get(), SIGNAL(current_fps(const float)), SLOT(show_fps(const float)));
    qRegisterMetaType<QSharedPointer<generation_options>>("QSharedPointer<generation_options>");
    qRegisterMetaType<QSharedPointer<decimation_options>>("QSharedPointer<decimation_options>");
    _rm->connect(this, SIGNAL(generate_surface(const QSharedPointer<generation_options>, const QSharedPointer<decimation_options>)), SLOT(generate_surface(const QSharedPointer<generation_options>, const QSharedPointer<decimation_options>)));
    connect(_rm.get(), SIGNAL(surface(bool, const float *, int, const float *, int, const unsigned *, int)), SLOT(render_surface(bool, const float *, int, const float *, int, const unsigned *, int)));
    connect(_ui->btnGenerate, SIGNAL(clicked()), SLOT(request_surface()));
    _rm->connect(this, SIGNAL(save_surface(const char *)), SLOT(save(const char *)));
    connect(_ui->saveButton, SIGNAL(clicked()), SLOT(save()));
    connect(_ui->polygonRB, SIGNAL(toggled(bool)), SLOT(render_polygon(bool)));
    connect(_ui->wireframeRB, SIGNAL(toggled(bool)), SLOT(render_wireframe(bool)));

    emit initialize();
  }

  void reconstructme::show_frame(reme_sensor_image_t type, const void* data, int length, int width, int height, int channels, int num_bytes_per_channel, int row_stride) {
    switch(type) {
    case REME_IMAGE_AUX:
      _ui->rgb_canvas->set_image_size(width, height);
      _ui->rgb_canvas->set_image_data(data, length);
      break;
    case REME_IMAGE_DEPTH:
      _ui->depth_canvas->set_image_size(width, height);
      _ui->depth_canvas->set_image_data(data, length);
      break;
    case REME_IMAGE_VOLUME:
      _ui->rec_canvas->set_image_size(width, height);
      _ui->rec_canvas->set_image_data(data, length);
      break;
    }
  }

  void reconstructme::toggle_mode() {
    disconnect(_fg.get(), SIGNAL(frame(reme_sensor_image_t, const void*, int, int, int, int, int, int)), this, SLOT(show_frame(reme_sensor_image_t, const void*, int, int, int, int, int, int)));

    if (_mode == PAUSE) {
      _mode = PLAY;
      _fg->request(REME_IMAGE_AUX);
      _fg->request(REME_IMAGE_DEPTH);
      _fg->request(REME_IMAGE_VOLUME);
      connect(_fg.get(), SIGNAL(frame(reme_sensor_image_t, const void*, int, int, int, int, int, int)), SLOT(show_frame(reme_sensor_image_t, const void*, int, int, int, int, int, int)));
      _ui->stackedWidget->setCurrentWidget(_ui->scanPage);
      emit start_scanning();
    }
    else if (_mode == PLAY) {
      _mode = PAUSE;
      _fg->release(REME_IMAGE_AUX);
      _fg->release(REME_IMAGE_DEPTH);
      _fg->release(REME_IMAGE_VOLUME);
      _ui->stackedWidget->setCurrentWidget(_ui->surfacePage);
      request_surface();
      emit stop_scanning();
    }
  }

  void reconstructme::request_surface()
  {
    //_dialog_license->show();

    _ui->viewer->start_loading_animation();
    
    // Assumes that the timer is stopped.
    // Remove old geometry
    const unsigned int n = _geode_group->getNumChildren();             
    _geode_group->removeChildren(0, n);

    _ui->viewer->stop_rendering();

    // surface options
    QSharedPointer<generation_options> go(new generation_options);
    go->set_merge_duplicate_vertices(true);
    go->set_merge_radius((float)_ui->spMergeRadius->value());

    // Decimate
    QSharedPointer<decimation_options> deco;
    if (_ui->gbDecimation->isChecked()) {
      deco = QSharedPointer<decimation_options>(new decimation_options);
      deco->set_maximum_error((float)_ui->spMaximumError->value());
      deco->set_maximum_faces(_ui->spMaximumFaces->value());
      deco->set_maximum_vertices(_ui->spMaximumVertices->value());
      deco->set_minimum_roundness(_ui->spMinRoundness->value());
    }
    emit generate_surface(go, deco);
  }

  void reconstructme::render_surface( bool has_surface,
      const float *points, int num_points,
      const float *normals, int num_normals,
      const unsigned *faces, int num_faces) 
  {
    std::cout << "render_surface" << std::endl;
    _ui->viewer->stop_loading_animation();

    if (!has_surface) {
      QMessageBox::information(this, "Rendering Surface", "Could not create surface.", QMessageBox::Ok);
      return;
    }


    _ui->numTrianglesLE->setText(QString::number(num_faces));
    _ui->numVerticesLE->setText(QString::number(num_points));

    osg::ref_ptr<osg::Geode> geode;
    osg::ref_ptr<osg::Geometry> geom;

    geom = new osg::Geometry();
    geom->setUseDisplayList(true);
    geom->setUseVertexBufferObjects(false);

    typedef osg::TemplateIndexArray<unsigned int, osg::Array::UIntArrayType, 24, 4> index_array_type;

    osg::ref_ptr<osg::Vec3Array> vertex_coords = new osg::Vec3Array();
    osg::ref_ptr<osg::Vec3Array> vertex_normals = new osg::Vec3Array();
    osg::ref_ptr<osg::DrawElementsUInt> face_to_vertex = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
    osg::ref_ptr<index_array_type> normal_to_vertex = new index_array_type();

    int i = 0;
    while (i < num_points)
    {
      vertex_coords->push_back(osg::Vec3(points[i*4+0], points[i*4+1], points[i*4+2]));
      vertex_normals->push_back(osg::Vec3(normals[i*4+0], normals[i*4+1], normals[i*4+2]));
      normal_to_vertex->push_back(i);
      i++;
    }

    face_to_vertex->insert(face_to_vertex->begin(), faces, faces + num_faces * 3);

    geom->setVertexArray(vertex_coords);
    geom->setNormalArray(vertex_normals);
    geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    geom->setNormalIndices(normal_to_vertex);
    geom->addPrimitiveSet(face_to_vertex);
      
    geode = new osg::Geode();
    geode->getOrCreateStateSet()->setAttributeAndModes(_mat, osg::StateAttribute::ON);
    geode->getOrCreateStateSet()->setAttributeAndModes(_lightmodel, osg::StateAttribute::ON);
    
    osgUtil::Optimizer optimizer;
    optimizer.optimize(geode, osgUtil::Optimizer::DEFAULT_OPTIMIZATIONS);
    
    geode->addDrawable(geom);
    _geode_group->addChild(geode);
    
    // Rendermode
    if (_ui->wireframeRB->isChecked())
      render_wireframe(true);
    else
      render_polygon(true);

    _root->dirtyBound();

    _manip->computeHomePosition();
    _manip->home(0);
    _ui->viewer->start_rendering();
    
    //_dialog_li->hide();
  }

  void reconstructme::save() {
    const QString file_name = QFileDialog::getSaveFileName(this, tr("Save 3D Model"),
      QDir::currentPath(),
      tr("PLY files (*.ply);;OBJ files (*.obj);;3DS files (*.3ds);;STL files (*.stl);; RAW Volume (*.raw)"),
      0);

    if (file_name.isEmpty())
      return;

    emit save_surface(file_name.toStdString().c_str());

  }

  osg::ref_ptr<osg::PolygonMode> reconstructme::poly_mode() {
    osg::ref_ptr<osg::StateSet> state = _geode_group->getOrCreateStateSet();
    osg::ref_ptr<osg::PolygonMode> polygon_mode;
    polygon_mode = dynamic_cast< osg::PolygonMode* >( state->getAttribute( osg::StateAttribute::POLYGONMODE ));
    if ( !polygon_mode ) {
      polygon_mode = new osg::PolygonMode;
      state->setAttribute( polygon_mode );
    }
    return polygon_mode;
  }

  void reconstructme::render_polygon(bool do_apply) {
    osg::ref_ptr<osg::PolygonMode> polygon_mode = poly_mode();
    if (do_apply)
      polygon_mode->setMode( osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL );
  }

  void reconstructme::render_wireframe(bool do_apply) {
    osg::ref_ptr<osg::PolygonMode> polygon_mode = poly_mode();
    if (do_apply)
      polygon_mode->setMode( osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE );
  }

  void reconstructme::create_url_mappings() {
    _url_mapper = new QSignalMapper(this);
    _url_mapper->setMapping(_ui->actionDevice, QString(url_device_matrix_tag));
    _url_mapper->setMapping(_ui->actionFAQ_2, QString(url_faq_tag));
    _url_mapper->setMapping(_ui->actionForum, QString(url_forum_tag));
    _url_mapper->setMapping(_ui->actionInstallation, QString(url_install_tag));
    _url_mapper->setMapping(_ui->actionProjectHome, QString(url_reconstructme_qt));
    _url_mapper->setMapping(_ui->actionSDKDocumentation, QString(url_sdk_doku_tag));
    _url_mapper->setMapping(_ui->actionUsage, QString(url_usage_tag));
    _url_mapper->setMapping(_dialog_state->onlineHelpBtn(), QString(url_faq_tag));
    
    _url_mapper->connect(_ui->actionDevice, SIGNAL(triggered()), SLOT(map()));
    _url_mapper->connect(_ui->actionFAQ_2, SIGNAL(triggered()), SLOT(map()));
    _url_mapper->connect(_ui->actionForum, SIGNAL(triggered()), SLOT(map()));
    _url_mapper->connect(_ui->actionInstallation, SIGNAL(triggered()), SLOT(map()));
    _url_mapper->connect(_ui->actionProjectHome, SIGNAL(triggered()), SLOT(map()));
    _url_mapper->connect(_ui->actionSDKDocumentation, SIGNAL(triggered()), SLOT(map()));
    _url_mapper->connect(_ui->actionUsage, SIGNAL(triggered()), SLOT(map()));
    _url_mapper->connect(_dialog_state->onlineHelpBtn(), SIGNAL(clicked()), SLOT(map()));

    connect(_url_mapper, SIGNAL(mapped(const QString&)), SLOT(open_url(const QString&)));
  }

  // ==================== closing ====================
  void reconstructme::closeEvent(QCloseEvent *ev) {
    if (_fg->is_grabbing()) {
      // 1. stop frame grabber before closing
      _fg->connect(this, SIGNAL(closing()), SLOT(stop()), Qt::QueuedConnection);
      this->connect(_fg.get(), SIGNAL(stopped_grabbing()), SLOT(really_close()), Qt::QueuedConnection);
      emit closing();
      ev->ignore();
    } else {
      // 3. close QMainWindow
      ev->accept();
      QMainWindow::closeEvent(ev);
    }
  }

  void reconstructme::really_close() {
    // 2. frame grabber stopped, so call close event again
    this->close();
  }

  reconstructme::~reconstructme() {
    // 4. Since the close event is accepted, destruct this
    _rm_thread->quit();
    _rm_thread->wait();

    delete _ui;
  }

  // ==================== closing ====================

  void reconstructme::show_message_box(
      int icon, 
      QString message, 
      int btn_1,
      int btn_2) {
    
    switch (icon) {
      case QMessageBox::Warning:
        QMessageBox::warning(this, warning_tag, message, btn_1, btn_2);
        break;
      case QMessageBox::Critical:
        QMessageBox::critical(this, critical_tag, message, btn_1, btn_2);
        break;
      case QMessageBox::Information:
        QMessageBox::information(this, information_tag, message, btn_1, btn_2);
        break;
      case QMessageBox::Question:
        QMessageBox::question(this, question_tag, message, btn_1, btn_2);
        break;
    }
  }

  void reconstructme::action_settings_clicked() {
    _dialog_settings->show();
    _dialog_settings->raise();
    _dialog_settings->activateWindow();
  }

  void reconstructme::open_url(const QString &url_string) {
    QUrl url (url_string);
    QDesktopServices::openUrl(url);
    QString msg = open_url_tag + url_string;
    _ui->reconstruct_satus_bar->showMessage(msg, STATUSBAR_TIME);
  }

  void reconstructme::status_bar_msg(const QString &msg, const int msecs) {
    statusBar()->showMessage(msg, msecs);
  }

  void reconstructme::show_fps(const float fps) {
    if (fps > 20) 
      _label_fps_color->setStyleSheet("background-color: green;");
    else if (fps > 10)
      _label_fps_color->setStyleSheet("background-color: orange;");
    else
      _label_fps_color->setStyleSheet("background-color: #FF4848;");
    
    _label_fps->setText(QString().sprintf("%.2f fps", fps));
  }
}
