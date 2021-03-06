/* Orbital Mechanics Toolbox */
#include "Omt.hpp"
#include <QVector3D>
#include <QMatrix3x3>

#include <cmath>
#include <iostream>

Omt::Omt()
{

}

/* This function calculates the right ascension and the
  declination from the position vector in the
  geocentric equatorial coordinae system.
*/
std::pair<double, double> Omt::right_ascension_and_declination(Eigen::Vector3d& r)
{
    double norm = r.norm();
    double l = r(0)/norm;
    double m = r(1)/norm;
    double n = r(2)/norm;

    double delta = asin(n);
    double alpha;
    if (m > 0) {
        alpha = acos(l/cos(delta));
    }
    else {
        alpha = 2*M_PI - acos(l/cos(delta));
    }
    return std::make_pair(alpha, delta);
}

/* Stumpff functions for use with universal variables */
double Omt::stumpffS(double z)
{
    double sq = sqrt(std::abs(z));
    if (z > 0) return (sq-sin(sq))/(sq*sq*sq);
    if (z < 0) return (sinh(sq)-sq)/(sq*sq*sq);
    return (1.0/6.0);
}

double Omt::stumpffC(double z)
{
    if (z > 0) return (1-cos(sqrt(z)))/z;
    if (z < 0) return (cosh(sqrt(-z))-1)/(-z);
    return 0.5;
}

/* Returns the eccentric anomaly E from the eccentricity, e, and the mean anomaly M_e
 * by solving Kepler's equation using Newton's method
 *
 * Error codes:
 * 0 - normal execution
 * 1 - maximum number of iterations exceeded
 *
 */
int Omt::e_anom_kepler(double& E, const double e, const double M_e) {
    // initial estimate, Prussing and Conway, 1993
    if (M_e < M_PI) {
        E = M_e + e/2;
    }
    else {
        E = M_e - e/2;
    }
    double f, fprime, rat;
    unsigned int iter;
    for (iter = 0; iter < MAX_ITER; iter++) {
        f = E - e*sin(E) - M_e;
        fprime = 1 - e*cos(E);
        rat = f/fprime;
        if (std::abs(rat) < ERR_TOL) {
            break;
        }
        E = E - rat;
    }
    if (iter == MAX_ITER) {
        return 1;
    }
    return 0;
}

/* Returns the universal anomaly corresponding to the position after time dt
 * from the initial position given by position r0 and velocity vr0
 *
 * Error codes:
 * 0 - normal execution
 * 1 - maximum number of iterations exceeded
 *
 * Parameters:
 *
 * chi   - universal anomaly (km^0.5)
 * dt    - time increment (s)
 * r0    - initial radial position (km)
 * vr0   - initial radial velocity (km/s)
 * alpha - reciprocal of semimajor axis (1/km)
 * mu    - gravitational parameter (km^3/s^2)
 *
 */
int Omt::u_anom_kepler(double& chi, double& C, double& S, double&z,
                       const double dt, const double r0, const double vr0, const double alpha, const double mu)
{
    // will use Newton's method to solve the universal Kepler Equation
    // use Chobotov (2002) for initial guess of the universal anomality
    double sqmu = sqrt(mu);
    chi = sqmu*std::abs(alpha)*dt;

    double f, fprime, rat, chisq;
    unsigned int iter;
    for (iter = 0; iter < MAX_ITER; iter++) {
        chisq = chi*chi;
        z = alpha*chisq;
        C = stumpffC(z);
        S = stumpffS(z);

        f = (r0*vr0/sqmu)*chisq*C + (1-alpha*r0)*chi*chisq*S + r0*chi - sqmu*dt;
        fprime = (r0*vr0/sqmu)*chi*(1-alpha*chisq*S) + (1-alpha*r0)*chisq*C + r0;
        rat = f/fprime;
        if (std::abs(rat) < ERR_TOL) {
            break;
        }
        chi = chi - rat;
        // std::cout << f << ", " << fprime << ", " << rat << ", " << chi << std::endl;
    }
    if (iter == MAX_ITER) {
        return 1;
    }
    return 0;
}

/* Returns the state (position and velocity) after time dt
 * from the initial position given by position r0 and velocity vr0
 *
 * Error codes:
 * 0 - normal execution
 * 1 - maximum number of iterations exceeded
 *
 * Parameters:
 *
 * r    - position vector (km)
 * v    - velocity vector (km/s)
 * dt   - elapsed time
 * mu   - gravitational parameter (km^3/s^2)
 *
 */
int Omt::state_transition(QVector3D& r, QVector3D& v, const double dt, const double mu)
{
    double r_scalar = r.length();
    double v_scalar = v.length();
    double v_r = QVector3D::dotProduct(r,v)/r_scalar;
    double chi, C, S, z;
    // double a = h*h/(mu*(1-e*e));
    // from energy equation:  vel*vel/2 - mu/r_scalar = -mu/2a, reciprocal of semimajor axis is
    double alpha = 2/r_scalar - v_scalar*v_scalar/mu;
    u_anom_kepler(chi, C, S, z, dt, r_scalar, v_r, alpha, mu);
    double chisq = chi*chi;
    double chicube = chisq*chi;
    // Lagrange coefficients
    double sqmu = sqrt(mu);
    double f = 1 - (chisq/r_scalar) * C;
    double g = dt - (1/sqmu)*chicube * S;
    QVector3D r1 = f*r + g*v;
    double r1_scalar = r1.length();
    double fprime = sqmu/(r_scalar*r1_scalar)*(alpha*chicube*S-chi);
    double gprime = 1 - chisq*C/r1_scalar;
    v = fprime*r + gprime*v;
    r = r1;
    return 0;
}

/* Generates orbital parameters from the state vector given by the position, r, and
 * the velocity, v.
 */
int Omt::orbit_desc(const QVector3D& r, const QVector3D& v, const double mu)
{
    Eigen::Vector3d r2, v2;
    r2 << r.x(), r.y(), r.z();
    v2 << v.x(), v.y(), v.z();
    orbit_desc(r2, v2, mu);
    return 0;
}

int Omt::orbit_desc(const Eigen::Vector3d& r, const Eigen::Vector3d& v, const double mu)
{
    this->mu = mu;
    double r_scalar = r.norm();
    double speed = v.norm();
    double v_r = r.dot(v)/r_scalar;
    h_vec = r.cross(v);
    h = h_vec.norm();
    // inclination
    i = acos(h_vec.z()/h);
    // node line
    Eigen::Vector3d un;
    un << 0,0,1;
    Eigen::Vector3d N;
    N= un.cross(h_vec);
    double N_len = N.norm();
    // right ascension of the ascending node
    Omega = acos(N(0)/N_len);
    if (N(1) < 0) {
        Omega = 2*M_PI - Omega;
    }
    // eccentricity vector
    e_vec = (1/mu)*((speed*speed - mu/r_scalar)*r - r_scalar*v_r*v);
    e = e_vec.norm();
    // argument of perigree
    omega = acos(N.dot(e_vec)/(N_len*e));
    if (e_vec(2) < 0) {
        omega = 2*M_PI - omega;
    }
    // true anomality
//    theta = acos(QVector3D::dotProduct(e_vec/e, r/r_scalar));
//    if (v_r < 0) {
//        theta = 2*M_PI - theta;
//    }
    secondary_params();
    return 0;
}


/* Generates orbital parameters from three position vectors, r1, r2, and r1 using
 * Gibbs method.
 */
int Omt::orbit_desc(const QVector3D& r1, const QVector3D& r2, const QVector3D& r3, const double mu)
{
    this->mu = mu;
    Eigen::Vector3d r,v;
    Omt::orbit_desc(r,v,r1,r2,r3,mu);
    orbit_desc(QVector3D(r(0),r(1),r(2)), QVector3D(v(0),v(1),v(2)), mu);
    return 0;
}

int Omt::orbit_desc(Eigen::Vector3d& r, Eigen::Vector3d& v, const QVector3D& r1, const QVector3D& r2, const QVector3D &r3, const double mu)
{
    double r1_norm = r1.length();
    double r2_norm = r2.length();
    double r3_norm = r3.length();
    QVector3D C12 = QVector3D::crossProduct(r1, r2);
    QVector3D C23 = QVector3D::crossProduct(r2, r3);
    QVector3D C31 = QVector3D::crossProduct(r3, r1);
    QVector3D N = r1_norm*C23+r2_norm*C31+r3_norm*C12;
    QVector3D D = C12 + C23 + C31;
    QVector3D S = (r2_norm-r3_norm)*r1+(r3_norm-r1_norm)*r2+(r1_norm-r2_norm)*r3;
    QVector3D v2 = sqrt(mu/(N.length()*D.length()))*(QVector3D::crossProduct(D,r2)/r2_norm+S);
    v << v2.x(), v2.y(), v2.z();
    r << r2.x(), r2.y(), r2.z();

    return 0;
}

int Omt::orbit_desc(Eigen::Vector3d& r, Eigen::Vector3d& v, const QVector3D& r1,
                    const QVector3D& r2, double dt, bool prograde, const double mu)
{
    double r1_norm = r1.length();
    double r2_norm = r2.length();
    QVector3D C12 = QVector3D::crossProduct(r1, r2);
    double D12 = QVector3D::dotProduct(r1, r2);
    double P12 = r1_norm*r2_norm;
    double invcos = acos(D12/P12);
    double dtheta;
    if (prograde)
    {
        if (C12.z() >= 0)
        {
            dtheta = invcos;
        }
        else
        {
            dtheta = 2*M_PI-invcos;
        }
    }
    else
    {
        if (C12.z() < 0)
        {
            dtheta = invcos;
        }
        else
        {
            dtheta = 2*M_PI-invcos;
        }
    }
    double A = sin(dtheta)*sqrt(P12/(1-cos(dtheta)));

    double sqmu = sqrt(mu);

    double f, y, fprime, rat, pre, z, C, S;
    unsigned int iter;
    z = 0;
    for (iter = 0; iter < MAX_ITER; iter++) {

        C = stumpffC(z);
        S = stumpffS(z);
        pre = pow(y/C,1.5);
        y = r1_norm + r2_norm + A*(z*S-1)/sqrt(C);
        f = pre*S+A*sqrt(y)-sqmu*dt;
        if (fabs(z) > ERR_TOL)
        {
            fprime = pre*((1/(2*z))*(C-1.5*S/C)+0.75*S*S/C)+(A/8)*(3*S/C*sqrt(y)+A*sqrt(C/y));
        }
        else
        {
            fprime = (sqrt(2)/40)*pow(y,1.5)+(A/8)*(sqrt(y)+A*sqrt(0.5/y));
        }
        rat = f/fprime;
        if (std::abs(rat) < ERR_TOL) {
            break;
        }
        z = z - rat;
        // std::cout << f << ", " << fprime << ", " << rat << ", " << chi << std::endl;
    }
    if (iter == MAX_ITER) {
        return 1;
    }

    double lf = 1 - y/r1_norm;
    double lg = A*sqrt(y/mu);
    //double lfprime = (sqrt(mu)/P12)*sqrt(y/C)*(z*S-1);
    //double lgprime = 1 - y/r2_norm;
    QVector3D v1 = (1/lg)*(r2-lf*r1);
    // QVector3D v2 = lgprime*r2/lg-(lf*lgprime-lfprime*lg)*r1/lg;

    v << v1.x(), v1.y(), v1.z();
    r << r1.x(), r1.y(), r1.z();

    return 0;
}

int Omt::secondary_params()
{
    r_p = h*h/(mu*(1+e));
    r_a = h*h/(mu*(1-e));
    a = (r_p+r_a)/2;
    period = 2*M_PI*pow(a,1.5)/sqrt(mu);
    return 0;
}

int Omt::orbit_desc(const double h, const double e, const double i, const double Omega, const double omega, const double mu)
{
    this->mu = mu;
    this->h = h;
    this->e = e;
    this->i = i;
    this->Omega = Omega;
    this->omega = omega;
    secondary_params();
    return 0;
}

int Omt::orbit_desc_apog(const double r_p, const double r_a, const double i, const double Omega, const double omega, const double mu)
{
    this->mu = mu;
    a = (r_p + r_a)/2;
    e = (r_a - r_p)/(r_a + r_p);
    h = sqrt(mu*r_p*(1+e));
    this->i = i;
    this->Omega = Omega;
    this->omega = omega;
    this->r_p = r_p;
    this->r_a = r_a;
    period = 2*M_PI*pow(a,1.5)/sqrt(mu);
    return 0;
}

void Omt::perifocal_to_geocentric (Eigen::Matrix3d &transf) {
    double sO = sin(Omega);
    double cO = cos(Omega);
    double so = sin(omega);
    double co = cos(omega);
    double si = sin(i);
    double ci = cos(i);

    transf << -sO*ci*so+cO*co, -sO*ci*co-cO*so, sO*si,
        cO*ci*so+sO*co, cO*ci*co-sO*so, -cO*si,
        si*so, si*co, ci;
}

void Omt::change_coords(Eigen::Vector3d &geocentric_v, const Eigen::Vector3d &perifocal_v) {
    Eigen::Matrix3d transf;
    perifocal_to_geocentric(transf);
    geocentric_v = transf*perifocal_v;
}



/* calculates the right ascension (longitude) and declination (latitude) relative to the rotating
 * planet dt seconds after the current time.  Note: the state vector is not updated.
 */
int Omt::sat_long_lat(double& longitude, double& latitude, const double theta0, const double dt) {
    // calculate eccentric anomaly from the true anomaly
    double E = 2*atan(tan(theta0/2)*sqrt((1-e)/(1+e)));
    // mean anomaly
    double M = E - e * sin(E);
    // update time from perigee
    double t = M*period/(2*M_PI) + dt;
    // recalculate the mean anomaly for the new time
    M = 2*M_PI*t/period;
    // calculate the eccentric anomaly for the new time
    e_anom_kepler(E, e, M);
    // calculate the true anomaly
    double theta = 2*atan(tan(E/2)*sqrt((1+e)/(1-e)));

    // calculate angular velocities due to earth's oblateness
    double pre = -1.5*sqrt(mu)*(1.08263e-3)*6378*6378/((1-e*e)*(1-e*e)*pow(a, 3.5));
    double Omega_prime = pre*cos(i);
    double omega_prime = pre*(2.5*sin(i)*sin(i)-2);
    // update orbit orientation
    Omega = Omega + Omega_prime * dt;
    omega = omega + omega_prime * dt;
    Eigen::Vector3d r, v, rX;
    // position in perifocal frame
    r << cos(theta), sin(theta), 0;
    r = ((h*h)/(mu*(1+e*cos(theta))))*r;

    Eigen::Vector3d Rx;
    // position in geocentric equatorial frame
    change_coords(Rx, r);

    Eigen::Matrix3d rot;
    // angle between stationary and rotating x axes
    double th = 2*dt*M_PI*(1+1/365.26)/(24*3600);
    // rotation matrix between stationary and rotating equatorial frames
    rot << cos(th), sin(th), 0,
            -sin(th),cos(th), 0,
            0,0,1;
    // position in the rotating equatorial frame
    Rx = rot*Rx;
    // longitude and latitude
    std::tie(longitude,latitude) =  right_ascension_and_declination(Rx);
    return 0;
}

// orbital maneuvers
/* Given two spacecraft with positions and velocities, r_i and v_i, respectively calculates
 * the relative position, velocity, and acceleration of the second spacecraft
 * relative to the first along the local vertical/local horizontal frame of the first craft.
 */
int Omt::target_rel_state(Eigen::Vector3d& r_rel, Eigen::Vector3d& v_rel, Eigen::Vector3d& a_rel,
                          Eigen::Vector3d& r_1, Eigen::Vector3d& v_1, Eigen::Vector3d& r_2, Eigen::Vector3d& v_2, double mu)
{
    double r_1norm = r_1.norm();
    double r_2norm = r_2.norm();
    double r_1normsq = r_1norm*r_1norm;
    Eigen::Vector3d h_1 = r_1.cross(v_1);
    Eigen::Vector3d i = r_1/r_1norm;
    Eigen::Vector3d k = h_1/h_1.norm();
    Eigen::Vector3d j = k.cross(i);
    Eigen::Matrix3d Q;
    Q << i(0), i(1), i(2),
         j(0), j(1), j(2),
         k(0), k(1), k(2);

    Eigen::Vector3d Omega = h_1/(r_1normsq);
    Eigen::Vector3d OmegaPrime = -2*v_1.dot(r_1)*Omega/(r_1normsq);
    Eigen::Vector3d a_1 = -mu*r_1/(r_1normsq*r_1norm);
    Eigen::Vector3d a_2 = -mu*r_2/(r_2norm*r_2norm*r_2norm);
    r_rel = r_2 - r_1;
    v_rel = v_2 - v_1 - Omega.cross(r_rel);
    a_rel = a_2 - a_1 - OmegaPrime.cross(r_rel) - Omega.cross(Omega.cross(r_rel)) - 2*Omega.cross(v_rel);
    r_rel = Q*r_rel;
    v_rel = Q*v_rel;
    a_rel = Q*a_rel;
    return 0;
}

// delta v required for plane change maneuver
double Omt::delv(double v1, double v2, double i)
{
    return sqrt(v1*v1+v2*v2-2*v1*v2*cos(i));
}

// convert from inertial to satellite fixed frames
int Omt::oblateness_force_perturbation(Eigen::Vector3d& f, Eigen::Vector3d pos, Eigen::Vector3d vel, double J_2)
{
    double r_norm = pos.norm();
    double r_normsq = r_norm*r_norm;
    double theta = acos(e_vec.dot(pos)/(e,r_norm));
    double R_e = 6378;
    Eigen::Vector3d u_r = pos/pos.norm();
    Eigen::Vector3d h_hat = h_vec/h_vec.norm();
    Eigen::Vector3d u_perp = h_hat.cross(u_r);
    double sinsq = sin(i)*sin(i);
    double sinot = sin(omega+theta);
    double pre = -(mu/r_normsq)*1.5*(R_e*R_e/r_normsq)*J_2;
    double p_r = pre*(1-3*sinsq*sinot*sinot);
    double p_perp = pre*sinsq*sin(2*(omega+theta));
    double p_h = pre*sin(2*i)*sinot;
    f = p_r*u_r + p_perp*u_perp + p_h*h_hat;
    return 0;
}
