/***********   SDL2misc.c  ******************/

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include <stdio.h>

#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 500

#define MAX_ANIMATIONS 10

#define TRUE 1
#define FALSE 0

/* GLOBAL VARIABLES FOR WINDOW */
extern SDL_Window *globalwindow;     // The window we'll be rendering to
extern SDL_Renderer *globalrenderer; // The window renderer */
extern SDL_Texture *globaltexture;   // texture for display window

void centerRect(SDL_Rect *centerme, int x, int y) {
  centerme->x = x - centerme->w / 2;
  centerme->y = y - centerme->h / 2;
  return;
}

int findmap(int flags, int mapindex) { return (2); }

/* FIXME update getmap(int) to
int getnewmap(int mapflags, SDL_Surface* mymap, SDL_Rect* camera)
static int mapindex, mapsetindex;  points to CURRENT mapset and map

mapflags = 0 means get current gps (init if necessary) and get associated map

process...
1. init gps if 1st time in this routine - static int first_time = 0
2. setting the camera is the complex job to make sure it is consistent with
        the previous map.
3. if mapflags = scalemapUPF or scalemapDNF,
        - get gps center of current camera = pixeltogps(camera.x+camera.w/2,
...)
        - get pixel center of new map ....etyc

*/

/* FIXME memory leak!!! sprite never gets SDL_FreeSurface( sprite ); */
SDL_Surface *getsprite(char *filename) {
  SDL_Surface *sprite;

  // Load image at specified path
  sprite = IMG_Load(filename);
  if (sprite == NULL)
    printf("Unable to load image %s! SDL_image Error: %s\n", filename,
           SDL_GetError());

  return sprite;
}

/*****************   initsprite ***************************
 * requires square sprites layed out length-wise in spritesheet
 * returns number of sprites. Completes array of SDL_Rects for extraction
 */
int initsprite(SDL_Surface *spritesheet, SDL_Rect *sprite) {
  int spritecount, i;

  spritecount = spritesheet->w / spritesheet->h;
  if (spritecount > MAX_ANIMATIONS)
    spritecount = MAX_ANIMATIONS;

  for (i = 0; i < spritecount; i++) {
    sprite->x = i * spritesheet->h;
    sprite->y = 0;
    sprite->w = spritesheet->h;
    sprite->h = spritesheet->h;
    sprite++;
  }

  SDL_SetColorKey(spritesheet, SDL_TRUE,
                  SDL_MapRGB(spritesheet->format, 0xFF, 0xFF, 0xFF));

  return spritecount;
}

SDL_Texture *gettexture(char *filename) {
  SDL_Surface *temp;
  SDL_Texture *texture;

  temp = IMG_Load(filename);
  if (temp == NULL)
    printf("Unable to load image %s! SDL_image Error: %s\n", filename,
           SDL_GetError());

  SDL_SetColorKey(temp, SDL_TRUE, SDL_MapRGB(temp->format, 0xFF, 0xFF, 0xFF));

  texture = SDL_CreateTextureFromSurface(globalrenderer, temp);

  return (texture);
}

int ongreenring(int x, int y) {
  // SCREEN_WIDTH = center of screen = center of circle
  if ((x - SCREEN_WIDTH / 2) * (x - SCREEN_WIDTH / 2) +
          (y - SCREEN_WIDTH / 2) * (y - SCREEN_WIDTH / 2) >
      (SCREEN_WIDTH / 2 - 30) * (SCREEN_WIDTH / 2 - 30))
    return (TRUE);
  else
    return (FALSE);
}

int incenter(int x, int y) {
  // check x**2 + y**2 < radius**2
  // SCREEN_WIDTH = center of screen = center of circle
  if ((x - SCREEN_WIDTH / 2) * (x - SCREEN_WIDTH / 2) +
          (y - SCREEN_WIDTH / 2) * (y - SCREEN_WIDTH / 2) <
      30 * 30)
    return (TRUE);
  else
    return (FALSE);
}

int incircle(SDL_Point xy, SDL_Point center) {
  // check x**2 + y**2 < radius**2
  if (((xy.x - center.x) * (xy.x - center.x) +
       (xy.y - center.y) * (xy.y - center.y)) <= (30 * 30))
    return (TRUE);
  else
    return (FALSE);
}

double getangle(int x, int y, int xnew, int ynew) {
  double angle, distance;
  int clockwise;

  // get distance between 2 points on circumference = base of triangle
  distance = sqrt((x - xnew) * (x - xnew) + (y - ynew) * (y - ynew));
  angle = 2 * asin(0.5 * distance / (SCREEN_WIDTH / 2)); // angle in radians

  // compute if clockwise(-) or counterclockwise(+)
  clockwise = TRUE;
  if (x >= SCREEN_WIDTH / 2 &&
      y <= SCREEN_HEIGHT / 2 && // 1st quadrant of circle
      xnew <= x && ynew <= y)
    clockwise = FALSE;
  if (x <= SCREEN_WIDTH / 2 &&
      y <= SCREEN_HEIGHT / 2 && // 2nd quadrant of circle
      xnew <= x && ynew >= y)
    clockwise = FALSE;
  if (x <= SCREEN_WIDTH / 2 &&
      y >= SCREEN_HEIGHT / 2 && // 3rd quadrant of circle
      xnew >= x && ynew >= y)
    clockwise = FALSE;
  if (x >= SCREEN_WIDTH / 2 &&
      y >= SCREEN_HEIGHT / 2 && // 4th quadrant of circle
      xnew >= x && ynew <= y)
    clockwise = FALSE;

  if (!clockwise)
    angle = -angle;

  return (angle * 180 / M_PI); // convert radians to degrees
}

int scalepoint(SDL_Rect *camera, SDL_Point *gpspt, SDL_Point *screenpt) {
  double position_ratiox, position_ratioy;

  // check if oldpoint inside src rectangle
  if (gpspt->x < camera->x || gpspt->x > (camera->x + camera->w) ||
      gpspt->y < camera->y || gpspt->y > (camera->y + camera->h))
    return (FALSE);

  position_ratiox = (double)(gpspt->x - camera->x) / (double)camera->w;
  position_ratioy = (double)(gpspt->y - camera->y) / (double)camera->h;

  screenpt->x = position_ratiox * 500;
  screenpt->y = position_ratioy * 500;

  return (TRUE);
}
