///Final del trabajo
#ifdef APPLE
#include <GLUT/glut.h>
#else
#include <windows.h>
#include <GL/glut.h>
#include <math.h>
#endif
#include <stdio.h>
#include <gl/glut.h>
#include <gl/Gl.h>
#include <gl/Glu.h>
#include <iostream>
using namespace std;
#include <string.h>
#define BODY_HEIGHT 4.0
#define BODY_WIDTH 2.5
#define BODY_LENGTH 1.0
float spin=0;// variable para hacer girar la luz
float tx=0.0f,ty=0.1f,tz=0.0f;//para el movimiento de avatar
float ang_rotacion =0.f;
float a=30;// tamaño del piso
//PARA MOVER LA ESCENA //
float MX=0.1 ,MZ=0.1;

float movLuz=12;
////ILUMINACION ///
GLfloat luzdifusa[]=    { 1.0, 1.0, 1.0, 0.0};	// Definimos valores de luz difusa, maxima intensidad de luz blanca
GLfloat luzambiente[]=  { 0.5, 0.5, 0.5, 0.0};	// Definimos valores de luz ambiente
GLfloat luzspecular[]= { 0.0, 0.0, 0.0, 0.0};	// Definimos el valor specular de la luz
//GLfloat posicion[]=     { 2.0, 2.0, 2.0, 0.0};	// La posicion de la luz

GLfloat material_difuso[]={1, 1, 0.5, 1.0};		// Definicion de un material
GLfloat material_ambiente[]={0.0, 0.0, 0.0, 0.0};	// Definicion de un material
GLfloat material_specular[]={1.0,1.0,0,0};	// Definicion de un material
GLfloat material_emission[]={0.1,0.1,0.1,0.0};	// Definicion de un material
GLfloat brillo=60;					// Definicion de un material
//////////////////////////////////////////////////////////////////////
typedef struct
{
        GLubyte  *dibujo;        // Un puntero a los datos de la imagen
	    GLuint	bpp;		// bpp significa bits per pixel (bits por punto) es la calidad en palabras sensillas
	    GLuint largo;		// Largo de la textura
        GLuint ancho;		// Ancho de la textura
	    GLuint ID;		// ID de la textura, es como su nombre para opengl
}textura;

textura	texturas[1];		// Definimos nuestras texturas, por ahora solo 1

int cargarTGA( char *nombre, textura *imagen)
{
	GLubyte		cabezeraTGA[12]={0,0,2,0,0,0,0,0,0,0,0,0};		// Cabezera de un tga sin compresion
    GLubyte         compararTGA[12];                                        // Aca vamos a comprar la cabezera
	GLubyte		cabezera[6];						// Los 6 bytes importantes
	GLuint		bytesporpunto;						// Cuantos bytes hay por punto
	GLuint		tamanoimagen;						// Aca guardaremos el tamaño de la imagen
	GLuint		temp,i;							// Variable temporal, y una para usar con el for
	GLuint		tipo=GL_RGBA;						// Nuestro typo por defecto, lo veremos mas abajo

	FILE *archivo=fopen(nombre, "rb");	// Cargamos nuestro archivo en memoria
	if( archivo == NULL ||			// Existe nuestro archivo??
            fread(compararTGA,1,sizeof(compararTGA),archivo)!=sizeof(compararTGA)  ||      // Hay 12 bytes para leer??
	    memcmp(cabezeraTGA,compararTGA,sizeof(compararTGA))!=0		||	// Son iguales??
	    fread(cabezera,1,sizeof(cabezera),archivo)!=sizeof(cabezera)) {
		if(archivo==NULL) {
			printf("No se encontro el archivo %s\n",nombre);
			return 0;		// No se abrio el archivo
		}
		else {
			fclose(archivo);
			return 0;
		}
	}
	/* Ahora hay que leer la cabezera del archivo, para saber cuanto es el largo, ancho, y los bytes por puntos,
	para eso aca hay una ilustracion de la cabezera :
	6 bytes -> xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
		   |--- Largo ---| |---Ancho-----| |-bpp-|
	El dato del largo se guarda en el cabezera[0] y cabezera[1], para leerlo hay que multiplicar cabezera[0] por 256 y 	sumarselo a cabezera[1], para leer ancho y bpp es el mismo procedimiento */
	imagen->largo=256*cabezera[1]+ cabezera[0];
	imagen->ancho=256*cabezera[3]+ cabezera[2];

	/* Ahora vemos si hay datos no validos, como largo o ancho iguales menores a 0 o iguales a 0 */
	if( imagen->largo <= 0 ||	// Largo mayor que 0??
	    imagen->ancho <= 0 ||	// Ancho mayor que 0??
	    (cabezera[4]!=24 && cabezera[4]!=32)) {	// bpp es 24 o 32?? (solo se cargan 24 y 32 bpp)
                printf("Daton invalidos\n");
		fclose(archivo);
		return 0;
	}
	imagen->bpp=cabezera[4];	// Aca se guardan los bits por punto
	bytesporpunto=cabezera[4]/8;	// Aca los bytes por punto (1 byte = 8 bits)
        tamanoimagen=imagen->largo * imagen->ancho * bytesporpunto;      // Esta es la memoria que nesecitaremos para guardar los datos de la textura
	/*Ahora reservamos espacio en memoria para nuestra textura, luego leemos la textura del archivo */
	imagen->dibujo = (GLubyte *)malloc(tamanoimagen);	// Reservamos la memoria necesaria para nuestra textura
	if(imagen->dibujo== NULL ||		// Se logro reservar la memoria???
	   fread(imagen->dibujo, 1, tamanoimagen, archivo) != tamanoimagen ) {	// Se lee, y se comprueba que lo leido es de la misma longitud que la asignada a a dibujo.
		if(imagen->dibujo != NULL) {
			printf("Error leyendo imagen\n");
			free(imagen->dibujo);
		} else printf("Error asignando memoria\n");
		fclose(archivo);
		return 0;
	}
	/* El formato tga guarda las imagenes en BGR, y opengl usa RGB,por lo cambiamos de lugares */
        for(i=0; i< (int)tamanoimagen; i+=bytesporpunto)
	{
		temp=imagen->dibujo[i];				// Guardamos el primer valor
		imagen->dibujo[i] = imagen->dibujo[i + 2];	// Asignamos el nuevo primer valor
		imagen->dibujo[i + 2] = temp;			// Asignamos el ultimo valor
	}

	fclose (archivo);	// Cerramos el archivo

	/* Listo, terminamos con el codigo de carga, volvemos a opengl, ahora hay que asignarle a la textura un ID, luego 	decirle a opengl cuales son el largo, el ancho y los bpp */

        glGenTextures( 1 , &imagen[0].ID);    // Crea un ID para la textura, buscando un id que este vacio
        glBindTexture(GL_TEXTURE_2D, imagen[0].ID);       // Seleccionamos nuestra textura
	if(imagen->bpp ==24) tipo= GL_RGB;	// Si nuestra textura es de 24 bits, entonces se crea una textura rgb, sino una rgba
	/* Ahora creamos nuestra textura, entrando el largo, ancho y tipo */
        glTexImage2D(GL_TEXTURE_2D, 0, tipo, imagen[0].ancho, imagen[0].largo, 0, tipo, GL_UNSIGNED_BYTE, imagen[0].dibujo);
	/* Ahora le decimos a opengl como queremos que se vea nuestra textura, MAG_FILTER es cuando la textura es mas grande que el lugar donde la asignamos, y MIG_FILTER, es cuando la textura es mas pequeña que el lugar donde la asignamos, GL_LINEAR es para que se vea bien tanto cerca como lejos, pero ocupa bastante procesador. Otra opcion el GL_NEARES, que ocupa menos prosesador */
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return 1;	// Todo salio bien
}

void inicializar() {
        glShadeModel(GL_SMOOTH);       // Esto hace que al dibujar las sombran en los objetos, se vean mas parejas haciendo que se vean mejor.
        glClearColor(0.0, 0.0, 0.0, 0.0);       // Pondremos aca la funcion glclearcolor
        // Las 3 lineas siguientes hacen que el depth buffer ordene los objetos que deben ser puestos primero en panatalla y los que deben ser dibujados despues dependiendo de cuan cerca estan de la camara.
        glClearDepth(1.0);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);      // Esto hace que opengl calcule las perspectivas de la mejor forma, quita un poco de rendimiento, pero hace que las perspectivas se vean un poco mejor.
	    glEnable(GL_TEXTURE_2D);
        if(!cargarTGA("C:/Users/Estelamaris/Desktop/ComputacionGRAFICA/covid-Final/COVIDVENGANZA.tga", &texturas[0])) {
                printf("Error cargando textura\n");
		exit(0); // Cargamos la textura y chequeamos por errores
	}

	////ILUMINACION ///
	//glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

}

/*MOVIMIENTO DE LOS OBJETOS */
void movimientObj()
{
    static unsigned int tiempo_comparativo=0;
    float fps =100/60; // retardar el "giro" del alcohol,jabon , mascarilla.
    if(glutGet(GLUT_ELAPSED_TIME)>tiempo_comparativo+fps)
    {
         ang_rotacion=ang_rotacion+0.1;

         if(abs(tx-13)+abs(ty)+abs(tz+13)<0.15)
            {//caso del alcohol

            cout<<"Usted està Protegido"<<endl;
            }
            else if(abs(tx+13)+abs(ty)+abs(tz+13)<0.15)
                { //caso del jabon liquido
                     cout<<"Usted está Protegido"<<endl;
            }else if(abs(tx-13)+abs(ty)+abs(tz-13)<0.15){ // caso de la mascarilla
            cout<<"Usted está Protegido"<<endl;
        }else{
            //cout<< "CORRA "<<endl;
        }
        //Sleep(1);
        glutPostRedisplay();

        tiempo_comparativo=glutGet(GLUT_ELAPSED_TIME);
    }

}
void covid()
{
    glScalef(0.5,0.5,0.5);
    ///covid
     glPushMatrix();
    //glTranslatef(0,2.5,0);
    glScalef(0.6,0.6,0.6);//2.5,4,1
    glColor3f(0.73,0.10,0.82);
    glutSolidSphere(3.5,20,20);
    glPopMatrix();
    ///corona1
    glPushMatrix();
    glTranslatef(0,2.5,0);
    glScalef(0.5,3,0.5);//2.5,4,1
    glColor3f(0.74,0.88,0.97);
    glutSolidSphere(0.5,10,10);
    glPopMatrix();
    ///corona2
    glPushMatrix();
    glTranslatef(0,-2.5,0);
    glScalef(0.5,3,0.5);//2.5,4,1
    glColor3f(0.74,0.88,0.97);
    glutSolidSphere(0.5,10,10);
    glPopMatrix();
    ///corona3
    glPushMatrix();
    glTranslatef(2.5,0,0);
    glScalef(3,0.5,0.5);//2.5,4,1
    glColor3f(0.74,0.88,0.97);
    glutSolidSphere(0.5,10,10);
    glPopMatrix();
     ///corona4
    glPushMatrix();
    glTranslatef(-2.5,0,0);
    glScalef(3,0.5,0.5);//2.5,4,1
    glColor3f(0.74,0.88,0.97);
    glutSolidSphere(0.5,10,10);
    glPopMatrix();
     ///corona5
    glPushMatrix();
    glTranslatef(0,0,2.5);
    glScalef(0.5,0.5,3);//2.5,4,1
    glColor3f(0.74,0.88,0.97);
    glutSolidSphere(0.5,10,10);
    glPopMatrix();
    ///corona6
    glPushMatrix();
    glTranslatef(0,0,-2.5);
    glScalef(0.5,0.5,3);//2.5,4,1
    glColor3f(0.74,0.88,0.97);
    glutSolidSphere(0.5,10,10);
    glPopMatrix();
}
//// covid//
void COV()
{
    glPushMatrix();
    glTranslatef(tx-10,ty-0.1,tz+10);
    //glScalef(0.5,0.5,0.5);
    covid();//persona colocamos ultimo
    glPopMatrix();
}

///         FUNCION PERSONA    ///

  void persona()
{
    glPushMatrix();//esta en el origen
    //glTranslatef(0,BODY_HEIGHT/2,0);
    glScalef(BODY_WIDTH,BODY_HEIGHT,BODY_LENGTH);//4.0,2.5,1.0
    glColor3f(0.0,0.3,0.8);
    glutSolidCube(1);
    glPopMatrix();
    ///brazos1
   glPushMatrix();
    glTranslatef(-1.7,0,0);
    glScalef(0.5,4,0.5);//2.5,4,1
    glColor3f(1.0,0.0,0.0);
    glutSolidCube(1);
    glPopMatrix();
    ///brazo2
    glPushMatrix();
    glTranslatef(1.5,0,0);
    glScalef(0.5,4,0.5);//2.5,4,1
    glColor3f(1.0,0.0,0.0);
    glutSolidCube(1);
    glPopMatrix();
    ///pierna1
    glPushMatrix();
    glTranslatef(-1,-5.5,0);
    glScalef(0.6,6,0.6);//2.5,4,1
    glColor3f(0.43,0.43,0.65);
    glutSolidCube(1);
    glPopMatrix();
    ///pierna2
    glPushMatrix();
    glTranslatef(0.5,-5.5,0);
    glScalef(0.6,6,0.6);//2.5,4,1
    glColor3f(0.43,0.43,0.65);
    glutSolidCube(1);
    glPopMatrix();
    ///cabeza
     glPushMatrix();
    glTranslatef(0,2.5,0);
    glScalef(0.6,0.6,0.6);//2.5,4,1
    glColor3f(0.95,0.78,0.58);
    glutSolidSphere(1.5,20,20);
    glPopMatrix();
}

void avatar()
{
           ///PERSONA ADECUANDO TAMAÑO//
    glPushMatrix();
    glTranslatef(tx,ty,tz);
    glScalef(0.5,0.5,0.5);
    persona();//persona colocamos ultimo
    glPopMatrix();
}
void jabon()
{
    glPushMatrix();
    glScalef(0.5,0.5,0.5);

    ///JABON LIQUIDO///
    glPushMatrix();
    //glColor3f(0.3f,0.0f,0.25f);
    glColor3f(0.87,0.21,0.58);
    glScalef(1,2,1);
    glutSolidCube(2);
    glPopMatrix();

    glPushMatrix();/// semiesfera
    glColor3f(0.1f,0.1f,0.1f);
    glTranslated(0,2,0);
    glutSolidSphere(0.6,20,20);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0,2.5,0);
    glScalef(0.4,0.4,0.4);
    glColor3f(0.1f,0.1f,0.1f);
    GLUquadricObj *quadratic0;
    quadratic0 = gluNewQuadric();
    glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(quadratic0,1.0f,1.0f,3.0f,32,32);//dibuja un cilindro  ,2.0f,2.0f radios de las circunferencias , 4.0f altura del cilindro
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.2f,0.1f,0.1f);
    glTranslatef(0,3.5,0.5);
    glScalef(0.2,0.2,0.5);
    glutSolidCube(2);
    glPopMatrix();
    glPopMatrix();

}
void alcohol()
{
    glPushMatrix();
    glScalef(0.4,0.4,0.4);
    glPushMatrix();
    //glEnable(GL_DEPTH);
    glColor3f(0.91,0.90,0.89);
    GLUquadricObj *quadratic;
    quadratic = gluNewQuadric();
    glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(quadratic,2.0f,2.0f,6.0f,32,32);//dibuja un cilindro  ,2.0f,2.0f radios de las circunferencias , 4.0f altura del cilindro
    glPopMatrix();
      ///graficamos un segundo cilindro //tapa del alcohol
    glPushMatrix();
    glColor3d(1,0,0);
    GLUquadricObj *quadratic2;
    quadratic2 = gluNewQuadric();
    glTranslatef(0,6,0);
    glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(quadratic2,1.0f,1.0f,2.0f,32,32);
    glPopMatrix();
    glPopMatrix();
}
void mascarilla()
{
    glPushMatrix();
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    glBegin(GL_POLYGON);
    glColor3f(1.0,1.0,1.0);
    glVertex3f(2,2,0);
    glVertex3f(3,2.3,0);
    glVertex3f(3,4.3,0);
    glVertex3f(2,4,0);
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    glBegin(GL_POLYGON);
    glColor3f(1.0,1.0,1.0);
    glVertex3f(5,2,0);
    glVertex3f(6,2.3,0);
    glVertex3f(6,4.3,0);
    glVertex3f(5,4,0);
    glEnd();
    glPopMatrix();

   glPushMatrix();
   glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
   glBegin(GL_POLYGON);
   glColor3f(0.0,1.0,1.0);
   glVertex3f(2,2,0);
   glVertex3f(5,2,0);
   glVertex3f(5,4,0);
   glVertex3f(2,4,0);
   glEnd();
   glPopMatrix();

}
void objetos()
{
    movimientObj();
    glPushMatrix();
    glTranslatef(13,0,-13);
    glRotatef(ang_rotacion,1,1,1);
    alcohol();
    glPopMatrix();
    /////////////
    glPushMatrix();
    glTranslatef(-13,0,-13);
    glRotatef(ang_rotacion,1,1,1);
    jabon();
    glPopMatrix();
    ////////////
    glPushMatrix();
    glTranslatef(12.2,0,12.2);
    glRotatef(ang_rotacion,1,1,1);
    glScalef(0.5,0.5,0.5);
    mascarilla();
    glPopMatrix();
}


//////////////////////////////////////////////
void piso(){

	glPushMatrix();
	glTranslatef(0.0, -6.0, 0.0);
    //inicializar();
    glBindTexture(GL_TEXTURE_2D,texturas[0].ID);
	glBegin(GL_QUADS);
	glColor3f(0.4,0.5,0.15);// verde del piso
    //glColor3f(0.9,0.9,0.8);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(a,0,a);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(a,0,-a);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-a,0,-a);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(-a,0,a);
    glEnd();

	glPopMatrix();

}

void dibujar () {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   // Se borra el buffer de la pantalla y el de profundidad

    gluLookAt (20.0, 20.0, 20.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    //glLoadIdentity();
        glTranslatef(MX,0,MZ);
        ///// iluminacion  ///

         GLfloat posicion[]=     { 0.0, movLuz, 0.0, 0.0};	// La posicion de la luz
        glPushMatrix();
        glRotatef((GLdouble) spin, 1.0, 0.0, 0.0);   // Efectuamos la rotacion
        glLightfv(GL_LIGHT0,GL_POSITION,posicion);
        glLightfv(GL_LIGHT0,GL_AMBIENT,luzambiente);

        glMaterialfv(GL_FRONT, GL_DIFFUSE, material_difuso);
        glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambiente);
        //glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
        glMaterialfv(GL_FRONT, GL_EMISSION, material_emission);
        glMaterialf(GL_FRONT, GL_SHININESS, brillo);
        glEnable(GL_COLOR_MATERIAL);
        glPopMatrix();

      ////////////////////////TEXTURA PARA PAREDES///////////////////////////

    glPushMatrix();
	glTranslatef(0.0, -6.0, 0.0);
    glBindTexture(GL_TEXTURE_2D,texturas[0].ID);
    //inicializar();
	glBegin(GL_QUADS);
	glColor3f(0.84,0.74,0.88);
	/* La textura es una imagen cuadrada, ahora hay que asignarle los lugares donde va, para eso esta glTexCoord2f(float x, float y) donde x e y son las coordenadas de la textura que van a coincidir con el punto que vamos a definir */
        glTexCoord2f(0.0, 0.0);//PARED1
        glVertex3f(-a,0,a);
        glTexCoord2f(0.0, 1.0);
        glVertex3f(-a,0,-a);
        glTexCoord2f(1.0, 1.0);
        glVertex3f(-a,a,-a);
        glTexCoord2f(1.0, 0.0);
        glVertex3f(-a,a,a);

        glTexCoord2f(0.0, 0.0);//PARED2
        glVertex3f(-a,0,-a);
        glTexCoord2f(0.0, 1.0);
        glVertex3f(-a,a,-a);
        glTexCoord2f(1.0, 1.0);
        glVertex3f(a,a,-a);
        glTexCoord2f(1.0, 0.0);
        glVertex3f(a,0,-a);

	glEnd();
	glPopMatrix();

	///////////////COLOCANDO LOS OBJETOS EN LA ESCENA////////////////

    piso();//SE USO LA MISMA TEXTURA QUE LAS PAREDES PARA EL PISO
    objetos();
    COV();//COVID ANIMADO
    avatar();//PERSONA ANIMADA

	glLoadIdentity();

	glutSwapBuffers();
	spin=spin+0.2; //VARIABLE DE ROTACION
        //Sleep(4);
}



void reshape(int w, int h)
{
   glViewport(0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
  // glFrustum (-3.0, 3.0, -3.0, 2.0, 3.5, 300.0);//
   gluPerspective(60.0, (GLfloat) w/(GLfloat) h, 1.0, 1000.0);
   glMatrixMode(GL_MODELVIEW);
   //glLoadIdentity();
   //glTranslatef(0.0, 0.0, -3.6);
}

static void key (unsigned char key, int x, int y){// MANEJO DE EVENTOS POR TECLADO //
switch (key) {

 case 'z':case 'Z':
      tx=tx+1;
      break;
 case 'w':case 'W':
      tx= tx-1;
      break;
 case 'a':case 'A':
      tz=tz+1;
      break;
 case 's':case 'S':
     tz=tz-1;
     break;
////MOVER LA ESCENA COMPLETA//
 case '1':
    MX=MX+10; //AVANZA POR EL EJE X
    break;
 case '2':
    MX=MX-10;// RETROCEDE POR EL EJE X
    break;
 case '3':
    MZ=MZ+10;//AVANZA POR EL EJE Z
    break;
 case '4':
    MZ=MZ-10;//RETROCEDE POR EL EJE Z
    break;


}
glutPostRedisplay();

}
void mouse (int button, int state, int x, int y) //MENEJO DE EVENTOS POR MOUSE //
{
   switch (button) {//CLICK IZQUIERDO ACTIVA ILUMINACION
      case GLUT_LEFT_BUTTON:
         if (state == GLUT_DOWN) {

            glEnable(GL_LIGHTING);
			glEnable(GL_COLOR_MATERIAL);

         }
         break;
        case GLUT_RIGHT_BUTTON://CLICK DERECHO DESACTIVA ILUMINACION
         if (state == GLUT_DOWN) {
            glDisable(GL_LIGHTING);
           // glutPostRedisplay ();
         }
         break;

      default:
         break;
   }
}

void mensaje (){


 cout <<""<<endl;
    cout <<""<<endl;
    cout <<""<<endl;
    cout <<"       ***********************************"<<endl;
	cout<<"                                          COVID ::El Retorno del COVID-19 La Venganza (ESCENA)       "<<endl;
	cout <<"       ***********************************"<<endl;
	cout<<"       > Grupo 4"<<endl;
	cout<<"       "<<endl;
	cout<<"       > Presionar la tecla a ò A para avanzar  x."<<endl;
	cout<<"       > Presionar la tecla s ò S para retrocer x."<<endl;
	cout<<"       > Presionar la tecla w ò W para avanzar z."<<endl;
	cout<<"       > Presionar la tecla z ò Z para retroceder z."<<endl;
	cout<<"       > Presionar la tecla 1 avanza x+"<<endl;
	cout<<"       > Presionar la tecla 2 avanza x-"<<endl;
	cout<<"       > Presionar la tecla 3 avanza z+"<<endl;
	cout<<"       > Presionar la tecla 4 avanza z-"<<endl;
	cout<<"       > Click izquierdo activa iluminacion"<<endl;
	cout<<"       > Click derecha desactiva iluminacion"<<endl;
	cout<<"                                               "<<endl;
    cout<<"       > QUEDATE EN CASA CUIDA A TODA TU FAMILIA!"<<endl;

	cout <<"       ***********************************"<<endl;

}
int main(int argc, char **argv) {
        glutInit(&argc,argv);
        glutInitWindowPosition(100,100);
        glutInitWindowSize( 1000, 1000);
        glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
        glutCreateWindow("COVID:El retorno del covid (ESCENA)");
        inicializar();
        mensaje();
        glutDisplayFunc(dibujar);
        glutIdleFunc(dibujar);
        glutReshapeFunc(reshape);
        glutKeyboardFunc(key);
        glutMouseFunc(mouse);
        glutMainLoop();
}
