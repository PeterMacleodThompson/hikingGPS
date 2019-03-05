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
 *
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
 *            ... approx 78 records ...
 *
 *
 *
 */

/**
 * typedef struct PMTmapindex - list of all maps (filenames) in 1 mapset.
 * @mapkey: format is dddmmddmm = longitude, latitude of NW corner of map.
 * @mapwidth: format is ddmm = degrees*100+minutes
 * @mapheight: format is ddmm = degrees*100+minutes
 * @mapname: map filename. Normally map is a .png image
 *
 * PMTmapindex structure is loaded from the mapindex file.
 * There is one mapindex file per mapfolder sorted by mapkey to enable binary
 * search mapindex file is sorted via linux command sort -k1n unsortedmapindex >
 * mapindex prior to loading into PMTmapindex structure The mapfolder directory
 * contains the mapindex file (called mapindex) plus all maps (identified by map
 * filename). Each mapfolder directory located at datapath/maps/PMTmapset.name
 */
typedef struct PMTmapindex { /* key = top left (NorthWest) corner of map  */
  int mapkey;                /* format dddmmddmm = longitude*10000+latitude */
  int mapwidth;
  int mapheight;
  char mapname[50];
} PMTmapindex;

/**
 * typedef struct PMTmapset - list of all mapsets known to hikingGPS
 * @sequence: NOT loaded from  mapset file - generated with bookends 0, 9999
 * @name: the directory containing mapindex file and all map files for this
 *        mapset
 * @usage: max 15 char string; eg golf,marine, aeronautic, topograpic, satellite
 * @Kscale: 50 means 1:50,000 scale.  Used for zooming, panning properly
 * @mapcount: number of maps in mapset for *mapindexPtr
 * @mapindexPtr: NOT loaded from mapset file - pointer to mapcount records
 *
 * list of mapsets sorted by increasing kscale = order to be viewed.
 * mapset name is max 10 char string.  Is also the directory name of
 * the mapfolder containing the mapindex file and all map (image) files.
 * example mapset names for Canada are CIMW C250k, C50k, Ctopo, Cphoto
 */
typedef struct PMTmapset {
  int sequence;
  char name[11];
  char usage[16];
  int Kscale;
  int mapcount;
  PMTmapindex *mapindexPtr;
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

