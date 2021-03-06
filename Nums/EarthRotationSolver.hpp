#ifndef EARTHROTATIONSOLVER_H
#define EARTHROTATIONSOLVER_H

#include "AbstractOdeSolver.hpp"
#include "Orbital/Omt.hpp"

class EarthRotationSolver: public AbstractOdeSolver
{
public:
    // orbital mechanics toolbox
    Omt omt;
    // define initial conditions and the dynamics equation
    void InitialConditions();
    void UpdateState(double dt);
    void SolveEquation(std::vector<double> yi);

    // outputs from the simulation
    double rotation();
private:
    double rot = 0;
};

#endif // EARTHROTATIONSOLVER_H



