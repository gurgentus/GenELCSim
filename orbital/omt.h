#ifndef OMT_H
#define OMT_H

#include <tuple> // C++11, for std::tie
#include <QVector3D>
#include <QMatrix3x3>
#include "Eigen/Dense"

using Eigen::MatrixXd;
using Eigen::VectorXd;

class Omt
{
public:
    Omt();

    QVector3D e_vec;
    double e; // eccentricity
    QVector3D h_vec;
    double h; // specific angular momentum
    double i; // orbit inclination
    double Omega; // right ascension of the ascending node
    double omega; // argument of perigee
    double a;   // major semi-axis
    double alpha; // reciprical of major semi-axis
    double r_p;  // perigee
    double r_a;  // apogee
    double period; // period of the orbit
    double mu;

    int orbit_desc(const QVector3D& r, const QVector3D& v, const double mu);
    int orbit_desc(const double h, const double e, const double i,
                   const double Omega, const double omega, const double mu);
    int orbit_desc_apog(const double r_p, const double r_a, const double i, const double Omega,
                   const double omega, const double mu);
    int secondary_params();

    void perifocal_to_geocentric(Eigen::Matrix3f &transfs);

    void change_coords(Eigen::Vector3f& geocentric_v, const Eigen::Vector3f& perifocal_v);
    int sat_long_lat(double& longitude, double& latitude, const double theta, const double dt);
    static std::pair<double, double> right_ascension_and_declination(Eigen::Vector3f &r);
    static int e_anom_kepler(double& E, const double e, const double M_e);
    static int u_anom_kepler(double& chi, double& C, double& S, double&z,
                             const double dt, const double r0, const double vr0, const double alpha, const double mu);
    static int state_transition(QVector3D& r, QVector3D& v, const double dt, const double mu);

    static double stumpffS(double z);
    static double stumpffC(double z);

    static constexpr double ERR_TOL = 1e-8;
    static const int MAX_ITER = 1000;
};

#endif // OMT_H