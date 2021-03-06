#ifndef SATELLITESOLVER_H
#define SATELLITESOLVER_H

#include "Common/Output.hpp"
#include "Orbital/Omt.hpp"
#include "Eigen/Dense"

#include <QVector3D>
#include <QtCharts>

#include "RungeKuttaSolver.hpp"

using namespace std;

class SatelliteSolver : public RungeKuttaSolver
{
public:
    // orbital mechanics toolbox
    Omt omt;
    // define initial conditions and the dynamics equation
    void InitialConditions();
    void InitialConditions(Eigen::VectorXd& x, double dt);
    void RightHandSide(double t, const std::vector<double> &  y, std::vector<double> &  f);
    // outputs from the simulation
    QVector3D position();
    QVector3D velocity();

    double eccentricity();
};

#endif // SATELLITESOLVER_H
