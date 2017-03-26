#include <QDebug>
#include <QString>
#include <QtOpenGL>
#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include "vertex.h"
#include "input.h"
#include "mainviewwidget.h"
#include "textureandlightingpch.h"
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

void MainViewWidget::initializeObjects(QVBoxLayout *layout, QOpenGLShaderProgram* shader,
                                       Textures* textures, Terrain* terrain, Road *road)
{
    // Create Shader (Do not release until VAO is created)
    m_TexturedDiffuseShaderProgram = shader;
    m_pTextures = textures;
    m_pTerrain = terrain;
    m_pRoad = road;

//    u_worldToCamera = m_TexturedDiffuseShaderProgram->uniformLocation("worldToCamera");
//    u_cameraToView = m_TexturedDiffuseShaderProgram->uniformLocation("cameraToView");

//    m_TexturedDiffuseShaderProgram->setUniformValue(u_worldToCamera, QMatrix4x4());
//    m_TexturedDiffuseShaderProgram->setUniformValue(u_cameraToView, QMatrix4x4());
//    m_TexturedDiffuseShaderProgram->release();

    //printVersionInformation();
}


QSize MainViewWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize MainViewWidget::sizeHint() const
{
    return QSize(800, 800);
}

void MainViewWidget::initializeGL()
{

    // Initialize OpenGL Backend
    QOpenGLFunctions::initializeOpenGLFunctions();
    connect(this, SIGNAL(frameSwapped()), this, SLOT(update()));
    connect(context(), SIGNAL(aboutToBeDestroyed()), this, SLOT(teardownGL()), Qt::DirectConnection);
    printVersionInformation();


    m_TexturedDiffuseShaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/texturedDiffuse.vert");
    m_TexturedDiffuseShaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/texturedDiffuse.frag");
    m_TexturedDiffuseShaderProgram->link();
    m_TexturedDiffuseShaderProgram->bind();

    // Cache Uniform Locations
    u_worldToCamera = m_TexturedDiffuseShaderProgram->uniformLocation("worldToCamera");
    u_cameraToView = m_TexturedDiffuseShaderProgram->uniformLocation("cameraToView");

//    if ( !m_pTextures->loadTexture( ":/Textures/earth.dds", EARTH_TEXTURE ) )
//    {
//        std::cerr << "Failed to load terrain texture for earth!" << std::endl;
//    }
//    if ( !m_pTextures->loadTexture( ":/Textures/moon.dds", MOON_TEXTURE ) )
//    {
//        std::cerr << "Failed to load terrain texture for moon!" << std::endl;
//    }

    if ( !m_pTextures->loadTexture( ":/Data/Textures/grass.jpg", GRASS_TEXTURE ) )
    {
        std::cerr << "Failed to load terrain texture for grass!" << std::endl;
    }
    if ( !m_pTextures->loadTexture( ":/Data/Textures/rock.png", ROCK_TEXTURE ) )
    {
        std::cerr << "Failed to load terrain texture for rock!" << std::endl;
    }
    if ( !m_pTextures->loadTexture( ":/Data/Textures/snow.jpg", SNOW_TEXTURE ) )
    {
        std::cerr << "Failed to load terrain texture for snow!" << std::endl;
    }
    if ( !m_pTextures->loadTexture( ":/Data/Textures/road.jpg", ROAD_TEXTURE ) )
    {
        std::cerr << "Failed to load terrain texture for road!" << std::endl;
    }

    if (m_pRoad != NULL)
    {
        if ( !m_pRoad->setupDefaultMesh() )
        {
            std::cerr << "Failed to load the road!" << std::endl;
        }
    }
    else
    {
        std::cerr << "Road object doesn't exist!" << std::endl;
    }

    if (m_pTerrain != NULL)
    {
        if ( !m_pTerrain->setupDefaultMesh() )
        {
            std::cerr << "Failed to load heightmap for terrain!" << std::endl;
        }
    }
    else
    {
        std::cerr << "Terrain object doesn't exist!" << std::endl;
    }

  m_pTerrain->setTexture(m_pTextures, ROCK_TEXTURE);
  m_pRoad->setTexture(m_pTextures, ROCK_TEXTURE);


  // Set global information
  //glEnable(GL_DEPTH_TEST | GL_CULL_FACE);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  m_pTerrain->setModels();
  m_pRoad->setModels();

  m_TexturedDiffuseShaderProgram->release();
}

void MainViewWidget::resizeGL(int width, int height)
{
    // Currently we are not handling width/height changes.
    (void)width;
    (void)height;
    m_projection.setToIdentity();
    m_projection.perspective(45.0f, width / float(height), 1.0f, 1000.0f);
}

void MainViewWidget::paintGL()
{
  // std::cout << "draw" << std::endl;
  // Clear
  glClear(GL_COLOR_BUFFER_BIT);
  glClear(GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  m_TexturedDiffuseShaderProgram->bind();
  m_TexturedDiffuseShaderProgram->setUniformValue(u_worldToCamera, m_camera.toMatrix());
  m_TexturedDiffuseShaderProgram->setUniformValue(u_cameraToView, m_projection);
  //m_TexturedDiffuseShaderProgram->setUniformValue(m_TexturedDiffuseShaderProgram->uniformLocation("modelToWorld"), QMatrix4x4());


  m_pRoad->setModels();
  m_pRoad->draw();
  m_pRoad->render();
  m_pTerrain->setModels();
  //m_pTerrain->render();
  m_pTerrain->draw();


  m_TexturedDiffuseShaderProgram->release();


}

void MainViewWidget::update()
{
    // Update input
    Input::update();

    // Camera Transformation
    if (Input::buttonPressed(Qt::RightButton))
    {
      static const float transSpeed = 0.5f;
      static const float rotSpeed   = 0.5f;

      // Handle rotations
      m_camera.rotate(-rotSpeed * Input::mouseDelta().x(), Camera3D::LocalUp);
      m_camera.rotate(-rotSpeed * Input::mouseDelta().y(), m_camera.right());

      // Handle translations
      QVector3D translation;
      if (Input::keyPressed(Qt::Key_W))
      {
        translation += m_camera.forward();
      }
      if (Input::keyPressed(Qt::Key_S))
      {
        translation -= m_camera.forward();
      }
      if (Input::keyPressed(Qt::Key_A))
      {
        translation -= m_camera.right();
      }
      if (Input::keyPressed(Qt::Key_D))
      {
        translation += m_camera.right();
      }
      if (Input::keyPressed(Qt::Key_Q))
      {
        translation -= m_camera.up();
      }
      if (Input::keyPressed(Qt::Key_E))
      {
        translation += m_camera.up();
      }
      m_camera.translate(transSpeed * translation);

    }
  // Schedule a redraw
   //std::cout << "TTEST" << std::endl;
   this->makeCurrent();
   this->paintGL();
  QOpenGLWidget::update();
}
void MainViewWidget::mainUpdate()
{
}

void MainViewWidget::teardownGL()
{
  // Actually destroy our OpenGL information
  delete m_TexturedDiffuseShaderProgram;
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
