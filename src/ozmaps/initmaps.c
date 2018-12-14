/****************************     initmaps.c     ***********************
  Nov 2012 - revised Mar 2013, July 2015
  - compile with gcc -g initmaps.c

        creates n+1 arrays in memory pointing to maps:
        - 1 mapset array with n rows (1 per mapset)
        - n mapindex arrays each with

*************************************************************************/

// #define DEBUG

#include "../include/maps.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*********************************************************************
                        mapsetInit()
 creates via malloc mapset[] array loaded with records in /MAPSPATH/mapset file
 returns pointer to mapset array
**********************************************************************/

PMTmapset *mapsetInit(int mapsetcount) {
  PMTmapset *mapsetPtr;

  FILE *fp;
  FILE *fp2;
  char filename[100], filename2[100], inbuf[400];
  int i;

  /* malloc mapset array */
  mapsetPtr = (PMTmapset *)malloc(mapsetcount * sizeof(PMTmapset));
  if (mapsetPtr == NULL) {
    printf(" ERROR-mapindexInit:: no memory left\n");
    exit(EXIT_FAILURE);
  }

  // open mapset file
  strcpy(filename, MAPSPATH);
  strcat(filename, "mapset");
  if ((fp = fopen(filename, "r")) == NULL) {
    printf("ERROR mapsetInit:: unable to open %s \n", filename);
    exit(EXIT_FAILURE);
  }

  // input mapsets, add sequence with bookends start=0, end=9999
  i = 0;
  while (fgets(inbuf, 400, fp) != NULL) {
    if (inbuf[0] == '#')
      continue; /* skip this record */
    sscanf(inbuf, "%s %s %d %d", (mapsetPtr + i)->name, (mapsetPtr + i)->usage,
           &(mapsetPtr + i)->Kscale, &(mapsetPtr + i)->mapcount);
    /* validate directory and containes a mapindex file */
    strcpy(filename2, MAPSPATH);
    strcat(filename2, (mapsetPtr + i)->name);
    strcat(filename2, "/mapindex");
    if ((fp2 = fopen(filename2, "r")) == NULL) {
      printf("ERROR mapsetInit:: unable to open %s => ignored\n", filename2);
      continue; /* skip this record */
    }
    close(fp2);
    printf("mapset record %s %s %d %d \n", (mapsetPtr + i)->name,
           (mapsetPtr + i)->usage, (mapsetPtr + i)->Kscale,
           (mapsetPtr + i)->mapcount);
    (mapsetPtr + i)->sequence = i;
    i++;
  }
  (mapsetPtr + i - 1)->sequence =
      9999; // closing bookend marking last mapset record
  close(fp);
  return (mapsetPtr);
}

/*********************** openmapset *******************************
        returns count of records in mapset file so array can be malloc'd
*/

int openmapset() {
  FILE *fp;
  char filename[100], inbuf[400];
  int i;

  // open mapset file
  strcpy(filename, MAPSPATH);
  strcat(filename, "mapset");
  if ((fp = fopen(filename, "r")) == NULL) {
    printf("ERROR mapsetInit:: unable to open %s \n", filename);
    exit(EXIT_FAILURE);
  }

  // read mapsets and count
  i = 0;
  while (fgets(inbuf, 400, fp) != NULL) {
    if (inbuf[0] != '#') // # first character = comment line = ignore
      i++;
  }
  close(fp);
  printf("count of mapsets = %d \n", i);
  return (i); // return record count = mapsetcount
}

/*********************************************************************
               void mapindexInit( PMTmapset mapset, int mapsetcount)
for each record in PMTmapset:
-1- malloc PMTmapindex[mapset.mapcount]
-2- load mapindex file from disk to memory.
-3- save memory pointer in mapset.mapindexPtr
**********************************************************************/

void mapindexInit(PMTmapset mapset[], int mapsetcount) {
  struct PMTmapindex *p;
  char mapindexFilename[50], mapFilename[50];
  char inbuf[400];
  int i, j, mapkey, mapwidth, mapheight, flagMapindex = 0;
  int lat;
  float angle;

  for (i = 0; i < mapsetcount; i++) // for each mapset
  {
    // open the mapindex file associated with the next mapset record
    strcpy(mapindexFilename, MAPSPATH);
    strcat(mapindexFilename, mapset[i].name); // folder-name containing maps
    strcat(mapindexFilename, "/mapindex");    // filename always "mapindex"

    FILE *fp;
    if ((fp = fopen(mapindexFilename, "r")) == NULL) {
      printf("ERROR mapindexInit:: unable to open %s\n", mapindexFilename);
      exit(EXIT_FAILURE);
    }

    // grab memory for  mapnames being loaded from mapindex file & angles
    p = (PMTmapindex *)malloc(mapset[i].mapcount * sizeof(PMTmapindex));
    if (p == NULL) {
      printf(" ERROR-mapindexInit:: no memory left\n");
      exit(EXIT_FAILURE);
    }
    // save pointers in mapset
    mapset[i].mapindexPtr = p;

    // load the mapindex file.
    j = 0;
    while (fgets(inbuf, 400, fp) != NULL) {
      if (inbuf[0] == '#')
        continue; /* skip this record */
      sscanf(inbuf, "%d %d %d %s", &mapkey, &mapwidth, &mapheight, mapFilename);

      (*(p + j)).mapkey = mapkey;
      (*(p + j)).mapwidth = mapwidth;
      (*(p + j)).mapheight = mapheight;
      strcpy((*(p + j)).mapname, mapFilename);

      j++;
    }
    if (j != mapset[i].mapcount) {
      printf("ERROR-mapindexInit:: mapset.mapcount %d != actual %d\n",
             mapset[i].mapcount, j);
      exit(EXIT_FAILURE);
    }

    fclose(fp);
  }
  return;
}

void closemaps(PMTmapset *mapsetPtr) {
  int i;
  int mapsetcount;

  mapsetcount = openmapset();

  /* release memory for each mapindex */
  for (i = 0; i < mapsetcount; i++)
    free((mapsetPtr + i)->mapindexPtr);

  free(mapsetPtr);
  return;
}

/**************** main initmaps() routine **************************/
/* 	malloc + init from 1 file mapset array
        malloc + init from n files mapindex array for every mapset entry
        return pointer to mapset array[0]
                mapset array bookended with 0, 9999
*/
PMTmapset *initmaps(void) {
  PMTmapset *mapsetPtr;
  int mapsetcount; // number of mapset records

  /*  initialize 1 mapset, mapsetcount mapindexes*/
  mapsetcount = openmapset();
  mapsetPtr = mapsetInit(mapsetcount);
  mapindexInit(mapsetPtr, mapsetcount);

  return (mapsetPtr);
}

/**************************************************
/* test routine to read and print file
*****************************************************/
#ifdef DEBUG

main() {
  PMTmapset *mapsetPtr;

  int i;

  mapsetPtr = initmaps();

  closemaps(mapsetPtr);
}

#endif
