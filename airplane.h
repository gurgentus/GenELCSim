#ifndef AIRPLANE_H
#define AIRPLANE_H

#include "mesh.h"

class Airplane : public Mesh
{
public:
    Airplane();
    bool setupDefaultMesh();
private:
    void setupCapMesh();
    void setupSphereMesh();
    void setupFuselageMesh();
};

#endif // AIRPLANE_H
