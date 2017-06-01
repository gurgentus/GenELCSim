#include <QDebug>
#include <QString>
#include <QtOpenGL>
#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <common/vertex.h>
#include "mainviewwidget.h"
#include "nums/simulator.h"
#include <window.h>

#include <iostream>

MainViewWidget::MainViewWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    setFocusPolicy( Qt::StrongFocus );
}

MainViewWidget::~MainViewWidget()
{
}

void MainViewWidget::InitializeObjects(QOpenGLShaderProgram* shader, Textures* textures)
{
    p_program_ = shader;
    p_textures_ = textures;
}


QSize MainViewWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize MainViewWidget::sizeHint() const
{
    return QSize(1000, 1000);
}

void MainViewWidget::initializeGL()
{
    // Initialize OpenGL Backend
    QOpenGLFunctions::initializeOpenGLFunctions();
    connect(context(), SIGNAL(aboutToBeDestroyed()), this, SLOT(teardownGL()), Qt::DirectConnection);
    printVersionInformation();

    p_program_->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/texturedDiffuse.vert");
    p_program_->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/texturedDiffuse.frag");
    p_program_->link();
    p_program_->bind();

    // Cache Uniform Locations
    world_to_camera_ = p_program_->uniformLocation("worldToCamera");
    camera_to_view_ = p_program_->uniformLocation("cameraToView");
    p_program_->setUniformValue(world_to_camera_, QMatrix4x4());
    p_program_->setUniformValue(camera_to_view_, QMatrix4x4());
}

void MainViewWidget::resizeGL(int width, int height)
{
    // Currently we are not handling width/height changes.
    (void)width;
    (void)height;
    projection_.setToIdentity();
    projection_.perspective(45.0f, width / float(height), 1.0f, 1000.0f);
}

void MainViewWidget::paintGL()
{
  // Clear
  glClear(GL_COLOR_BUFFER_BIT);
  glClear(GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  p_program_->bind();

  p_program_->setUniformValue(world_to_camera_, camera_.toMatrix());
  p_program_->setUniformValue(camera_to_view_, projection_);
  p_program_->setUniformValue(p_program_->uniformLocation("modelToWorld"), QMatrix4x4());

  p_program_->release();
}

void MainViewWidget::update()
{
    // Update input
    Input::update();

    if (Input::buttonPressed(Qt::RightButton))
    {
      static const float transSpeed = 0.5f;
      static const float rotSpeed   = 0.5f;

      // Handle rotations
      camera_.rotate(-rotSpeed * Input::mouseDelta().x(), Camera3D::LocalUp);
      camera_.rotate(-rotSpeed * Input::mouseDelta().y(), camera_.right());

      // Handle translations
      QVector3D translation;
      if (Input::keyPressed(Qt::Key_W))
      {
        translation += camera_.forward();
      }
      if (Input::keyPressed(Qt::Key_S))
      {
        translation -= camera_.forward();
      }
      if (Input::keyPressed(Qt::Key_A))
      {
        translation -= camera_.right();
      }
      if (Input::keyPressed(Qt::Key_D))
      {
        translation += camera_.right();
      }
      if (Input::keyPressed(Qt::Key_Q))
      {
        translation -= camera_.up();
      }
      if (Input::keyPressed(Qt::Key_E))
      {
        translation += camera_.up();
      }
      camera_.translate(transSpeed * translation);

    }
   // Schedule a redraw
   this->makeCurrent();
   this->paintGL();
   QOpenGLWidget::update();
}

void MainViewWidget::teardownGL()
{
  // Actually destroy our OpenGL information
  delete p_program_;
}

void MainViewWidget::keyPressEvent(QKeyEvent *event)
{
  if (event->isAutoRepeat())
  {
    event->ignore();
  }
  else
  {
    Input::registerKeyPress(event->key());
  }
}

void MainViewWidget::keyReleaseEvent(QKeyEvent *event)
{
  if (event->isAutoRepeat())
  {
    event->ignore();
  }
  else
  {
    Input::registerKeyRelease(event->key());
  }
}

void MainViewWidget::mousePressEvent(QMouseEvent *event)
{
  Input::registerMousePress(event->button());
  lastPos = event->pos();
}

void MainViewWidget::mouseReleaseEvent(QMouseEvent *event)
{
  Input::registerMouseRelease(event->button());
}

void MainViewWidget::mouseMoveEvent(QMouseEvent *event)
{
   update();
//    int dx = event->x() - lastPos.x();
//    int dy = event->y() - lastPos.y();

//    if (event->buttons() & Qt::LeftButton) {
//        setXRotation(xRot + 0.5 * dy);
//        setYRotation(yRot + 0.5 * dx);
//    } else if (event->buttons() & Qt::RightButton) {
//        setXRotation(xRot + 0.5 * dy);
//        setZRotation(zRot + 0.5 * dx);
//    }

//    lastPos = event->pos();

    // Update instance information
    //earth_transform.rotate(1.0f, QVector3D(0.4f, 0.3f, 0.3f));
}


/*******************************************************************************
 * Private Helpers
 ******************************************************************************/

void MainViewWidget::printVersionInformation()
{
  QString glType;
  QString glVersion;
  QString glProfile;

  // Get Version Information
  glType = (context()->isOpenGLES()) ? "OpenGL ES" : "OpenGL";
  glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));

  // Get Profile Information
#define CASE(c) case QSurfaceFormat::c: glProfile = #c; break
  switch (format().profile())
  {
    CASE(NoProfile);
    CASE(CoreProfile);
    CASE(CompatibilityProfile);
  }
#undef CASE

  // qPrintable() will print our QString w/o quotes around it.
  qDebug() << qPrintable(glType) << qPrintable(glVersion) << "(" << qPrintable(glProfile) << ")";
}
