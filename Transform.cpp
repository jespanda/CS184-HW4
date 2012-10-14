// Transform.cpp: implementation of the Transform class.


#include "Transform.h"


// Helper rotation function.  Please implement this.  
mat3 Transform::rotate(const float degrees, const vec3& axis) {
  // YOUR CODE FOR HW2 HERE
  // Please implement this.  Likely the same as in HW 1.
  vec3 u = glm::normalize(axis);
  float x = u[0]; float y = u[1]; float z = u[2];
  float radians = degrees * (pi/180);
  float cosRad = cos(radians);
  float sinRad = sin(radians);
  vec3 m0 = vec3(cosRad + x * x * (1-cosRad), y*x*(1-cosRad) + z*sinRad, z*x*(1-cosRad)-y*sinRad);
  vec3 m1 = vec3(x*y*(1-cosRad)-z*sinRad, cosRad+y*y*(1-cosRad), z*y*(1-cosRad)+x*sinRad);
  vec3 m2 = vec3(x*z*(1-cosRad) + y*sinRad, y*z*(1-cosRad) - x*sinRad, cosRad+z*z*(1-cosRad));
  mat3 rotationMatrix = mat3(m0, m1, m2);
    return rotationMatrix;
}

void Transform::left(float degrees, vec3& eye, vec3& up) {
  // YOUR CODE FOR HW2 HERE
  // Likely the same as in HW 1.  
  mat3 rotationMatrix = rotate(degrees, up);
  eye = rotationMatrix * eye;
  up = rotationMatrix * up;
}

void Transform::up(float degrees, vec3& eye, vec3& up) {
  // YOUR CODE FOR HW2 HERE 
  // Likely the same as in HW 1. 
  vec3 crossProduct = glm::cross(eye, up);
  mat3 rotationMatrix = rotate(degrees, crossProduct);
  eye = rotationMatrix * eye;
  up = rotationMatrix * up; 
}

mat4 Transform::lookAt(const vec3 &eye, const vec3 &center, const vec3 &up) {
  // YOUR CODE FOR HW2 HERE
  // Likely the same as in HW 1.  
  vec3 w = glm::normalize(eye - center);
  vec3 u = glm::normalize(glm::cross(up, w));
  vec3 v = glm::cross(w, u);
  vec4 m0 = vec4(u.x, v.x, w.x, 0);
  vec4 m1 = vec4(u.y, v.y, w.y, 0);
  vec4 m2 = vec4(u.z, v.z, w.z, 0);
  vec4 m3 = vec4(-u.x*eye.x-u.y*eye.y-u.z*eye.z,
                 -v.x*eye.x-v.y*eye.y-v.z*eye.z,
                 -w.x*eye.x-w.y*eye.y-w.z*eye.z,
                 1);
  mat4 lookAtMatrix = mat4(m0, m1, m2, m3);
    return glm::transpose(lookAtMatrix);
}

mat4 Transform::perspective(float fovy, float aspect, float zNear, float zFar)
{
  // YOUR CODE FOR HW2 HERE
  // New, to implement the perspective transform as well.
  float radians = fovy * (pi/180);
    float d = 1/tan(radians/2);
  float A = -(zFar + zNear)/(zFar - zNear);
  float B = -(2*zFar * zNear)/(zFar - zNear);
  mat4 perspectiveMatrix = mat4(d/aspect, 0,  0, 0,
                                0,        d,  0, 0,
                                0,        0,  A, B,
                                0,        0, -1, 0
                                );
    return perspectiveMatrix;
}

mat4 Transform::scale(const float &sx, const float &sy, const float &sz) {
  // YOUR CODE FOR HW2 HERE
  // Implement scaling 
  mat4 scaleMatrix = mat4(sx, 0, 0, 0,
                          0, sy, 0, 0,
                          0, 0, sz, 0, 
                          0, 0,  0, 1
                          );
  return scaleMatrix;
}

mat4 Transform::translate(const float &tx, const float &ty, const float &tz) {
  // YOUR CODE FOR HW2 HERE
  // Implement translation 

  mat4 translateMatrix = mat4(1, 0, 0, tx,
                              0, 1, 0, ty,
                              0, 0, 1, tz,
                              0, 0, 0, 1  
                              );
    return glm::transpose(translateMatrix);
  
}

// To normalize the up direction and construct a coordinate frame.  
// As discussed in the lecture.  May be relevant to create a properly 
// orthogonal and normalized up. 
// This function is provided as a helper, in case you want to use it. 
// Using this function (in readfile.cpp or display.cpp) is optional.  

vec3 Transform::upvector(const vec3 &up, const vec3 & zvec) {
  vec3 x = glm::cross(up,zvec) ; 
  vec3 y = glm::cross(zvec,x) ; 
  vec3 ret = glm::normalize(y) ; 
  return ret ; 
}


Transform::Transform()
{

}

Transform::~Transform()
{

}
