/*!\file window.c
 * \brief Utilisation du raster "maison" pour finaliser le pipeline de
 * rendu 3D. Ici on peut voir les géométries disponibles.
 * \author Farès BELHADJ, amsi@up8.edu
 * \date December 4, 2020.
 * \todo pour les étudiant(e)s : changer la variation de l'angle de
 * rotation pour qu'il soit dépendant du temps et non du framerate
 */


 #include <assert.h>
#include "moteur.h"
#include <stdio.h>
#include <GL4D/gl4dp.h>
#include <math.h>
#include <GL4D/gl4duw_SDL2.h>
#define NB_ASTRE 11

#include "pile.h"


//dessin
static GLuint _screenId = 0;
static void init(void);
static void key(int keycode);
static void draw(void);
static void collision(float coordonnee_x[], float coordonnee_y[]);
static void animation_vue(float x, float y, float z);
static void RotationDuSoleil(float * m, float angle, float rayon, float centre_x, float centre_y, float * coordonnee_x, float * coordonnee_y);
static void quit(void);

//sphere
static surface_t * _astres = NULL;
static surface_t * _soleil = NULL;
static surface_t * _mercure = NULL;
static surface_t * _venus = NULL;
static surface_t * _terre = NULL;
static surface_t * _mars = NULL;
static surface_t * _lune = NULL;
static surface_t * _jupiter = NULL;
static surface_t * _saturne = NULL;
static surface_t * _disqueDeSaturne = NULL;
static surface_t * _uranus = NULL;
static surface_t * _neptune = NULL;
static surface_t * _orbite = NULL;

//texture des sphere
const char * astres_tex[NB_ASTRE] = {
  "images/Soleil.bmp",
  "images/Mercure.bmp",
  "images/Venus.bmp",
  "images/Terre.bmp",
  "images/Lune.bmp",
  "images/Mars.bmp",
  "images/Jupiter.bmp",
  "images/Saturne.bmp",
  "images/Disque_Saturne.bmp",
  "images/Uranus.bmp",
  "images/Neptune.bmp",

};

//Rotation des planetes
static float a = 0.0f;
static float angle = 0.0f;
static float angleDeLalune = 0.0f;
//coordonnée + coordonnée des astres
static float orbitex = 14.0f;
static float orbitey = 3.0f;
static float orbitez= 0.0f;
static int vue_orbite = 0;
static float coordonnee_x[NB_ASTRE];
static float coordonnee_y[NB_ASTRE];
//arret des Rotations
static int stop = 0;
//arret de la SIMULATION
static int stopCollision = 0;
//variable des angles
static float var_a = 0.1f;
static float var_angle = 0.01f;
static float var_lune = 0.02f;

//Variables qui permettront de changer le point de vue de la caméra
static int vue_x = 120.0f;
static int vue_y = 15.0f;
static int vue_z = 5.0f;
static int var_x = 120.0f;
static int var_y = 15.0f;
static int var_z = 5.0f;
//rayon des differents astres
const float rayon[NB_ASTRE] = {
  10.0f,0.8f,1.2f,1.5f,0.4f,1.3f,6.0f,5.0f,8.0f,1.2f,1.5f};



//boucle infinie
  int main(int argc, char ** argv) {
    if (!gl4duwCreateWindow(argc, argv,
        "Systeme Solaire ",
        //largeur et heuteur
        10, 10, 800, 600,
        // fenetre visible   )
        GL4DW_SHOWN) ){

      return 1;
    }
    //forcer la désactivation de la synchronisation verticale 
    SDL_GL_SetSwapInterval(0);
    init();

    //création de la fenetre GL4Dummies
     _screenId = gl4dpInitScreen();
  gl4duwKeyDownFunc(key);
  // display
    gl4duwDisplayFunc(draw);
    //boucle infinie pour évité que la fenetre se ferme
    gl4duwMainLoop();
    return 0;
  }


//donnee des surface
void init(void) {
  int i;
  GLuint id[11];
  vec4 w = {1,1,1,1};



  for (i = 0; i < NB_ASTRE; i++) {
    //recuperation des  images des astres
    id[i] = getTexFromBMP(astres_tex[i]);
    //Ocreation d'une sphere + un disque pour saturne
    if (i == 8) _astres = mkDisk(30, 30);
    else _astres = mkSphere(30, 30);
    //couleur de l'image
    _astres -> dcolor = w;
    //images
    setTexId(_astres, id[i]);
    //Oactivation de la texture de planete et de l'ombrage
    enableSurfaceOption(_astres, SO_USE_TEXTURE);
    enableSurfaceOption(_astres, SO_USE_LIGHTING);
    //On l'empile
    push(_astres);
  }
  _orbite = mkOrbite(30, 30);
  _orbite -> dcolor = w;

  //On dépile notre pile
  _neptune = pop();
  _uranus = pop();
  _disqueDeSaturne = pop();
  _saturne = pop();
  _jupiter = pop();
  _mars = pop();
  _lune = pop();
  _terre = pop();
  _venus = pop();
  _mercure = pop();
  _soleil = pop();

  //Liberation de la surface
  atexit(quit);
}



static void animation_vue(float x, float y, float z) {
  //On a là une animation de caméra
  //Lorsqu'une caméra va se placer devant une planete
  //En fonction de la position initial, la caméra va baisser ou augmenter
  //petit à petit, la valeur de ses positions afin de crée une belle animation
  if (vue_y != y) {
    if (vue_y > y) vue_y -= 1.0f;
    else vue_y += 1.0f;
  }

  if (vue_x != x) {
    if (vue_x > x) vue_x -= 1.0f;
    else vue_x += 1.0f;
  }

  if (vue_z != z) {
    if (vue_z > z) vue_z -= 1.0f;
    else vue_z += 1.0f;
  }
}

//collision entre les astres = arret du prog
void collision(float coordonnee_x[], float coordonnee_y[]) {
  float distancex, distancey, distance, sommeDuRayon;
  int i;
  for (i = 0; i < NB_ASTRE - 1; i++) {
    if (i == 6) i += 1;
      else{
      distancex = coordonnee_x[i + 1] - coordonnee_x[i];
      distancey = coordonnee_y[i + 1] - coordonnee_y[i];
      distance = sqrtf(distancex * distancex + distancey * distancey);
      sommeDuRayon = rayon[i] + rayon[i + 1];
      //Si la distance entre les deux astres est inférieur à la somme des rayons = collision
      if (distance < sommeDuRayon) {
        //arret du prog
        stopCollision = 1;
      }
    }
  }
}

// Rotation des planetes autour du Soleil
void RotationDuSoleil(float * m, float angle, float rayon, float centre_x, float centre_y, float * coordonnee_x, float * coordonnee_y) {
  * coordonnee_x = centre_x + cos(angle) * rayon;
  * coordonnee_y = centre_y + sin(angle) * rayon;

  translate(m, * coordonnee_y, 0.0f, * coordonnee_x);
}


void draw(void) {
  float mvMat[16], projMat[16], nmv[16];
  coordonnee_x[0] = 0;
  coordonnee_y[0] = 0;
  if (stopCollision == 0) {
    // effacer ecran
    gl4dpClearScreen();
    clearDepth();

    MFRUSTUM(projMat, -0.05f, 0.05f, -0.05f, 0.05f, 0.1f, 1000.0f);
    MIDENTITY(mvMat);
    //camera
    animation_vue(var_x, var_y, var_z);
    lookAt(mvMat, vue_x, vue_y, vue_z, 0, 0, 0, 0, 1, 0);
    //Lorsque l'utilisateur voudra voir les planete d'en haut, on affichera l'orbite d'un astre
    if (vue_orbite == 1) {
      //Orbite
      memcpy(nmv,  mvMat, sizeof nmv);
      translate(nmv, orbitex, 0.0f, orbitey);
      scale(nmv, orbitez, 0.0f, orbitez);
      transform_n_raster(_orbite, nmv, projMat);
    }

    //planetes, Soleil et Lune
    //Soleil
    memcpy(nmv, mvMat, sizeof nmv);
    scale(nmv, 10.0f, 12.0f, rayon[0]);
    rotation(nmv, a, 3.0f, 4.0f, 5.0f);
    transform_n_raster(_soleil, nmv, projMat);

    //Mercure
    memcpy(nmv, mvMat, sizeof nmv);
    RotationDuSoleil(nmv, angle + 20, 14.0f, 0.0f, 3.0f, & coordonnee_x[1], & coordonnee_y[1]);
    scale(nmv, 0.8f, 1.0f, rayon[1]);
    rotation(nmv, a, 0.0f, 0.1f, 0.0f);
    transform_n_raster(_mercure, nmv, projMat);

    //Venus
    memcpy(nmv, mvMat, sizeof nmv);
    RotationDuSoleil(nmv, angle + 2, 20.0f, -2.0f, -1.0f, & coordonnee_x[2], & coordonnee_y[2]);
    scale(nmv, 1.2f, 1.5f, rayon[2]);
    rotation(nmv, a, 5.0f, 1.0f, 3.0f);
    transform_n_raster(_venus, nmv, projMat);

    //Terre
    memcpy(nmv,  mvMat, sizeof nmv);
    RotationDuSoleil(nmv, angle, 29.0f, 0, 0, & coordonnee_x[3], & coordonnee_y[3]);
    scale(nmv, 1.5f, 1.8f, rayon[3]);
    rotation(nmv, a, 2.0f, 6.0f, 2.0f);
    transform_n_raster(_terre, nmv, projMat);

    //Lune
    memcpy(nmv,  mvMat, sizeof nmv);
    RotationDuSoleil(nmv, angleDeLalune, 3.0f, coordonnee_x[3], coordonnee_y[3], & coordonnee_x[4], & coordonnee_y[4]);
    scale(nmv, 0.4f, 0.5f, rayon[4]);
    rotation(nmv, a, 1.0f, 5.0f, 3.0f);
    transform_n_raster(_lune, nmv, projMat);

    //Mars
    memcpy(nmv,  mvMat, sizeof nmv);
    RotationDuSoleil(nmv, angle + 1, 39.0f, 3, -3, & coordonnee_x[5], & coordonnee_y[5]);
    scale(nmv, 1.3f, 1.5f, rayon[5]);
    rotation(nmv, a, 2.0f, 2.0f, 5.0f);
    transform_n_raster(_mars, nmv, projMat);

    //Jupiter
    memcpy(nmv,  mvMat, sizeof nmv);
    RotationDuSoleil(nmv, angle - 10, 49.0f, 0, 0, & coordonnee_x[6], & coordonnee_y[6]);
    scale(nmv, 6.0f, 6.8f, rayon[6]);
    rotation(nmv, a, 0.0f, 4.0f, 0.0f);
    transform_n_raster(_jupiter, nmv, projMat);

    //Saturne
    memcpy(nmv,  mvMat, sizeof nmv);
    RotationDuSoleil(nmv, angle + 7, 66.0f, 0, 0, & coordonnee_x[7], & coordonnee_y[7]);
    scale(nmv, 5.0f, 5.9f, rayon[7]);
    rotation(nmv, a, 5.0f, 0.0f, 5.0f);
    transform_n_raster(_saturne, nmv, projMat);

    //Anneau de Saturne
    memcpy(nmv,  mvMat, sizeof nmv);
    RotationDuSoleil(nmv, angle + 7, 66.0f, 0, 0, & coordonnee_x[8], & coordonnee_y[8]);
    scale(nmv, 8.0f, 0.0f, rayon[8]);
    rotation(nmv, a, 0.0f, 5.0f, 0.0f);
    transform_n_raster(_disqueDeSaturne, nmv, projMat);

    //Uranus
    memcpy(nmv,  mvMat, sizeof nmv);
    RotationDuSoleil(nmv, angle - 1, 78.0f, 0, 0, & coordonnee_x[9], & coordonnee_y[9]);
    scale(nmv, 1.2f, 1.5f, rayon[9]);
    rotation(nmv, a, 4.0f, 5.0f, 7.0f);
    transform_n_raster(_uranus, nmv, projMat);

    //Neptune
    memcpy(nmv,  mvMat, sizeof nmv);
    RotationDuSoleil(nmv, angle + 15, 86.0f, 0, 0, & coordonnee_x[10], & coordonnee_y[10]);
    scale(nmv, 1.5f, 1.8f, rayon[10]);
    rotation(nmv, a, 5.0f, 5.0f, 5.0f);
    transform_n_raster(_neptune, nmv, projMat);

    collision(coordonnee_x, coordonnee_y);

    //Lorsqu'on voudra placer la caméra devant un astre, on devra stopper les mvMats autour du soleil des
    //astres
    if (stop == 0) {
      a += var_a;
      angle += var_angle;
      angleDeLalune += var_lune;
    } else {
      a += var_a;
      angle = 0;
      angleDeLalune += var_lune;
    }
   
    gl4dpScreenHasChanged();
   
    //fonction permettant de raffraîchir toutes les fenêtres
    gl4dpUpdateScreen(NULL);

  }
}

void key(int keycode) {
  switch (keycode) {
    //la touche r (rapide) pour Accelere le temps
  case GL4DK_r:
    var_a += 0.01f;
    var_angle += 0.001f;
    var_lune += 0.002f;
    break;
    //la touche l (lent) pour Decelere le temps
  case GL4DK_l:
    var_a -= 0.01f;
    var_angle -= 0.001f;
    var_lune -= 0.002f;
    break;
    //Pour  se rapprocher de 0 sur l'axe x
  case GL4DK_UP:
    var_x -= 1.0f;
    break;
    //Pour s'éloigner de 0 sur l'axe x
  case GL4DK_DOWN:
    var_x += 1.0f;
    break;
    //Pour pouvoir se déplacer à gauche en visant le Soleil
  case GL4DK_LEFT:
    var_z += 1.0f;
    break;
    //Pour pouvoir se déplacer à droite en visant le Soleil
  case GL4DK_RIGHT:
    var_z -= 1.0f;
    break;
    
        //Place la caméra devant le Soleil
  case GL4DK_a:
    stop = 1;
    vue_orbite = 0;
    var_x = 30.0f;
    var_y = 0.0f;
    var_z = 0.0f;
    break;

 //Place la caméra devant Mercure
  case GL4DK_b:
    stop = 1;
    vue_orbite = 0;
   var_x = 21.0f;
    var_y = 0.0f;
    var_z = 8.0f;
    break;
    //Place la caméra devant Venus
  case GL4DK_c:
    stop = 1;
    vue_orbite = 0;
    var_x = 20.0f;
    var_y = 0.0f;
    var_z = -12.0f;
    break;
    //Place la caméra devant la Terre et la Lune
  case GL4DK_d:
    stop = 1;
    vue_orbite = 0;
    var_x = -0.0f;
    var_y = 0.0f;
    var_z = 35.0f;
    break;
    //Place la caméra devant Mars
  case GL4DK_e:
    stop = 1;
    vue_orbite = 0;
    var_x = 33.0f;
    var_y = 0.0f;
    var_z = 27.0f;
    break;
    //Place la caméra devant Jupiter
  case GL4DK_f:
    stop = 1;
    vue_orbite = 0;
    var_x = 50.0f;
    var_y = 0.0f;
    var_z = -75.0f;
    break;
    //Place la caméra Saturne et le Disque de Saturne
  case GL4DK_g:
    stop = 1;
    vue_orbite = 0;
    var_x = 65.0f;
    var_y = 1.0f;
    var_z = 75.0f;
    break;
    //Place la caméra devant Uranus
  case GL4DK_h:
    stop = 1;
    vue_orbite = 0;
    var_x = -70.0f;
    var_y = 0.0f;
    var_z = 45.0f;
    break;
    //Place la caméra devant Neptune
  case GL4DK_i:
    stop = 1;
    vue_orbite = 0;
    var_x = 60.0f;
    var_y = 0.0f;
    var_z = -70.0f;
    break;
    //Pour pouvoir voir les orbites des Astres sur un plan orthogonal
  case GL4DK_j:
    stop = 0;
    vue_orbite = !vue_orbite;
    var_x = 3.0f;
    var_y = 190.0f;
    var_z = 0.0f;
    break;
    //Place la caméra sur un point de vue 
  case GL4DK_k:
    stop = 0;
    vue_orbite = 0;
    var_x = 120.0f;
    var_y = 15.0f;
    var_z = 5.0f;
    break;
    
}
}


void quit(void) {
  //on libère nos surfaces
  if (_soleil) {
    freeSurface(_soleil);
    freeSurface(_mercure);
    freeSurface(_venus);
    freeSurface(_terre);
    freeSurface(_lune);
    freeSurface(_mars);
    freeSurface(_jupiter);
    freeSurface(_saturne);
    freeSurface(_disqueDeSaturne);
    freeSurface(_uranus);
    freeSurface(_neptune);
    freeSurface(_orbite);
    _soleil = NULL;
    _mercure = NULL;
    _venus = NULL;
    _terre = NULL;
    _lune = NULL;
    _mars = NULL;
    _jupiter = NULL;
    _saturne = NULL;
    _disqueDeSaturne = NULL;
    _uranus = NULL;
    _neptune = NULL;
    _orbite = NULL;
  }
  //libèration de tous les objets produits par GL4Dummies
  
  gl4duClean(GL4DU_ALL);
}
