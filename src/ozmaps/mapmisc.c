/*********    mapmisc.c   ********************
* miscellaneous functions to  convert longlat to pixel and back
 gps longlat format=dddmmssddmmss::  mapkey longlat format=dddmmddmm
 FIXME switch to latlong for searching AND research using double on BBB
  since NOAA=double degrees, GPS:GPGGA=double minutes  AND look for format converter
  on github.   FIXME 
*/

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "../include/maps.h"

#define TRUE 1
#define FALSE 0

/*******************   function prototypes - mapmisc.c    ************/
int gpstoypixel(int, float, uint64_t);
int gpstoxpixel(int, float, uint64_t);
int sskeylong(int);
int seconds(int ddmm);
uint64_t ypixeltogps(int, float, int);
uint64_t xpixeltogps(int, float, int);

/*******  PMTmapindex* findmapkey( int mapkey, PMTmapset mapsetNow )
************ Given
        - a mapkey ... format == dddmmddmm = longitude*10000+latitude
        - mapsetNow ==> mapset currently in use
    Return
        pointer to record in mapindex containing mapkey (NWCorner of map)
                I use binary search with pointers - K&Ritchie pg 129
**/

PMTmapindex *findmapkey(int mapkey, PMTmapset *mapsetNow,
                        PMTmapindex *mapindexNow) {
  struct PMTmapindex *low, *high, *mid; // pointers used in binary sea
  // set pointers to first and last records of mapindex
  low = mapsetNow->mapindexPtr;
  high = mapsetNow->mapindexPtr + mapsetNow->mapcount - 2;

  while (low <= high) {

    mid = low + (high - low) / 2;
    if (mapkey < mid->mapkey)
      high = mid - 1;
    else if (mapkey > mid->mapkey)
      low = mid + 1;
    else {
      printf("Found map %s mapkey %d mapset %d %s\n", mid->mapname, mid->mapkey,
             mapsetNow->sequence, mapsetNow->name);
      return (mid); /* found */
    }
  }
  /* exact key not found, start a linear search from here
          look for matching latitude, but longitudes */
  /* FIXME this search problematic.  It would be easier of PMTmapindex
          sorted by lat long (except long is 3 digits) end FIXME*/
  // no exact match - do a linear search from here
  // establish new boundaries (low,high) for linear search
  if (low - 17 < mapsetNow->mapindexPtr)
    low = mapsetNow->mapindexPtr;
  else
    low = low - 17;

  if (high + 100 > mapsetNow->mapindexPtr + mapsetNow->mapcount - 2)
    high = mapsetNow->mapindexPtr + mapsetNow->mapcount - 2;
  else
    high = high + 100;

  // each map -17 to +100 from where binary seach stopped,  is checked for a
  // match
  while (low <= high) { /* if latitudes match  && longitude bigger ==> found */
    if (mapindexNow->mapkey % 10000 == low->mapkey % 10000 &&
        mapindexNow->mapkey / 10000 < low->mapkey / 10000) {
      printf("Found map %s mapkey %d mapset %d %s\n", mid->mapname, mid->mapkey,
             mapsetNow->sequence, mapsetNow->name);
      return (low); /*found*/
    }
    low++;
  }

  /* nothing found, return NULL */
  printf("NOT Found mapkey %d mapset %d %s\n", mapkey, mapsetNow->sequence,
         mapsetNow->name);
  return (NULL); /* not found */
}

/*********************   gpstogpskey( gps ) ********************/

int gpstogpskey(uint64_t gps) {
  int longdd, longmm, longss, latdd, latmm, latss; // gps in longlat components
  int gpskey;

  // convert gps into dddmmddmm format
  longdd = gps / 10000000000;
  longmm = (gps / 100000000) % 100;
  longss = (gps / 1000000) % 100;
  latdd = (gps % 1000000) / 10000;
  latmm = (gps % 10000) / 100;
  latss = gps % 100;

  if (longss > 0)
    longmm++;
  if (latss > 0)
    latmm++;
  gpskey = longdd * 1000000 + longmm * 10000 + latdd * 100 + latmm;


  return (gpskey);
}

/***********    keysubtract ( mapkey x, mapkey y )      ********************/
/* returns (x - y) where x, y, and returned value are all mapkey format: ddmm
 * d=degrees, m=minutes */

int keysubtract(int x, int y) {
  int xdd, xmm, ydd, ymm, zdd, zmm;

  xdd = x / 100;
  xmm = x % 100;
  ydd = y / 100;
  ymm = y % 100;
  zmm = xmm - ymm;
  if (zmm < 0) {
    xdd--;
    zmm = (xmm + 60) - ymm;
  }
  zdd = xdd - ydd;
  if (zdd >= 0)
    return (zdd * 100 + zmm);
  else
    return (0);
}

/************  inthemap( key, map) checks if key is in the map **********/

int inthemap(int gpskey, PMTmapindex *map) {

  if (gpskey / 10000 <= map->mapkey / 10000 &&
      gpskey / 10000 >= keysubtract(map->mapkey / 10000, map->mapwidth) &&
      gpskey % 10000 <= map->mapkey % 10000 &&
      gpskey % 10000 >= keysubtract(map->mapkey % 10000, map->mapheight))
    return (TRUE);
  else
    return (FALSE);
}

/*******  PMTmapindex* findgpsmap( uint64_t gps, PMTmapset mapsetNow )
************ Given
        - current mapset ie mapsetNow
                if mapsetNow is NULL, find any map in any mapset starting at
sequence=0 if mapsetNow points to a mapset, look in this mapset only.  Return
                        old mapNow if nothing found
        - gps
    Return
        pointer to the closest mapindex record greater than mapkey
                I use binary search with pointers - K&Ritchie pg 129
*********************************************************************************************/
PMTmapindex *findgpsmap(uint64_t gps, PMTmapset *mapsetNow) {
  struct PMTmapindex *low, *high, *mid; // pointers used in search
  int gpskey;                           // gps in mapkey format

  gpskey = gpstogpskey(gps);

  // set pointers to first and last records of mapindex
  low = mapsetNow->mapindexPtr;
  high = mapsetNow->mapindexPtr + mapsetNow->mapcount - 2;

  while (low <= high) {
    mid = low + (high - low) / 2;
    if (gpskey < mid->mapkey)
      high = mid - 1;
    else if (gpskey > mid->mapkey)
      low = mid + 1;
    else {
      printf("Bin Srch Found GPS map %s mapkey %d mapset %d %s GPS %lu\n",
             mid->mapname, mid->mapkey, mapsetNow->sequence, mapsetNow->name,
             gps);
      return (mid); /* found */
    }
  }

  // no exact match - do a linear search from here
  // establish new boundaries (low,high) for linear search
  if (low - 17 < mapsetNow->mapindexPtr)
    low = mapsetNow->mapindexPtr;
  else
    low = low - 17;

  if (high + 100 > mapsetNow->mapindexPtr + mapsetNow->mapcount - 2)
    high = mapsetNow->mapindexPtr + mapsetNow->mapcount - 2;
  else
    high = high + 100;

  // each map -17 to +100 from where binary seach stopped,  is checked for a
  // match
  while (low <= high) {
    if (inthemap(gpskey, low)) {
      printf("Lin Srch Found GPS map %s mapkey %d mapset %d %s GPS %lu\n",
             low->mapname, low->mapkey, mapsetNow->sequence, mapsetNow->name,
             gps);
      return (low); /*found*/
    }
    low++;
  }
  // not found return NULL
  printf("NOT Found GPS map: mapset %d %s GPS %lu\n", mapsetNow->sequence,
         mapsetNow->name, gps);
  return (NULL);
}

/******************************  loadmap()   ****************************/
/*	load from file mapindexNow->name into mapNow
 */
SDL_Surface *loadmap(PMTmapset *mapsetNow, PMTmapindex *mapindexNow,
                     SDL_Surface *mapOld) {
  SDL_Surface *mapNow;
  char filename[PATH_MAX];

  strcpy(filename, datapath);
  strcat(filename, "maps/");
  strcat(filename, mapsetNow->name);
  strcat(filename, "/");
  strcat(filename, (mapindexNow->mapname));
  /* free old surface, make new surface */
  if (mapOld != NULL)
    SDL_FreeSurface(mapOld);
  mapNow = IMG_Load(filename);
  if (mapNow == NULL) {
    printf("YOU HAVE mapindex error, nonexistent filename %s\n", filename);
    printf("shutting down, please fix mapindex error Now\n");
    exit(0);
  }

  return (mapNow); /* SDL_Surface mapNow is "returned"*/
}

/******************** gpstopixel main ********************************/

SDL_Point *gpstopixel(PMTmapindex *mapindexNow, SDL_Surface *mapNow,
                      uint64_t gps) {
  static SDL_Point gpspixel; /* instantiation of gpspixel pointer!!! */

  float xpps, ypps;

  /* compute pps = pixels per second for mapNow */
  xpps = (float)mapNow->w / (float)seconds(mapindexNow->mapwidth);
  ypps = (float)mapNow->h / (float)seconds(mapindexNow->mapheight);

  /* convert gps to pixel location using pps */
  gpspixel.x = gpstoxpixel(mapindexNow->mapkey, xpps, gps);
  gpspixel.y = gpstoypixel(mapindexNow->mapkey, ypps, gps);

  return (&gpspixel);
}

/************ gps to pixel for each of x and y ***********/

int gpstoypixel(int mapkey, float ypps, uint64_t gps) {
  int y, temp;
  int gpsdegrees, gpsminutes, gpsseconds;
  int latdegrees, latminutes;
  int ssgps, sskey;

  // decode gps longlat format dddmmssddmmss
  temp = gps % 1000000;
  gpsdegrees = temp / 10000;
  gpsminutes = (temp / 100) % 100;
  gpsseconds = temp % 100;
  ssgps = gpsdegrees * 3600 + gpsminutes * 60 + gpsseconds;

  // decode mapkey longlat format dddmmddmm
  temp = mapkey % 10000;
  latdegrees = temp / 100;
  latminutes = temp % 100;
  sskey = latdegrees * 3600 + latminutes * 60;

  // compute distance from map edge & convert to  pixels
  y = (float)(sskey - ssgps) * ypps + 0.5;
  return (y);
}

int gpstoxpixel(int mapkey, float xpps, uint64_t gps) {
  int x, temp;
  int gpsdegrees, gpsminutes, gpsseconds;
  int sslong, sskey;

  // convert gps format dddmmssddmmss to longitude seconds
  temp = gps / 1000000;
  gpsdegrees = temp / 10000;
  gpsminutes = (temp / 100) % 100;
  gpsseconds = temp % 100;
  sslong = gpsdegrees * 3600 + gpsminutes * 60 + gpsseconds;

  // FES calculation ONLY
  // decode mapkey longlat format dddmmddmm to longitude seconds
  sskey = sskeylong(mapkey);
  // compute distance from map edge & convert to  pixels
  x = (float)(sskey - sslong) * xpps + 0.5;

  return (x);
}

/********** converts mapkey structure dddmmddmm to seconds ss ****/
int sskeylong(int mapkey) {
  int longkey = mapkey / 10000;
  return (((longkey / 100) * 3600) + (longkey % 100) * 60);
}

int sskeylat(int mapkey) {
  int latkey = mapkey % 10000;
  return (((latkey / 100) * 3600) + (latkey % 100) * 60);
}

/*************   converts ddmm to seconds ***************/
int seconds(int ddmm) {
  int degrees, minutes, seconds;
  degrees = ddmm / 100;
  minutes = ddmm % 100;
  seconds = (degrees * 60 + minutes) * 60;
  return (seconds);
}

/***********    keyadd( mapkey x, mapkey y )      ********************/
/* returns (x + y) where x, y, and returned value are all mapkey format: ddmm
 * d=degrees, m=minutes */

int keyadd(int x, int y) {
  int xdd, xmm, ydd, ymm, zdd, zmm;

  xdd = x / 100;
  xmm = x % 100;
  ydd = y / 100;
  ymm = y % 100;
  zmm = xmm + ymm;
  if (zmm >= 60) {
    zmm = zmm - 60;
    xdd++;
  }
  zdd = xdd + ydd;
  return (zdd * 100 + zmm);
}

/*************** pixeltogps()  ****************************/
/*	given: camera on a map and associated mapindexNow
        return: gps location of the center of the camera on map
                2 calculation steps
  1 compute pixel location ratio (xratio, yratio) as per total map pixels
  2 convert ratios to long lat locations as per total map long lat

*/

uint64_t pixeltogps(PMTmapindex *mapindexNow, SDL_Surface *mapNow,
                    SDL_Rect *camera) {
  uint64_t fakegps;
  int xpixel, ypixel;
  float xpps, ypps;

  /* compute pixel location  */
  xpixel = (camera->x + camera->w / 2);
  ypixel = (camera->y + camera->h / 2);

  /* compute pps = pixels per second for mapNow */
  xpps = (float)mapNow->w / (float)seconds(mapindexNow->mapwidth);
  ypps = (float)mapNow->h / (float)seconds(mapindexNow->mapheight);

  fakegps = xpixeltogps(mapindexNow->mapkey, xpps, xpixel) * 1000000 +
            ypixeltogps(mapindexNow->mapkey, ypps, ypixel);

  return (fakegps); /* return Percy Lake for now */
}

/*****************     pixel to gps....FES maps only!
*************************** used in markwaypt.c, modifywaypt.c and createhike.c.
DISALLOW ON UTM, LCC MAPS!!! Works for FES maps only. check before calling. Just
show start points for hikes on UTM maps.
*/

uint64_t ypixeltogps(int mapkey, float yppsNow, int y) {
  int seconds = y / yppsNow + 0.5;
  int ydegrees = seconds / 3600;
  int yminutes = (seconds % 3600) / 60;
  int yseconds = (seconds % 3600) % 60;

  int mapdegrees = (mapkey % 10000) / 100;
  int mapminutes = mapkey % 100;

  ydegrees = mapdegrees - ydegrees;
  yseconds = 60 - yseconds; // must subtract 1 from map-minutes below...
  if ((mapminutes - yminutes) < 0) {
    ydegrees--;
    yminutes = (mapminutes - 1) - yminutes + 60;
  } else
    yminutes = (mapminutes - 1) - yminutes;

  return ((long)(ydegrees * 10000 + yminutes * 100 + yseconds));
}

uint64_t xpixeltogps(int mapkey, float xppsNow, int x) {
  return (ypixeltogps(mapkey / 10000, xppsNow, x));
}
