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

// static PMTmapset* 		mapsetNow; 		/* points to current
// mapset in use */
static PMTmapindex *mapindexNow; /* points to current map in use */
// static SDL_Surface*		mapNow;			/* current map on
// display
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
int keyadd(int, int);
int keysubtract(int, int);
int gpstogpskey(uint64_t);
int inthemap(int, PMTmapindex *);

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
  } else
    return (mapNow); /* should never reach here */
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
