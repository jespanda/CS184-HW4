/*****************************************************************************/
/* This is the program skeleton for homework 2 in CS 184 by Ravi Ramamoorthi */
/* Extends HW 1 to deal with shading, more transforms and multiple objects   */
/*****************************************************************************/

// This file is display.cpp.  It includes the skeleton for the display routine

// Basic includes to get this file to work.  
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <deque>
#include <stack>
#include <GL/glut.h>
#include <vector>
#include "Transform.h"
using namespace std ; 
#include "variables.h"
#include "readfile.h"

// objLoader (from wiki)

void drawTriangle (vec4 a, vec4 b, vec4 c){
    glPushMatrix();
    glBegin(GL_TRIANGLES);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(a.x, a.y, a.z);
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(b.x, b.y, b.z);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(c.x, c.y, c.z);
    glEnd();
    glPopMatrix();
}

void draw_obj(vector<glm::vec4> vertices, vector<GLushort>elements){
    for (int i=0; i<elements.size(); i+=3){
        GLushort a = elements[i];
        GLushort b = elements[i+1];
        GLushort c = elements[i+2];
        
        //std::cout << "elements size " << elements.size() << "\n";
        //std::cout << "faces " << a << " " << b << " " << c  << "\n";
        //std::cout << "vertices a " << vertices[a][0] << " " << vertices[a][1] << " " << vertices[a][2] << "\n ";
        //std::cout << "vertices b " << vertices[b][0] << " " << vertices[b][1] << " " << vertices[b][2] << "\n ";
        //std::cout << "vertices c " << vertices[c][0] << " " << vertices[c][1] << " " << vertices[c][2] << "\n ";

        vec4 vert_a = vertices[a];
        vec4 vert_b = vertices[b];
        vec4 vert_c = vertices[c];
        drawTriangle(vert_a, vert_b, vert_c);
    }
    //glFlush();
}

// New helper transformation function to transform vector by modelview
// May be better done using newer glm functionality.
// Provided for your convenience.  Use is optional.  
// Some of you may want to use the more modern routines in readfile.cpp 
// that can also be used.  

void transformvec (const GLfloat input[4], GLfloat output[4]) {
  GLfloat modelview[16] ; // in column major order
  glGetFloatv(GL_MODELVIEW_MATRIX, modelview) ; 
  
  for (int i = 0 ; i < 4 ; i++) {
    output[i] = 0 ; 
    for (int j = 0 ; j < 4 ; j++) 
      output[i] += modelview[4*j+i] * input[j] ; 
  }
}

void display() {
	glClearColor(1, 1, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        
        // I'm including the basic matrix setup for model view to 
        // give some sense of how this works.  

	glMatrixMode(GL_MODELVIEW);
	mat4 mv ; 

        if (useGlu) mv = glm::lookAt(eye,center,up) ; 
	else {
          mv = Transform::lookAt(eye,center,up) ; 
          mv = glm::transpose(mv) ; // accounting for row major
        }
        glLoadMatrixf(&mv[0][0]) ; 

        // Set Light and Material properties for the teapot
        // Lights are transformed by current modelview matrix. 
        // The shader can't do this globally. 
        // So we need to do so manually.

        if (numused) {
          glUniform1i(enablelighting,true) ;
	 
          // YOUR CODE FOR HW 2 HERE.  
          // You need to pass the lights to the shader. 
          // Remember that lights are transformed by modelview first.  
	  for (int i = 0; i < numused; i++){
	    transformvec(&lightposn[i*4], &lightransf[i*4]);
          
	  }
            glUniform4fv(ambientcol, 1, ambient);
            glUniform4fv(diffusecol, 1, diffuse);
            
           //################### Coloring code glColor3f(diffuse[0],diffuse[1],diffuse[2]);
            
            glUniform4fv(specularcol, 1, specular);
            glUniform4fv(emissioncol, 1, emission);
            glUniform1f(shininesscol, shininess);
            glUniform1i(numusedcol, numused);
            glUniform4fv(lightpos, numused, lightransf);
            glUniform4fv(lightcol, numused, lightcolor);
        }

        else glUniform1i(enablelighting,false) ;
     
    
    

        // Transformations for objects, involving translation and scaling 
        mat4 sc(1.0) , tr(1.0), transf(1.0) ; 
        sc = Transform::scale(sx,sy,1.0) ; 
        tr = Transform::translate(tx,ty,0.0) ;
        transf = mv*tr*sc;
        glLoadMatrixf(&transf[0][0]) ; 

        for (int i = 0 ; i < numobjects ; i++) {
          object * obj = &(objects[i]) ; 

          {
          // YOUR CODE FOR HW 2 HERE. 
          // Set up the object transformations 
          // And pass in the appropriate material properties
	    glUniform4fv(ambientcol, 1, obj->ambient);
	    glUniform4fv(diffusecol, 1, obj->diffuse);
	    glUniform4fv(specularcol, 1, obj->specular);
	    glUniform4fv(emissioncol, 1, obj->emission);
	    glUniform1f(shininesscol, obj->shininess);
	    mat4 finalTransform = transf * obj->transform *mat4(1.0);
	    //glMatrixMode(GL_MODELVIEW);
	    glLoadMatrixf(&finalTransform[0][0]);
          }

          // Actually draw the object
          // We provide the actual glut drawing functions for you.  
          if(((obj -> animation)&&(goanimate))||(obj -> nonanimation)||((!obj->animation)&&(!obj->nonanimation)))
          {
          if (obj -> type == cube) {
            glutSolidCube(obj->size) ; 
          }
          else if (obj -> type == sphere) {
            const int tessel = 20 ; 
            glutSolidSphere(obj->size, tessel, tessel) ; 
          }
          else if (obj -> type == teapot) {
            glutSolidTeapot(obj->size) ; 
          }
          else if (obj -> type == book){
            draw_obj(book_vertices, book_elements);
          }
          else if (obj -> type == fish){
              draw_obj(fish_vertices, fish_elements);
          }
          }
        }
            
    		glPushMatrix() ;
    		glColor3f(1, 1, 1);
    		glScalef(5,8,1);
			glTranslatef(0.0,0.0,-3) ;
			glRotatef(ballRot, 0.0, 0.0, 40.0);
			glRotatef(90.0,-1.0,0.0,0.0) ;
            glEnable(GL_TEXTURE_2D);
			draw_obj(fish_vertices, fish_elements) ;
            glDisable(GL_TEXTURE_2D);
			glPopMatrix() ;
			glFlush ();
			
            glPushMatrix() ;
            glColor3f(.5, .5, .5);
            glScalef(5,8,1);
            glTranslatef(0.0,0.0,-3) ;
            glRotatef(ballRot, 0.0, 0.0, 40.0);
            glRotatef(90.0,-1.0,0.0,0.0) ;
            glEnable(GL_TEXTURE_2D);
            draw_obj(fish_vertices, fish_elements) ;
            glDisable(GL_TEXTURE_2D);
            glPopMatrix() ;
            glFlush ();
    
    
        //transf = mv*tr*sc;
        //glLoadMatrixf(&transf[0][0]) ;
        //draw_obj(book_vertices, book_elements);
    
    
        glutSwapBuffers();
}
