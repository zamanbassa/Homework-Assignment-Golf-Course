#include "Shape3D.h"

Shape3D::Shape3D() {}

Shape3D::Shape3D(Vector<4> colour) {}

vector<float> Shape3D::getVertices() const {}

vector<unsigned int> Shape3D::getIndices() const {}

vector<float> Shape3D::getColours() const {}

vector<float> Shape3D::getNormals() const {}

vector<float> Shape3D::getTextureCoords() const {}

void Shape3D::setVertices(vector<float> vertices) {}

void Shape3D::setNormals(vector<float> normals) {}

void Shape3D::setIndices(vector<unsigned int> indices) {}

void Shape3D::setTextureCoords(vector<float> texture) {}

void Shape3D::setColour(Vector<4> colour) {}