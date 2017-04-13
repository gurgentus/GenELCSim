#ifndef MAINVIEWWIDGET_H
#define MAINVIEWWIDGET_H

#include <QOpenGLWidget>
#include <QScrollArea>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QMatrix4x4>
#include "camera3d.h"
#include "transform3d.h"
#include "terrain.h"
#include "road.h"
#include "car.h"
#include "adaptivecar.h"
#include "textures.h"

class QOpenGLShaderProgram;

class MainViewWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit MainViewWidget(QWidget *parent = 0);
    ~MainViewWidget();

    public:
      void InitializeObjects(QVBoxLayout *layout, QVBoxLayout *control_layout, QOpenGLShaderProgram* shader,
                             Textures *textures, Terrain* terrain, Road* road, AdaptiveCar *car, Car *leadCar);
      // inherited QOpenGLWidget methods
      void initializeGL();
      void resizeGL(int width, int height);
      void paintGL();

      Camera3D camera_;

    protected:
      QSize minimumSizeHint() const;
      QSize sizeHint() const;

      // handle events
      void keyPressEvent(QKeyEvent *event);
      void keyReleaseEvent(QKeyEvent *event);
      void mousePressEvent(QMouseEvent *event);
      void mouseReleaseEvent(QMouseEvent *event);
      void mouseMoveEvent(QMouseEvent *event);

    protected Q_SLOTS:
      void update();
      void teardownGL();

//    public Q_SLOTS:
//        // slots for xyz-rotation slider
//        void setXRotation(int angle);
//        void setYRotation(int angle);
//        void setZRotation(int angle);

    Q_SIGNALS:
        // signaling rotation from mouse movement
        void xRotationChanged(int angle);
        void yRotationChanged(int angle);
        void zRotationChanged(int angle);

    private:
        // OpenGL State Information
        int world_to_camera_;
        int camera_to_view_;

        // shader program and camera info
        QOpenGLShaderProgram* p_program_;
        QMatrix4x4 projection_;

        // various objects
        Textures* p_textures_;
        Terrain* p_terrain_;
        Road* p_road_;
        AdaptiveCar* p_car_;
        Car* p_lead_car_;

        // print openGL info
        void printVersionInformation();

        QPoint lastPos;
        vector<QVector3D> results;
        int it;

        constexpr static const float pi = 3.1415926535897932384626433832795f;
        constexpr static const float _2pi = 2.0f * pi;

};

#endif // MAINVIEWWIDGET_H
