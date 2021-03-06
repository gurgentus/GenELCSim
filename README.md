# Generic Estimation Learning Control Simulator

Simulator for testing state estimation (Kalman Filter, Extended Kalman Filter, Unscented Kalman Filter, etc), Machine Learning (classical, deep learning, etc), and Control (classical, optimal, robust, adaptive)  algorithms on general models.  For speed and flexibility rather than using a game engine like Unity or Unreal, I decided to directly use modern OpenGL with OpenGL Shading Language with a light Qt wrapper.  The GUI is also done in Qt for platform independence.


# TODO

 - [X] Basic Qt GUI setup
 - [X] Basic modular OOP framework setup with OpenGL wrappers for 3D graphics
 - [X] Basic car module with PID control algorithm and adaptive cruise control
 - [X] More realistic car model with steering
 - [X] Satellite orbit determination and restricted three body (Moon mission) orbital mechanics simulations
 - [X] Probabilistic filter module (Kalman, Extended Kalman, Unscented Kalman)
 - [X] Connect probabilistic filter module to orbit determination module (with the state consisting of position and velocity of the satellite, positions of three ground tracking stations, as well as mu, J2, and drag coefficient for estimating higher order perturbation effects). [Derivations Link](./Docs/derivations.pdf)
 - [X] Add sequential estimation, batch estimation with same epoch measurement for each batch
 - [X] Add uncertainty ellipse visualization
 - [X] Use QWebSockets and separate plugins to better separate ground truth measurement simulation from state estimation.
 - [ ] Test running the two processes on different machines and sending telemetry data through TCP connection.
 - [ ] Implement multiple epoch measurements in each batch
 - [X] Write a two point boundary value problem solver for nonlinear systems
 - [ ] Optimal control: add minimal time orbit transfer solver using maximum principle and boundary problem solver
 - [ ] Testing
 - [ ] Controls toolbox with basic algorithms (mixed C++ and calls to Python libraries, eventually all C++ for performance improvement)
 - [ ] Add path planning simulations for the car module
 - [ ] Testing, bug fixes
 - [ ] Optimize code design for modularity and OOP for easy extensions to other models and simulations
 - [ ] Add a tutorial describing how to add new modules
 - [ ] Add full 6DF module

[//]: # (Image References)

 [GUI]: ./Docs/SolverDefinition.png "Solver"
 [Car]: ./Docs/Car.png "Car"
 [Orbit]: ./Docs/sat.png "Orbit"
 [Sat]: ./Docs/sat2.png "Sat"
 [R3B]: ./Docs/Restricted3Body.png "Restricted 3 Body"
 [video1]: ./Docs/orbit.mov "Satelite Orbit Video"
 [deriv]: ./Docs/derivations.pdf "Derivations"
#### Restricted 3 body problem - lunar mission

![R3B][R3B]

#### Satellite Tracking

##### Preliminary Orbit Determination

![Orbit][Orbit]

##### Statistical Orbit Determination

![StOrbit][Sat]

#### Adaptive Cruise Control

![Car][Car]

#### Example of Solver Code

![GUI][GUI]

# License

GNU LGPL license.

# References

I used several very helpful online tutorials in the initial version of the simulator.

I learned how to do OpenGL in Qt with the following tutorial (Input and Camera classes were also adapted from here)
http://www.trentreed.net/blog/qt5-opengl-part-1-basic-rendering/

Terrain class is based on the following tutorial which I adapted to Qt framework for basic terrain loading:
https://www.3dgep.com/multi-textured-terrain-in-opengl/

I learned about the OBJ files for 3D models and a basic loader from the Modern OpenGL tutorial at:
http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/

Initial testing car meshes were loaded based on car obj file in:
http://people.sc.fsu.edu/~jburkardt/data/obj/obj.html
