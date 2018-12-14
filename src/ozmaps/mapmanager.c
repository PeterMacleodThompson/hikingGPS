/*    mapmanager.c
        - compile with  gcc -g mapmanager.c initmaps.c mapmisc.c
   ../ozdisplay/SDL2init.c initgps.c -o ../../mapmanager -lSDL2 -lSDL2_image -lm


*/
/* written Peter Thompson Sept 2015
   Designed to simplify the main game loop calls to 3 simple calls

PMTmapset* 		initmaps();
SDL_Surface* 	getmap(int, PMTmapset*);
SDL_Point*		placegps(SDL_Surface*);


****************   3 key pointers are maintained by mapmanager.c
**************** PMTmapset* 		mapsetNow; 			points
to current mapset in use PMTmapindex* 	mapindexNow;		points to
current map in use SDL_Surface*	mapNow;				current map on
display


*/

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdint.h>
#include <stdio.h>

#include "../include/maps.h"

#define TRUE 1
#define FALSE 0

/************ static Now variables shared within this source file ***********/
/*	- there is no way around these 3 amigos + = semi-global variables
 */

// static PMTmapset* 		mapsetNow; 		/* points to current mapset
// in use */
static PMTmapindex *mapindexNow; /* points to current map in use */
// static SDL_Surface*		mapNow;			/* current map on display
// */
static int firsttime = TRUE; /* gps initialization flag */
static PMTmapindex *homemapindex;

/******************************************************************************/

/* function prototypes*/
SDL_Point *gpstopixel(PMTmapindex *, SDL_Surface *, uint64_t);
PMTmapset *initmaps(void);
uint64_t getgps(void);
uint64_t pixeltogps(PMTmapindex *, SDL_Surface *, SDL_Rect *);

PMTmapindex *findmapkey(int, PMTmapset *, PMTmapindex *);
PMTmapindex *findgpsmap(uint64_t, PMTmapset *);
SDL_Surface *loadmap(PMTmapset *, PMTmapindex *, SDL_Surface *);

//  #define DEBUG
#ifdef DEBUG

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "SDL2/SDL2_gfxPrimitives.h"
#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 500

#endif

/**********************   getmap ****************************************
        ALL map movement is handled in this routine.
        3 Static variables are used to know where we are ALL times
        mapflags are used to define where we are going
*/

SDL_Surface *getmap(int mapflags,      /* instructions on which map to get */
                    PMTmapset *mapset, /* location of all maps */
                    SDL_Rect *camera)  /* camera has to be reset on new map */
{

  static PMTmapset *mapsetNow; /* points to current mapset in use */
  /* don't confuse mapsetNow with passed paramater mapset = array[0] */
  static SDL_Surface *mapNow;
  // static backup copy of the 3 amigos
  static PMTmapset *homemapset;
  //	static PMTmapmapindex*		homemapindex;
  static SDL_Surface *homemap;

  PMTmapindex *mapindexTemp;
  int mapkey; // format dddmmddmm = longitude*10000+latitude
  int keylong;
  int keylat;

  int bookend;
  uint64_t gps;
  uint64_t gpsfake;
  SDL_Point *gpspixel;

  if (firsttime)
    mapsetNow = mapset; /* initialize mapsetNow = &mapset[0] */

  /**********   get gps map  *****************/
  if (mapflags == GPSMAP && firsttime) {
    /* get gps  */
    gps = getgps();

    /* search each mapset starting a 0 and going to 9999 bookend */
    while (TRUE) {
      bookend = mapset->sequence;
      if ((mapindexTemp = findgpsmap(gps, mapsetNow)) == NULL) { // not found
        mapset++;
        if (bookend == 9999) {
          printf(" no map found for gps (dddmmssddmmss)=%llu \n", gps);
          exit(0);
        }
      } else { // found
        /* save 3 amigos - map Now locations statically */
        mapsetNow = mapset;
        mapindexNow = mapindexTemp;
        mapNow = loadmap(mapsetNow, mapindexNow, mapNow);

        // create static copy of 3 amigos for fast return.
        if (homemap != NULL)
          SDL_FreeSurface(homemap);
        homemap = SDL_CreateRGBSurface(0, mapNow->w, mapNow->h, 32, 0x000000ff,
                                       0x0000ff00, 0x00ff0000, 0xff000000);
        SDL_BlitSurface(mapNow, NULL, homemap, NULL);
        homemapset = mapsetNow;
        homemapindex = mapindexNow;

        firsttime = FALSE;
        return (mapNow);
      }
    }
  } else if (mapflags == GPSMAP && !firsttime) /* return saved homemap */
  {
    if (mapsetNow == homemapset && homemapindex == mapindexNow)
      return (mapNow); /* already at homemap */
    /* restore 3 amigo maps from backup */
    if (mapNow != NULL)
      SDL_FreeSurface(mapNow);
    mapNow = SDL_CreateRGBSurface(0, homemap->w, homemap->h, 32, 0x000000ff,
                                  0x0000ff00, 0x00ff0000, 0xff000000);
    SDL_BlitSurface(homemap, NULL, mapNow, NULL);
    mapsetNow = homemapset;
    mapindexNow = homemapindex;

    return (mapNow);
  }

  /*************  get mapflags map *******************/
  if (mapflags & scalemapUPF) {
    if (mapsetNow->sequence == 9999)
      return (mapNow);
    else {
      /* convert camera center to gps format for map search */
      gpsfake = pixeltogps(mapindexNow, mapNow, camera);
      mapset = mapsetNow;
      mapset++;
      if ((mapindexTemp = findgpsmap(gpsfake, mapset)) == NULL)
        return (mapNow);
      else {
        /* save 3 map Now locations statically */
        mapsetNow = mapset;
        mapindexNow = mapindexTemp;
        mapNow = loadmap(mapsetNow, mapindexNow, mapNow);

        /* center camera on fakegps on new map */
        gpspixel = gpstopixel(mapindexNow, mapNow, gpsfake);
        camera->x = gpspixel->x - camera->w / 2;
        camera->y = gpspixel->y - camera->h / 2;

        return (mapNow);
      }
    }
  } else if (mapflags & scalemapDNF) {
    if (mapsetNow->sequence == 0)
      return (mapNow);
    else {
      /* convert camera center to gps format for map search */
      gpsfake = pixeltogps(mapindexNow, mapNow, camera);
      mapset = mapsetNow;
      mapset--;
      if ((mapindexTemp = findgpsmap(gpsfake, mapset)) == NULL)
        return (mapNow);
      else {
        /* save 3 map Now locations statically */
        mapsetNow = mapset;
        mapindexNow = mapindexTemp;
        mapNow = loadmap(mapsetNow, mapindexNow, mapNow);

        /* center camera on fakegps on new map */
        gpspixel = gpstopixel(mapindexNow, mapNow, gpsfake);
        camera->x = gpspixel->x - camera->w / 2;
        camera->y = gpspixel->y - camera->h / 2;

        return (mapNow);
      }
    }

  } else if (mapflags & movemapWF) {
    /* get new mapNow based on new  mapindexNow. mapsetNow unchanged*/
    keylong = keyadd(mapindexNow->mapkey / 10000, mapindexNow->mapwidth);
    mapkey = keylong * 10000 + (mapindexNow->mapkey % 10000);
    if ((mapindexTemp = findmapkey(mapkey, mapsetNow, mapindexNow)) == NULL)
      return (mapNow); /* no change, map not found */
    else {
      /* map found */
      mapindexNow = mapindexTemp;
      mapNow = loadmap(mapsetNow, mapindexNow, mapNow); /* get new mapNow */
      /* move camera East */
      camera->x = mapNow->w - camera->w;
      return (mapNow);
    }
  } else if (mapflags & movemapEF) {
    /* get new mapNow based on new  mapindexNow. mapsetNow unchanged*/
    keylong = keysubtract(mapindexNow->mapkey / 10000, mapindexNow->mapwidth);
    mapkey = keylong * 10000 + (mapindexNow->mapkey % 10000);
    if ((mapindexTemp = findmapkey(mapkey, mapsetNow, mapindexNow)) == NULL)
      return (mapNow); /* no change, map not found */
    else {
      /* map found */
      mapindexNow = mapindexTemp;
      mapNow = loadmap(mapsetNow, mapindexNow, mapNow); /* get new mapNow */
      /* move camera East */
      camera->x = 0;
      return (mapNow);
    }
  } else if (mapflags & movemapNF) {
    /* get new mapNow based on new  mapindexNow. mapsetNow unchanged*/
    keylat = keyadd(mapindexNow->mapkey % 10000, mapindexNow->mapheight);
    mapkey = keylat + (mapindexNow->mapkey / 10000) * 10000;
    if ((mapindexTemp = findmapkey(mapkey, mapsetNow, mapindexNow)) == NULL)
      return (mapNow); /* no change, map not found */
    else {
      /* map found */
      mapindexNow = mapindexTemp;
      mapNow = loadmap(mapsetNow, mapindexNow, mapNow); /* get new mapNow */
      /* move camera East */
      camera->y = mapNow->h - camera->h;
      return (mapNow);
    }
  } else if (mapflags & movemapSF) {
    /* get new mapNow based on new  mapindexNow. mapsetNow unchanged*/
    keylat = keysubtract(mapindexNow->mapkey % 10000, mapindexNow->mapheight);
    mapkey = keylat + (mapindexNow->mapkey / 10000) * 10000;
    if ((mapindexTemp = findmapkey(mapkey, mapsetNow, mapindexNow)) == NULL)
      return (mapNow); /* no change, map not found */
    else {
      /* map found */
      mapindexNow = mapindexTemp;
      mapNow = loadmap(mapsetNow, mapindexNow, mapNow); /* get new mapNow */
      /* move camera East */
      camera->y = 0;
      return (mapNow);
    }
  }
}

/********************** SDL_Point placegps(SDL_Surface) *******************/
/*   must be FAST - called every display cycle */

SDL_Point *placegps(SDL_Surface *mymap) {

  SDL_Point *gpspixel; /* instantiation of gpspixel pointer */
  uint64_t gps;
  int gpskey;

  gps = getgps();
  gpskey = gpstogpskey(gps);

  if (!(inthemap(gpskey, homemapindex))) /* always true unless gps moves */
    firsttime = TRUE;

  if (!(inthemap(gpskey, mapindexNow))) /* mapindexNow is static global */
    return (NULL);

  gpspixel = gpstopixel(mapindexNow, mymap, gps);

  return (gpspixel);
}

#ifdef DEBUG

/*****************************************************************/

/****************    main() for independent testing **************/
main() {

  /* GLOBAL VARIABLES FOR WINDOW */
  extern SDL_Window *globalwindow;     // The window we'll be rendering to
  extern SDL_Renderer *globalrenderer; // The window renderer */
  extern SDL_Texture *globaltexture;   // texture for display window

  SDL_Surface *mymap; /*mymap= active map on screen */
  SDL_Surface *screenmap;
  PMTmapset *mapsetPtr; /* points to loaded array of mapsets */
  SDL_Rect camera = {2500, 1500, SCREEN_WIDTH, SCREEN_HEIGHT};
  SDL_Event e;

  SDL_Point *gpspixel; // eg Percy Lake logcabin = { 2890, 1600} on C50k
  int err;

  int x = 0, y = 0, // coordinates for inMotion movement
      xnew = 0, ynew = 0, xdelta = 0, ydelta = 0;
  int quit, inMotionF;
  int mapflags = 0;

  /******************** Step 1 game initialize **************************/
  if (!initSDL2())
    exit(0);

  /* initialize maps, get map containing gps, place gps on map */
  mapsetPtr = initmaps();
  mymap = getmap(GPSMAP, mapsetPtr, &camera);
  if (mymap == NULL)
    printf("mymap = NULL\n");
  if ((gpspixel = placegps(mymap)) == NULL)
    printf("gpspixel = NULL\n");

  screenmap = SDL_GetWindowSurface(globalwindow);

  /************ Start Giant Loop: Step 2 Events = input events ****************/

  // While application is running
  quit = FALSE;
  inMotionF = FALSE;
  while (!quit) {
    /*Handle events on queue  KEEP THIS TIGHT!!*/
    while (SDL_PollEvent(&e) != 0) {
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
      if (e.type == SDL_MOUSEBUTTONUP)
        inMotionF = FALSE;

      if (inMotionF && e.type == SDL_MOUSEMOTION) {
        /*Move the camera by the same delta that the mouse offsets moved*/
        xnew = e.motion.x; // current mouse position
        ynew = e.motion.y;
        xdelta = xnew - x; // delta moved by mouse
        ydelta = ynew - y;
        camera.x -= (int)(xdelta); // move camera same
        camera.y -= (int)(ydelta); // delta NEGATIVE amount
        x = xnew;
        y = ynew;
        if (camera.x < 0) {
          camera.x = 0;
        }
        if (camera.y < 0) {
          camera.y = 0;
        }
        if (camera.x > mymap->w - camera.w) {
          camera.x = mymap->w - camera.w;
        }
        if (camera.y > mymap->h - camera.h) {
          camera.y = mymap->h - camera.h;
        }
      }

      if (e.type == SDL_KEYDOWN) {
        // Select surfaces based on key press
        switch (e.key.keysym.sym) {
        case SDLK_UP: {
          mapflags = mapflags | movemapNF;
          break;
        }
        case SDLK_DOWN: {
          mapflags = mapflags | movemapSF;
          break;
        }
        case SDLK_LEFT: {
          mapflags = mapflags | movemapWF;
          break;
        }

        case SDLK_RIGHT: {
          mapflags = mapflags | movemapEF;
          break;
        }

        case SDLK_q: {
          mapflags = mapflags | scalemapUPF;
          break;
        }
        case SDLK_a: {
          mapflags = mapflags | scalemapDNF;
          break;
        }

        case SDLK_z: {

          break;
        }
        }
      }
    }
    /************ In Giant Loop: Step 3 Update data  *****************
     *                          eg NPC Non Character Players **********/

    if (mapflags != 0) {
      mymap = getmap(mapflags, mapsetPtr, &camera);
      mapflags = 0;
    }

    /************ In Giant Loop: Step 4 Render to screen ****************/

    // Render
    err = SDL_BlitScaled(mymap, &camera, screenmap, NULL);
    err = SDL_UpdateTexture(globaltexture, NULL, screenmap->pixels,
                            screenmap->pitch);
    if (err != 0)
      printf("SDL_UpdateTexture Error %s\n", SDL_GetError());

    /* Render map and gps  */
    err = SDL_RenderClear(globalrenderer);
    err = SDL_RenderCopy(globalrenderer, globaltexture, NULL, NULL); // map
    // draw texture rectangle for gps

    SDL_RenderPresent(globalrenderer);
  }

  /************ Step 5 Close = cleanup ****************/

  SDL_FreeSurface(mymap);
  SDL_FreeSurface(screenmap);
  closemaps(mapsetPtr);

  // Free global resources and close SDL
  closeSDL2();

  return 0;
}
#endif
