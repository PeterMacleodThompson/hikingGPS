/*********************    maps.h  **************************
revised Sept 2015

Government of Canada Map attributes
        C50k  MAP WIDTH = 30 minutes	11633 pixels varies  	NOT USED
        C50k  MAP HEIGHT = 15 minutes 	8027 pixels varies		NOT USED
        C50k  SIZE		unknown - not yet downloaded  FIXME

        Ctopo MAP WIDTH = 30 minutes	4500 pixels fixed
        Ctopo MAP HEIGHT = 15 minutes 	3600 pixels fixed
        Ctopo SIZE 				630 gigbytes, 13909 maps (du -h;
ls -l | wc -l)

        C250k MAP WIDTH = 2 degrees		8707 pixels varies  map to map
        C250k MAP HEIGHT = 1 degree 	6758 pixels varies somewhat map to map
        C250K SIZE 				80 gigbytes, 915 maps (du -h; ls
-l | wc -l)

        CIMW  MAP WIDTH = 6 degrees		4127 pixels varies
        CIMW  MAP HEIGHT = 4 degrees	3738 pixels varies
        CIMW SIZE 				1.7 gigbytes, 78 maps (du -h; ls
-l | wc -l)

SEE verifytopo.c for more info


*********************************************************************
   Structure for mapset and map files on disk
1.  There must be 1 mapset located at:  /MAPSPATH/mapset
2.  There must be 1 mapindex file [sorted] for each record in mapset:
                == /MAPSPATH/mapset[i].name/mapindex


                *** structure of mapset and maps in memory arrays **
        loaded from /MAPSPATH/mapset
        PMTmapset
        -----------------------------------------
        |C50k		| pointer to C50k mapindex	|--------
        -----------------------------------------		|
        |C250k		| pointer to C250k mapindex |-------|----
        -----------------------------------------		|	|
        |CIMW		| pointer to CIMW mapindex 	|-------|--	|---|
        -----------------------------------------		|	|
| |	|	| |	|	| PMTmapindex
|	|	|
        loaded from /MAPSPATH/C50k/mapindex				|
|	|
        -----------------------------------------		|	|
| |dddmmddmm key	| map filename to load	|<-------	|	|
        |----------------------------------------			|
| |    ... approx 13,909 records ...		|			|
|
        -----------------------------------------			|
| |	| PMTmapindex
|	| loaded from /MAPSPATH/C250k/mapindex				|
|
        -----------------------------------------			|
| |dddmmddmm key	| map filename to load	|<----------|	|
        |---------------------------------------- | |    ... approx 13,909
records ...		|				|
        ----------------------------------------- |
                                                                                                                        |
        PMTmapindex
| loaded from /MAPSPATH/CIMW/mapindex |
        ----------------------------------------- | |dddmmddmm key	| map
filename to load	|<--------------|
        |----------------------------------------
        |    ... approx 13,909 records ...		|
        -----------------------------------------



*/

/****************  PMTmapindex ************************************************
mapindex file contains list of all maps (filenames) in 1 mapset.
 One mapset per mapfolder.
 map files are located in folder in mapfolder name defined in PMTmapset.
 mapkey is dddmmddmm = longitude, latitude of NW corner of map.
 sorted by mapkey to enable binary search via linux:
 sort -k1n unsortedmapindex > mapindex
*/
typedef struct PMTmapindex { // key = top left (NorthWest) corner of map
  int mapkey;                // format dddmmddmm = longitude*10000+latitude
  int mapwidth;              // format is ddmm = degrees*100+minutes
  int mapheight;             // format is ddmm = degrees*100+minutes
  char mapname[50];          // map filename
} PMTmapindex;

/**************   PMTmapset   *****************************************/

/*list of mapsets sorted by increasing kscale = order to be viewed */
typedef struct PMTmapset {
  int sequence;             // NOT in file - generated with bookends 0, 9999
  char name[11];            // {CIMW C250k, C50k, Ctopo, Cphoto}
                            // max 10 char + string terminator = 11
  char usage[16];           // golf,marine, aeronautic, topograpic, satellite
                            // max 15 char + string terminator = 16
  int Kscale;               // scale of mapset - 50 means 1:50,000 scale
  int mapcount;             // number of maps in mapset for *mapindexPtr
  PMTmapindex *mapindexPtr; // NOT in file - pointer to mapcount records
} PMTmapset;

#define MAXMAPSETS 10
/* mapflags */
#define GPSMAP 0           /* mapflags=0 ==> get map containing gps */
#define scalemapUPF 0x0001 /* mapflags!=0 ==> new map  */
#define scalemapDNF 0x0002
#define movemapWF 0x0004
#define movemapEF 0x0008
#define movemapNF 0x0010
#define movemapSF 0x0020

extern char *datapath;

/* there are 3 function prototypes used by display
PMTmapset* 		initmaps();
SDL_Surface* 	getmap(int, PMTmapset*);	// returns mapSurface based on
int mapflags
SDL_Point*		placegps(SDL_Surface*); 	// returns point on
SDL_Surface for gps
*/

/*				 int = GPSMAP = 0, returns the map containing
 * gps
 */
