/**
 * DOC: -- maps.h -- contains oz2 map structure --
 * Peter Thompson Sept 2015
 *
 *
 * a mapset is a set of maps that:
 *    have the same scale, and the same "look and feel"
 *    all maps can be stitched together to form a single huge map
 *
 * a mapindex is a list of pointers to all map images that make up 1 mapset.
 *
 */

/**
 * DOC: Government of Canada Topographic Maps Example (4 mapsets)
 *      C50k  MAP WIDTH = 30 minutes	11633 pixels varies
 *      C50k  MAP HEIGHT = 15 minutes 	8027 pixels varies
 *      C50k  SIZE	unknown - not yet downloaded est 750GB
 *
 *      Ctopo MAP WIDTH = 30 minutes	4500 pixels fixed
 *      Ctopo MAP HEIGHT = 15 minutes 	3600 pixels fixed
 *      Ctopo SIZE 	630 gigbytes; 13909 maps (du -h;ls -l | wc -l)
 *	Ctopo map of Canada is 5.3km x 4.2km

 *      C250k MAP WIDTH = 2 degrees	8707 pixels varies  map to map
 *      C250k MAP HEIGHT = 1 degree 	6758 pixels varies somewhat map to map
 *      C250K SIE 	80 gigbytes; 915 maps (du -h; ls -l | wc -l)
 *	C250k map of Canada is 675m x 523m
 *
 *      CIMW  MAP WIDTH = 6 degrees		4127 pixels varies
 *      CIMW  MAP HEIGHT = 4 degrees	3738 pixels varies
 *      CIMW SIZE 	1.7 gigbytes; 78 maps (du -h; ls -l | wc -l)
 *      CIMW map of Canada is 27m x 25m
 *
*/

/**
 * DOC: Example mapset structure for Canada
 *
 * 1.  There must be 1 mapset file located at:  /MAPSPATH/mapset
 * 2.  There must be 1 mapindex file [sorted] for each record in mapset:
 *              == /MAPSPATH/mapset[i].name/mapindex
 *
 *
 *               *** structure of mapset and maps in memory arrays **
 *      		loaded from /MAPSPATH/mapset
 *      PMTmapset
 *        -----------------------------------------
 *        |C50k		| pointer to C50k mapindex	
 *        |-----------------------------------------	
 *        |C250k	| pointer to C250k mapindex
 *        |-----------------------------------------
 *        |CIMW		| pointer to CIMW mapindex 
 *        |-----------------------------------------
 *
 *
 *	PMTmapindex
 *      loaded from /MAPSPATH/C50k/mapindex		
 *        |-----------------------------------------	
 *	  |dddmmddmm key	| map filename to load		
 *        |----------------------------------------
 *		 ... approx 13,909 records ...	
 *
 *		
 *	PMTmapindex
 *	loaded from /MAPSPATH/C250k/mapindex
 *        |-----------------------------------------	
 *	  |dddmmddmm key	| map filename to load	
 *        |---------------------------------------- 
 *		... approx 915 records ...		
 *
 *
 *      PMTmapindex
 *	loaded from /MAPSPATH/CIMW/mapindex 
 *       |-----------------------------------------  
 *	 |dddmmddmm key	| mapfilename to load	
 *       |----------------------------------------
 *       |    ... approx 78 records ...		
 *
 *
 *
 */

/**
 * typedef struct PMTmapindex
 *
 * mapindex file contains list of all maps (filenames) in 1 mapset.
 * One mapset per mapfolder.
 * map files are located in folder in mapfolder name defined in PMTmapset.
 * mapkey is dddmmddmm = longitude, latitude of NW corner of map.
 * sorted by mapkey to enable binary search via linux:
 * sort -k1n unsortedmapindex > mapindex
 */
typedef struct PMTmapindex { /* key = top left (NorthWest) corner of map  */
  int mapkey;                /* format dddmmddmm = longitude*10000+latitude */
  int mapwidth;              /* format is ddmm = degrees*100+minutes */
  int mapheight;             /* format is ddmm = degrees*100+minutes */
  char mapname[50];          /* map filename */
} PMTmapindex;



/**
 * typedef struct PMTmapset
 *
 *list of mapsets sorted by increasing kscale = order to be viewed 
 */
typedef struct PMTmapset {
  int sequence;             /* NOT in file - generated with bookends 0, 9999 */
  char name[11];            /* {CIMW C250k, C50k, Ctopo, Cphoto} */
                            /* max 10 char + string terminator = 11 */
  char usage[16];           /* golf,marine, aeronautic, topograpic, satellite */
                            /* max 15 char + string terminator = 16 */
  int Kscale;               /* scale of mapset - 50 means 1:50,000 scale */
  int mapcount;             /* number of maps in mapset for *mapindexPtr */
  PMTmapindex *mapindexPtr; /* NOT in file - pointer to mapcount records */
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


