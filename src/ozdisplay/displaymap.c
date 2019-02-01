
/******************  display map  ***************************
   compile with
gcc -g  displaymap.c SDL2init.c SDL2misc.c -o displaymap -lSDL2 -lSDL2_image
-lSDL2_gfx -lm

RGB color scheme:  	stykman belt	= magenta (255,0,255)
                                        movemap arrows 	= yellow (255,255,0)
                                        newmap 			= orange
(255,150,0)

FIXME todo someday when bored
- indent is wrong
- update all error handling
        - change SDL calls to if( (x=SDL) !=0) SDLerr("display.c:12");
        - change PMT function calls: if( functioncall() != 0 )
PMTerr("display.c:13);
- figure out problem with alpha blending.... ??? and incorporate for max
coolness
- when you define a pointer,  set it to NULL....  as in SDL_Surface peter =
NULL;

END FIXME
*/

#include "../include/maps.h" /* for mapflags only */
#include "SDL2/SDL2_gfxPrimitives.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <limits.h>
#include <stdio.h>

#define DEBUG

#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 500
#define MAPSCALEDELTA 0.50
#define BUTTONSPEED 5     // lower number = higher speed
#define MAPBUTTONSPEED 15 // 15 => 3 seconds to push button

#define TRUE 1
#define FALSE 0

#define MAX_ANIMATIONS 10
#define FPS 8     // default 8 frames per second == 1/8th second per frame
#define ALPHA 125 // 125/255 = 50% alpha for green ring

/* buttons in buttonsheet */
#define WEST 0
#define EAST 1
#define SOUTH 2
#define NORTH 3
#define UP 4
#define DOWN 5

/* GLOBAL VARIABLES FOR WINDOW */
extern SDL_Window *globalwindow;     // The window we'll be rendering to
extern SDL_Renderer *globalrenderer; // The window renderer */
extern SDL_Texture *globaltexture;   // texture for display window

/* function prototypes in SDLmisc.c */
void centerRect(SDL_Rect *centerme, int x, int y);
int findmap(int mapflags, int mymapindex);

SDL_Surface *getsprite(char *filename);
int initsprite(SDL_Surface *spritesheet, SDL_Rect *sprite);
SDL_Texture *gettexture(char *filename);
int ongreenring(int x, int y);
int incenter(int x, int y);
int initSDL2(void);
void closeSDL2(void);
double getangle(int, int, int, int);
int incircle(SDL_Point, SDL_Point);
int scalepoint(SDL_Rect *, SDL_Point *, SDL_Point *);

PMTmapset *initmaps(void);
SDL_Surface *getmap(int, PMTmapset *, SDL_Rect *);
SDL_Point *placegps(SDL_Surface *mymap);

char *datapath = NULL;

/* Try to find datadir relative to the executable path */
static char *find_datadir() {
  char *base_path;
  size_t length;

  base_path = SDL_GetBasePath();
  if (base_path) {
    /* We replace the last bin/ with share/oz2 to get the the resource path */
    length = SDL_strlen(base_path);
    if ((length > 4) && !SDL_strcmp(base_path + length - 5, "/bin/")) {
      char *path =
          (char *)SDL_realloc(base_path, length + SDL_strlen("share/oz2/") + 1);
      if (path == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't realloc memory for base path: %s\n", base_path);
        SDL_free(base_path);
        return NULL;
      }

      base_path = path;
      SDL_strlcpy(base_path + length - 4, "share/oz2/", 11);
      return base_path;
    } else {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "Couldn't find a valid base path: %s\n", base_path);
      SDL_free(base_path);
    }
  } else {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't find base path: %s\n",
                 SDL_GetError());
  }
  /* An error happened */
  return NULL;
}

main() {
  SDL_Surface *mymap; /*mymap= active map on screen */
  SDL_Surface *spritesheet;
  SDL_Surface *buttonsheet;
  SDL_Surface *screenmap;

  SDL_Rect camera = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
  SDL_Rect sprite[MAX_ANIMATIONS];
  SDL_Rect spriteW = {250, 250, 0, 0};     // SpriteWindow for display screen
  SDL_Rect button[6];                      // currently 6 buttons on buttonsheet
  SDL_Rect buttonW = {250, 250, 100, 100}; // buttons are 50x50 pixels FIXME

  SDL_Texture *spritetexture;
  SDL_Texture *buttontexture;
  SDL_Texture *blackcircle;
  SDL_Texture *greenring;
  SDL_Texture *pleaseorient;
  SDL_Event e;

  SDL_Point *gpspixel; // eg Percy Lake logcabin = { 2890, 1600} on C50k
  SDL_Point center;    // generic center of a rectangle
  SDL_Point scalept;
  /* screen geography */
  SDL_Point myfinger;
  SDL_Point screencenter = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
  SDL_Point east = {SCREEN_WIDTH - 30, SCREEN_HEIGHT / 2};
  SDL_Point west = {30, SCREEN_HEIGHT / 2};
  SDL_Point north = {SCREEN_WIDTH / 2, 30};
  SDL_Point south = {SCREEN_WIDTH / 2, SCREEN_HEIGHT - 30};
  SDL_Point nowest = {SCREEN_WIDTH / 5, SCREEN_HEIGHT / 5};
  SDL_Point noeast = {400, SCREEN_HEIGHT / 5};

  SDL_RendererFlip flip = SDL_FLIP_NONE;

  int x = 0, y = 0, // coordinates for inMotion movement
      xnew = 0, ynew = 0, xdelta = 0, ydelta = 0;
  /* flags */
  int inMotionF = FALSE;
  int ringF = FALSE;
  int centerF = FALSE;
  int pleaseorientF = FALSE;
  int spriteF = TRUE;
  int changemapF = FALSE;
  int mapflags = 0;

  PMTmapset *mapsetPtr; /* points to loaded array of mapsets */
  int err;
  int quit;

  int animationRate = FPS;
  int spritecount; // 4 frames in spritesheet
  int spriteframe;
  int startTime = SDL_GetTicks();
  float spritescale = 1.2; // between between 0.25 and 4.0
  float mapscale = 1.0;    // between 0.25 and 4.0
  double angle, angledelta;

  int centerFmsecs, mapmsecs; // timers in milliseconds
  Uint8 alphabutton;          // alpha fading for buttons
  char filename[PATH_MAX];

  Uint64 gps;
  int mymapindex; /* %100 = mapsetindex, /100 = mapindex within mapset*/

  /******************** Step 1 game initialize **************************/

  /* initialize SDL, get spritesheets */
  if (!initSDL2())
    exit(0);

  /* Try to find the images and maps resources path */
  datapath = find_datadir();
  if (!datapath)
    exit(0);

  strcpy(filename, datapath);
  strcat(filename, "images/styxwalkerR.png");
  // strcat( filename, "questionmark.png" );
  if ((spritesheet = getsprite(filename)) == NULL)
    exit(0);
  strcpy(filename, datapath);
  strcat(filename, "images/buttonsheet.png");
  if ((buttonsheet = getsprite(filename)) == NULL)
    exit(0);

  /* initialize sensors,maps, get map containing gps, place gps on map */
  // initsensors(); //comment this line out for X86, leave in for igepv2
  mapsetPtr = initmaps();
  mymap = getmap(GPSMAP, mapsetPtr, &camera);
  if (mymap == NULL)
    exit(0);
  if ((gpspixel = placegps(mymap)) == NULL)
    exit(0);

  // convert spritesheet & buttonsheet to texture + clip rectangles
  spritecount = initsprite(spritesheet, sprite);
  spritetexture = SDL_CreateTextureFromSurface(globalrenderer, spritesheet);
  spriteW.w = sprite[0].w * spritescale;
  spriteW.h = sprite[0].h * spritescale;

  initsprite(buttonsheet, button);
  buttontexture = SDL_CreateTextureFromSurface(globalrenderer, buttonsheet);
  SDL_SetTextureBlendMode(buttontexture, SDL_BLENDMODE_BLEND);

  // create screenmap in display format
  screenmap = SDL_GetWindowSurface(globalwindow);

  // get background textures for screen
  strcpy(filename, datapath);
  strcat(filename, "images/circle1.png");
  blackcircle = gettexture(filename);

  strcpy(filename, datapath);
  strcat(filename, "images/ring3.png");
  greenring = gettexture(filename);

  strcpy(filename, datapath);
  strcat(filename, "images/ringN.png");
  pleaseorient = gettexture(filename);

  /* SDL BUG:: FIXME ALPHA + SDL_RenderCopyEx() below causes greenring to
     disappear SDL_SetTextureBlendMode(greenring, SDL_BLENDMODE_BLEND);
          SDL_SetTextureAlphaMod( greenring, ALPHA);
          // see also SDL_SetTextureColorMod for ideas */

  // initialize camera: center = gps
  camera.w = SCREEN_WIDTH;
  camera.h = SCREEN_HEIGHT;
  camera.x = gpspixel->x - SCREEN_WIDTH / 2;
  camera.y = gpspixel->y - SCREEN_HEIGHT / 2;

  /************ Start Giant Loop: Step 2 Events = input events ****************/

  // While application is running
  quit = FALSE;
  while (!quit) {
    /*Handle events on queue  KEEP THIS TIGHT!!*/
    while (SDL_PollEvent(&e) != 0) {
      /*   try switch (e.type) with case statements for each */
      // User requests quit
      if (e.type == SDL_QUIT) {
        quit = TRUE;
      }

      /* mouse and finger events */
      if (e.type == SDL_MOUSEBUTTONDOWN) {
        x = e.button.x;
        y = e.button.y;
        inMotionF = TRUE;
      }

      if (e.type == SDL_FINGERDOWN) {
        x = e.tfinger.x;
        y = e.tfinger.y;

        if (ongreenring(x, y)) {
          angle = 0.0;
          ringF = TRUE;
        } else if (incenter(x, y)) {
          centerF = TRUE; // start march to center-reset button
          centerFmsecs = SDL_GetTicks();
        } else
          inMotionF = TRUE;
      }

      if (e.type == SDL_MOUSEBUTTONUP || e.type == SDL_FINGERUP) {
        myfinger.x = e.tfinger.x;
        myfinger.y = e.tfinger.y;

        inMotionF = FALSE;
        ringF = FALSE;
        centerF = FALSE;

        if (mapflags) {
          /* check if a button pushed to change maps
NOTE incircle is pass by value structure, NOT pass by reference */
          if ((mapflags & scalemapUPF && incircle(myfinger, nowest)) ||

              (mapflags & scalemapDNF && incircle(myfinger, noeast)) ||

              (mapflags & movemapEF && incircle(myfinger, east)) ||

              (mapflags & movemapWF && incircle(myfinger, west)) ||

              (mapflags & movemapNF && incircle(myfinger, north)) ||

              (mapflags & movemapSF && incircle(myfinger, south)))

            changemapF = TRUE;
        }
      }

      if (inMotionF && e.type == SDL_MOUSEMOTION) {
        /*Move the camera by the same delta that the mouse offsets moved*/
        xnew = e.motion.x; // current mouse position
        ynew = e.motion.y;
        xdelta = xnew - x; // delta moved by mouse
        ydelta = ynew - y;
        camera.x -= (int)(xdelta * mapscale); // move camera same
        camera.y -= (int)(ydelta * mapscale); // delta NEGATIVE amount
        x = xnew;
        y = ynew;
      }

      if (e.type == SDL_FINGERMOTION) {
        if (inMotionF) {
          /*Move the camera by the same delta
          that the mouse offsets moved*/
          xnew = e.tfinger.x; // current mouse position
          ynew = e.tfinger.y;
          xdelta = xnew - x; // delta moved by mouse
          ydelta = ynew - y;
          camera.x -= (int)(xdelta * mapscale); // move camera same
          camera.y -= (int)(ydelta * mapscale); // delta NEGATIVE amount
          x = xnew;
          y = ynew;
        } else if (ringF) {
          xnew = e.tfinger.x; // current mouse position
          ynew = e.tfinger.y;
          if (ongreenring(x, y)) {
            angledelta = getangle(x, y, xnew, ynew);
            angle += angledelta;
            mapscale += angledelta / 40.0;
            if (mapscale > 8.0) { // MAX_MAPSCALE
              mapscale = 8.0;
              mapflags = mapflags | scalemapUPF;
              mapmsecs = SDL_GetTicks();
            }
            if (mapscale < 0.5) { // MIN_MAPSCALE
              mapscale = 0.5;
              mapflags = mapflags | scalemapDNF;
              mapmsecs = SDL_GetTicks();
            }
          } else
            ringF = FALSE;
          x = xnew;
          y = ynew;
        } else if (centerF)
          if (!incenter(x, y))
            centerF = FALSE; // stop center-reset button march
      }

      if (e.type == SDL_KEYDOWN) {
        // Select surfaces based on key press
        switch (e.key.keysym.sym) {
        case SDLK_UP:
          if (animationRate++ > 30)
            animationRate = 30;
          break;

        case SDLK_DOWN:
          if (animationRate-- < 2)
            animationRate = 2;
          break;

        case SDLK_LEFT:
          break;

        case SDLK_RIGHT:
          break;

        case SDLK_q:
          spritescale += 0.25;
          if (spritescale > 4.0)
            spritescale = 4.0;
          break;

        case SDLK_a:
          spritescale -= 0.25;
          if (spritescale < 0.25)
            spritescale = 0.25;
          break;

        case SDLK_w:
          mapscale += MAPSCALEDELTA;
          if (mapscale > 8.0) { // MAX_MAPSCALE
            mapscale = 8.0;
            mapflags = mapflags | scalemapUPF;
            mapmsecs = SDL_GetTicks();
          }
          break;

        case SDLK_s:
          mapscale -= MAPSCALEDELTA;
          if (mapscale < 0.5) { // MIN_MAPSCALE
            mapscale = 0.5;
            mapflags = mapflags | scalemapDNF;
            mapmsecs = SDL_GetTicks();
          }
          break;

        case SDLK_e:
          pleaseorientF = TRUE;
          break;

        case SDLK_d:
          pleaseorientF = FALSE;
          break;

        case SDLK_z:
          mymap = getmap(GPSMAP, mapsetPtr, &camera);
          gpspixel = placegps(mymap);
          mapscale = 1.0;
          spritescale = 1.1;
          animationRate = FPS;
          camera.w = SCREEN_WIDTH;
          camera.h = SCREEN_HEIGHT;
          centerRect(&camera, gpspixel->x, gpspixel->y);
          break;

        default:
          break;
        }
      }
    }

    /************ In Giant Loop: Step 3 Update data  *****************
     *                          eg NPC Non-Player Character **********/

    // compute spriteframe to put on renderer - copied off internet
    spriteframe =
        ((SDL_GetTicks() - startTime) * animationRate / 1000) % spritecount;
    /* FIXME this IS REALLY WIERD - DOESN'T WORK FOR spritescale=1.0 FIXME
    FIXME I suspect a bug in SDL confusing NULL value and 1.0 scale
    FINALVERSION get rid of spritescale...FIXME*/

    if (spritescale != 1.0) {
      spriteW.w = (int)(sprite[0].w * spritescale); // all sprites same size
      spriteW.h = (int)(sprite[0].h * spritescale);
    }

    // scale camera - ensure it remains centered & inside map surface
    center.x = camera.x + camera.w / 2;
    center.y = camera.y + camera.h / 2;
    camera.w = SCREEN_WIDTH * mapscale;
    camera.h = SCREEN_HEIGHT * mapscale;
    centerRect(&camera, center.x, center.y);
    if (camera.x < 0) {
      camera.x = 0;
      mapflags = mapflags | movemapWF;
      mapmsecs = SDL_GetTicks();
    }
    if (camera.y < 0) {
      camera.y = 0;
      mapflags = mapflags | movemapNF;
      mapmsecs = SDL_GetTicks();
    }
    if (camera.x > mymap->w - camera.w) {
      camera.x = mymap->w - camera.w;
      mapflags = mapflags | movemapEF;
      mapmsecs = SDL_GetTicks();
    }
    if (camera.y > mymap->h - camera.h) {
      camera.y = mymap->h - camera.h;
      mapflags = mapflags | movemapSF;
      mapmsecs = SDL_GetTicks();
    }

    gpspixel = placegps(mymap);
    // scale gps position on camera
    if (gpspixel != NULL) { /* gps on map? */
      // check if gps onscreen
      spriteF = scalepoint(&camera, gpspixel, &scalept);
      // center sprite on his belt
      centerRect(&spriteW, scalept.x, scalept.y);
    }

    // timecheck on center-reset button
    if (centerF && (SDL_GetTicks() - centerFmsecs) >= 250 * BUTTONSPEED) {
      // reset displaymap to defaults
      mymap = getmap(GPSMAP, mapsetPtr, &camera);
      gpspixel = placegps(mymap);
      mapscale = 1.0;
      spritescale = 1.1;
      animationRate = FPS;
      camera.w = SCREEN_WIDTH;
      camera.h = SCREEN_HEIGHT;
      centerRect(&camera, gpspixel->x, gpspixel->y);
      centerF = FALSE;
    }

    // timecheck on mapchange buttons
    if (mapflags) {
      alphabutton = 255 - (SDL_GetTicks() - mapmsecs) / MAPBUTTONSPEED;
      if (alphabutton <= 10)
        mapflags = 0; /* turn off all changemap flags */
    }

    if (changemapF) {
      mymap = getmap(mapflags, mapsetPtr, &camera);

      /* reset mapscale to 1.0 */
      if (mapflags & scalemapUPF || mapflags & scalemapDNF)
        mapscale = 1.0;

      mapflags = 0;
      changemapF = FALSE;
    }

    /************ In Giant Loop: Step 4 Render to screen ****************/

    // Render
    err = SDL_BlitScaled(mymap, &camera, screenmap, NULL);
    err = SDL_UpdateTexture(globaltexture, NULL, screenmap->pixels,
                            screenmap->pitch);
    if (err != 0)
      printf("SDL_UpdateTexture Error %s\n", SDL_GetError());

    /* Render map and gps sprite */
    err = SDL_RenderClear(globalrenderer);
    err = SDL_RenderCopy(globalrenderer, globaltexture, NULL, NULL); // map
    if (spriteF)
      err = SDL_RenderCopy(globalrenderer, spritetexture, &sprite[spriteframe],
                           &spriteW); // sprite
    err = SDL_RenderCopy(globalrenderer, blackcircle, NULL,
                         NULL); // screen outline

    /* Render outer rings if needed */
    if (pleaseorientF)
      err = SDL_RenderCopy(globalrenderer, pleaseorient, NULL, NULL);
    if (ringF)
      err = SDL_RenderCopyEx(globalrenderer, greenring, NULL, NULL, angle,
                             &screencenter, SDL_FLIP_NONE);

    /* render expanding center button */
    if (centerF)
      err = filledCircleRGBA(globalrenderer, SCREEN_WIDTH / 2,
                             SCREEN_HEIGHT / 2, /* circle center */
                             (SDL_GetTicks() - centerFmsecs) /
                                 BUTTONSPEED,     /* circle radius*/
                             207, 231, 229, 150); /* RGBA */

    /* render mapflag buttons for new maps?  */
    if (mapflags & scalemapUPF) {
      /*				FIXME again (see line 181) alpha causes
         texture to disappear when rendered....    alpha is problematic!!! err =
         SDL_SetTextureAlphaMod( buttontexture, alphadeleteme ); err =
         filledCircleRGBA( globalrenderer, SCREEN_WIDTH/4, SCREEN_HEIGHT/4,
         // circle center 30,	255, 0, 255, alphabutton );		//
         circle radius, RGBA
      */

      buttonW.x = nowest.x - buttonW.w / 2;
      buttonW.y = nowest.y - buttonW.h / 2;
      SDL_RenderCopy(globalrenderer, buttontexture, &button[4], &buttonW);
    }

    if (mapflags & scalemapDNF) {
      buttonW.x = noeast.x - buttonW.w / 2;
      buttonW.y = noeast.y - buttonW.h / 2;
      SDL_RenderCopy(globalrenderer, buttontexture, &button[5], &buttonW);
    }

    if (mapflags & movemapWF) {
      buttonW.x = west.x - buttonW.w / 2;
      buttonW.y = west.y - buttonW.h / 2;
      SDL_RenderCopy(globalrenderer, buttontexture, &button[0], &buttonW);
    }

    if (mapflags & movemapEF) {
      buttonW.x = east.x - buttonW.w / 2;
      buttonW.y = east.y - buttonW.h / 2;
      SDL_RenderCopy(globalrenderer, buttontexture, &button[1], &buttonW);
    }

    if (mapflags & movemapSF) {
      buttonW.x = south.x - buttonW.w / 2;
      buttonW.y = south.y - buttonW.h / 2;
      SDL_RenderCopy(globalrenderer, buttontexture, &button[2], &buttonW);
    }

    if (mapflags & movemapNF) {
      buttonW.x = north.x - buttonW.w / 2;
      buttonW.y = north.y - buttonW.h / 2;
      SDL_RenderCopy(globalrenderer, buttontexture, &button[3], &buttonW);
    }

    SDL_RenderPresent(globalrenderer);
  }

  /************ Step 5 Close = cleanup ****************/

  SDL_FreeSurface(mymap);
  SDL_FreeSurface(screenmap);
  SDL_FreeSurface(spritesheet);
  closemaps(mapsetPtr);

  // Free global resources and close SDL
  closeSDL2();

  return 0;
}
