
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <deque>
#include <stack>
#include <GL/glut.h>
#include "shaders.h"
#include "Transform.h"
#include <FreeImage.h>
#include "UCB/grader.h"

using namespace std ; 

// Main variables in the program.  
#define MAINPROGRAM 
#include "variables.h" 
#include "readfile.h" // prototypes for readfile.cpp  

int mouseoldx, mouseoldy ;
GLdouble eyeloc = 1.0 ;
void display(void) ;  // prototype for display function. 

Grader grader;
bool allowGrader = false;
bool switchOn = false;

// Uses the Projection matrices (technically deprecated) to set perspective 
// We could also do this in a more modern fashion with glm.  
void reshape(int width, int height){
	w = width;
	h = height;
        mat4 mv ; // just like for lookat

	glMatrixMode(GL_PROJECTION);
        float aspect = w / (float) h, zNear = 0.1, zFar = 99.0 ;
        // I am changing the projection stuff to be consistent with lookat
        if (useGlu) mv = glm::perspective(fovy,aspect,zNear,zFar) ; 
        else {
          mv = Transform::perspective(fovy,aspect,zNear,zFar) ; 
          mv = glm::transpose(mv) ; // accounting for row major 
        }
        glLoadMatrixf(&mv[0][0]) ; 

	glViewport(0, 0, w, h);
}

// objLoader (from wiki)
void load_obj(const char* filename, vector<glm::vec4> &vertices, vector<glm::vec3> &normals,
              vector<glm::vec2> &textures, vector<GLushort> &elements) {
    ifstream in(filename, ios::in);
    if (!in) { cerr << "Cannot open " << filename << endl; exit(1); }
    
    string line;
    while (getline(in, line)) {
        if (line.substr(0,2) == "v ") {
            istringstream s(line.substr(2));
            glm::vec4 v;
            s >> v.x;
            s >> v.y;
            s >> v.z;
            v.w = 1.0f;
            //std::cout << "This is v: " << v.x << " " << v.y << " " << v.z  << "\n";
            vertices.push_back(v);
        } else if (line.substr(0,3) == "vt ") {
            istringstream s(line.substr(2));
            glm::vec2 t;
            s >> t.x;
            s >> t.y;
            textures.push_back(t);
        } else if (line.substr(0,2) == "f ") {
            replace(line.begin(),line.end(), '/',' ');
            istringstream s(line.substr(2));
            int a,b,c, d, e, f, g, i, j;
            s >> a; s >> b; s >> c;
            s >> d; s >> e; s >> f;
            s >> g; s >> i; s >> j;
            a--;
            d--;
            g--;
            elements.push_back(a); elements.push_back(d); elements.push_back(g);
        }
        else if (line[0] == '#') { /* ignoring this line */ }
        else { /* ignoring this line */ }
    }
    
    normals.resize(vertices.size(), glm::vec3(0.0, 0.0, 0.0));
    for (int i = 0; i < elements.size(); i+=3) {
        GLushort ia = elements[i];
        GLushort ib = elements[i+1];
        GLushort ic = elements[i+2];
        glm::vec3 normal = glm::normalize(glm::cross(
                                                     glm::vec3(vertices[ib]) - glm::vec3(vertices[ia]),
                                                     glm::vec3(vertices[ic]) - glm::vec3(vertices[ia])));
        normals[ia] = normals[ib] = normals[ic] = normal;
        
    }
}

//load texture
GLuint load_texture(const char *filename, int width, int height)
{
    GLuint texture;
    unsigned char *data;
    FILE *file;
    
    // open texture data
    file = fopen(filename, "rb");
    if (file == NULL) return 0;
    
    // allocate buffer
    data = (unsigned char*) malloc(width * height * 4);
    
    // read texture data
    fread(data, width * height * 4, 1, file);
    fclose(file);
    
    // allocate a texture name
    glGenTextures(1, &texture);
    
    // select our current texture
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // select modulate to mix texture with color for shading
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_DECAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_DECAL);
    
    // when texture area is small, bilinear filter the closest mipmap
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    // when texture area is large, bilinear filter the first mipmap
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // texture should tile
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    // build our texture mipmaps
    gluBuild2DMipmaps(GL_TEXTURE_2D, 4, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
    
    // free buffer
    free(data);
    
    return texture;
}


//Original code

void saveScreenshot(string fname) {
	int pix = w * h;
	BYTE pixels[3*pix];	
	glReadBuffer(GL_FRONT);
	glReadPixels(0,0,w,h,GL_BGR,GL_UNSIGNED_BYTE, pixels);

	FIBITMAP *img = FreeImage_ConvertFromRawBits(pixels, w, h, w * 3, 24, 0xFF0000, 0x00FF00, 0x0000FF, false);
	
	std::cout << "Saving screenshot: " << fname << "\n";

	FreeImage_Save(FIF_PNG, img, fname.c_str(), 0);
}


void printHelp() {
	std::cout << "\npress 'h' to print this message again.\n" 
       << "press '+' or '-' to change the amount of rotation that\noccurs with each arrow press.\n" 
	    << "press 'i' to run image grader test cases\n"
            << "press 'g' to switch between using glm::lookAt and glm::Perspective or your own LookAt.\n"       
            << "press 'r' to reset the transformations.\n"
            << "press 'v' 't' to do view [default], translate.\n"
            << "press 'x' to stop fish rotation"
            << "press ESC to quit.\n" ;               
}
//############################################################################################################################
int upRotCounter = 0;
int downRotCounter = 0;
//centerinit = center;

GLint animate = 0;

void animateBall(void){
		if(drawSphere){
			ballRot += 0.25;
        	glutPostRedisplay();
        }	
}
//############################################################################################################################
void keyboard(unsigned char key, int x, int y) {
	switch(key) {
	case '+':
		amount++;
		std::cout << "amount set to " << amount << "\n" ;
		break;
	case '-':
		amount--;
		std::cout << "amount set to " << amount << "\n" ; 
		break;
	case 'i':
		if(allowGrader) {
			std::cout << "Running tests...\n";
			grader.runTests();
			std::cout << "Done! [ESC to quit]\n";
		} else {
			std::cout << "Error: no input file specified for grader\n";
		}
		break;
	case 'g':
		useGlu = !useGlu;
                reshape(w,h) ; 
		std::cout << "Using glm::LookAt and glm::Perspective set to: " << (useGlu ? " true " : " false ") << "\n" ; 
		break;
	case 'h':
		printHelp();
		break;
        case 27:  // Escape to quit
                exit(0) ;
                break ;
        case 'r': // reset eye and up vectors, scale and translate. 
				eye = eyeinit ; 
				up = upinit ;
				center = centerinit;
				upRotCounter = 0;
				downRotCounter = 0;
				Transform::up(amount,  eye,  up);
				Transform::left(amount*18, eye,  up);
				ty = 0 - ((amount * 0.01)*3 );
                sx = sy = 1.0 ; 
                tx = 0.0 ; 
		break ;   
        case 'v': 
                transop = view ;
                std::cout << "Operation is set to View\n" ; 
                break ; 
        case 't':
                transop = translate ; 
                std::cout << "Operation is set to Translate\n" ; 
                break ;
//############################################################################################################################
		case 'w': //strafe forward
				eye = eye + vec3(0,0,-0.05);
				break;	                 
        case 's': //strafe back
         //       transop = scale ; 
           //     std::cout << "Operation is set to Scale\n" ;
    			eye = eye + vec3(0,0,0.05);
    			break ;
		case 'a': //strafe left
				eye = eye + vec3(-0.1,0,0);
				center = center + vec3(-0.1,0,0);
				break;
		case 'd': //strafe right
				eye = eye + vec3(0.1,0,0);
				center = center + vec3(0.1,0,0);
				break;		            	
        case 'x':
        		//animate = !animate ;
				//if (animate) glutIdleFunc(animateBall) ;
//				else glutIdleFunc(NULL) ;		
				if (drawSphere) drawSphere = false;
				else drawSphere = true;
        		break;
        case 'p':
        		//if (switchOnn == 0) switchOnn = 1;
				//else switchOnn = 0;
				std::cout << indexOfObjects;
        		break;		
//############################################################################################################################
        }			
	glutPostRedisplay();
}

//Adding mouse
// Defines a Mouse callback to zoom in and out
// This is done by modifying gluLookAt
// The actual motion is in mousedrag
// mouse simply sets state for mousedrag
void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_UP) {
			// Do Nothing ;
		}
		else if (state == GLUT_DOWN) {
			mouseoldx = x ; mouseoldy = y ; // so we can move wrt x , y
		}
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{ // Reset gluLookAt
		eyeloc = 2.0 ;
		glMatrixMode(GL_MODELVIEW) ;
		glLoadIdentity() ;
		gluLookAt(0,-eyeloc,eyeloc,0,0,0,0,1,1) ;
		glutPostRedisplay() ;
	}
}

void mousedrag(int x, int y) {
	int yloc = y - mouseoldy  ;    // We will use the y coord to zoom in/out
	eyeloc  += 0.005*yloc ;         // Where do we look from
	if (eyeloc < 0) eyeloc = 0.0 ;
	mouseoldy = y ;
    
	/* Set the eye location */
	glMatrixMode(GL_MODELVIEW) ;
	glLoadIdentity() ;
	gluLookAt(0,-eyeloc,eyeloc,0,0,0,0,1,1) ;
    
	glutPostRedisplay() ;
}


//  You will need to enter code for the arrow keys 
//  When an arrow key is pressed, it will call your transform functions

void specialKey(int key, int x, int y) {
	switch(key) {
	
	case 100: //left
          if (transop == view){
          		Transform::left(amount, eye,  up);
          		if (upRotCounter > 0){
          			Transform::up(amount*upRotCounter,eye,up);
          			upRotCounter = 0;
          		}
          		if (downRotCounter > 0){
          			Transform::up(-(amount*downRotCounter),eye,up);
          			downRotCounter = 0;
          		}
          }		
       //   else if (transop == scale) sx -= amount * 0.01 ; 
          else if (transop == translate) tx -= amount * 0.01 ; 
          break;
          
	case 101: //up
          if (transop == view){
//############################################################################################################################
		  	if (upRotCounter < 7){
		  		Transform::up(-amount,  eye,  up);
		  		if (downRotCounter > 0) downRotCounter --;
		  		else upRotCounter ++;
		  	}
          }
//############################################################################################################################          		
         // else if (transop == scale) sy += amount * 0.01 ; 
          else if (transop == translate) ty += amount * 0.01 ; 
          break;
          
	case 102: //right
          if (transop == view){
          		Transform::left(-amount, eye,  up);
          		if (upRotCounter > 0){
          			Transform::up(amount*upRotCounter,eye,up);
          			upRotCounter = 0;
          		}
          		if (downRotCounter > 0){
          			Transform::up(-(amount*downRotCounter),eye,up);
          			downRotCounter = 0;
          		}

          }		
      //    else if (transop == scale) sx += amount * 0.01 ; 
          else if (transop == translate) tx += amount * 0.01 ; 
          break;
          
	case 103: //down
          if (transop == view){
//############################################################################################################################
		  	if (downRotCounter < 5){
		  		Transform::up(amount,  eye,  up);
		  		if (upRotCounter > 0) upRotCounter --;
		  		else downRotCounter ++;
		  	}
          }
//############################################################################################################################          		
      //    else if (transop == scale) sy -= amount * 0.01 ; 
          else if (transop == translate) ty -= amount * 0.01 ; 
          break;
	}
	glutPostRedisplay();
}

void init() {
      load_obj("fish.obj", fish_vertices, fish_normals, fish_textures, fish_elements);
      load_obj("book.obj", book_vertices, book_normals, book_textures, book_elements);

      carpetTexture = load_texture("carpet.raw", 256, 256);
        std::cout << "carpetTexture: " << carpetTexture << "\n";
      // Initialize shaders
      vertexshader = initshaders(GL_VERTEX_SHADER, "shaders/light.vert.glsl") ;
      fragmentshader = initshaders(GL_FRAGMENT_SHADER, "shaders/light.frag.glsl") ;
      shaderprogram = initprogram(vertexshader, fragmentshader) ; 
      enablelighting = glGetUniformLocation(shaderprogram,"enablelighting") ;
      lightpos = glGetUniformLocation(shaderprogram,"lightposn") ;       
      lightcol = glGetUniformLocation(shaderprogram,"lightcolor") ;       
      numusedcol = glGetUniformLocation(shaderprogram,"numused") ;       
      ambientcol = glGetUniformLocation(shaderprogram,"ambient") ;       
      diffusecol = glGetUniformLocation(shaderprogram,"diffuse") ;       
      specularcol = glGetUniformLocation(shaderprogram,"specular") ;       
      emissioncol = glGetUniformLocation(shaderprogram,"emission") ;       
      shininesscol = glGetUniformLocation(shaderprogram,"shininess") ;       
}

int main(int argc, char* argv[]) {

	if (argc < 2) {
		cerr << "Usage: transforms scenefile [grader input (optional)]\n"; 
		exit(-1); 
	}

  	FreeImage_Initialise();
	glutInit(&argc, argv);
//############################################################################################################################
	switchOnn = 0;
	ballRot = 1.0;
	drawSphere = true;
//############################################################################################################################	
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutCreateWindow("HW4: Scene Viewer");
	init();
        readfile(argv[1]) ; 
	glutDisplayFunc(display);
	glutSpecialFunc(specialKey);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutIdleFunc(animateBall) ;
	glutReshapeWindow(w, h);
    glutMouseFunc(mouse) ;
	glutMotionFunc(mousedrag);

	if (argc > 2) {
		allowGrader = true;
		stringstream tcid;
		tcid << argv[1] << "." << argv[2];
		grader.init(tcid.str());
		grader.loadCommands(argv[2]);
		grader.bindDisplayFunc(display);
		grader.bindSpecialFunc(specialKey);
		grader.bindKeyboardFunc(keyboard);
		grader.bindScreenshotFunc(saveScreenshot);
	}

	printHelp();
//############################################################################################################################	
	Transform::up(amount,  eye,  up);
	Transform::left(amount*18, eye,  up);
	ty -= (amount * 0.01)*3 ;
//############################################################################################################################
	glutMainLoop();
	FreeImage_DeInitialise();
	return 0;
}
