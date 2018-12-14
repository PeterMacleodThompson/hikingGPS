/****************** read sensor data ***************************
copied from Avignon3 ~/Documents/daemons2016/pmtstartup/ozdataviewARM.c

There are 2 functions:
        (1) initsensors() to connect to the 3 memory-mapped files
        (2) getgps()
                etc....
*/

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h> // for /tmp/memfile reads
#include <unistd.h>

/* from daemon2016 */
#if HAVE_SENSORS
#include "/home/peter/Documents/daemons2016/pmtdiefaced/src/include/dieface.h"
#include "/home/peter/Documents/daemons2016/pmtfxosd/src/include/fxos8700.h"
#include "/home/peter/Documents/daemons2016/pmtgpsd/src/include/pmtgps.h"
#endif

#define FILEPATH1 "/tmp/pmtgps.bin"
#define FILESIZE1 (sizeof(struct PMTgps))
#define FILEPATH2 "/tmp/fxos8700.bin"
#define FILESIZE2 (sizeof(struct PMTfxos8700))
#define FILEPATH3 "/tmp/pmtdieEvent.bin"
#define FILESIZE3 (sizeof(struct PMTdieEvent))

#define TRUE 1
#define FALSE 0

/* shared memory structures - global to this program */
#if HAVE_SENSORS
static struct PMTgps *gpsnow;
static struct PMTfxos8700 *fxosnow;
static struct PMTdieEvent *dienow;
#endif

/****************   initialize sensors **************************/
void initsensors() {

#if HAVE_SENSORS
  int i;
  int fd1, fd2, fd3; // /tmp/pmtgpsd /tmp/pmtfxosd /tmp/pmtdieEvent

  /* initialize daemon access - pmtgpsd */
  fd1 = open(FILEPATH1, O_RDONLY);
  if (fd1 == -1) {
    perror("Error readsensors.c /tmp/pmtgps.bin");
    exit(EXIT_FAILURE);
  }

  gpsnow = mmap(0, FILESIZE1, PROT_READ, MAP_SHARED, fd1, 0);
  if (gpsnow == MAP_FAILED) {
    close(fd1);
    perror("Error mmapping /tmp/pmtgps.bin");
    exit(EXIT_FAILURE);
  }

  /* fxosd and diefaced commented out for testing gps */
  /* initialize daemon access fxosd */
  /*    fd2 = open(FILEPATH2, O_RDONLY);
      if (fd2 == -1) {
                  perror("Error readsensors.c /tmp/fxos8700.bin");
                  exit(EXIT_FAILURE);
      }

      fxosnow = mmap(0, FILESIZE2, PROT_READ, MAP_SHARED, fd2, 0);
      if (fxosnow == MAP_FAILED) {
                  close(fd2);
                  perror("Error mmapping /tmp/fxos8700.bin");
                  exit(EXIT_FAILURE);
      }

  */
  /* initialize daemon access dieface */
/*    fd3 = open(FILEPATH3, O_RDONLY);
    if (fd3 == -1) {
                perror("Error readsensors.c /tmp/pmtdieEvent.bin");
                exit(EXIT_FAILURE);
    }

    dienow = mmap(0, FILESIZE3, PROT_READ, MAP_SHARED, fd3, 0);
    if (dienow == MAP_FAILED) {
                close(fd3);
                perror("Error mmapping /tmp/pmtdieEvent.bin");
                exit(EXIT_FAILURE);
    }

        printf("gpsnow=%lld\n", (uint64_t)gpsnow->longitude * 1000000
                        + (uint64_t)gpsnow->latitude );
*/
#endif
}

/****************  get sensor data  **************************/
/*
Percy Lake longlat 78 22 11.0, 45 13 9.0
        on map 031e01 ==> NW corner at 78 30 00W  45 15 00
        return(782211451309);	// return Percy Lake for now
Panorama longlat 116 14 26.0, 50 27 33.9
Foster Lake longlat 105 30 0.0, 56 45 0.0
Forgetmenot longlat 114 48 50.0, 50 47 43.6
Nahanni longlat 123 49 55.0, 61 06 55.0	NOT FOUND
Bonavista longlat 114 03 12.84, 50 56 41.58
*/

uint64_t getgps() {
  /* use only one of the following lines
          next line for igepv2, following line for X86
     comment out whichever line is appropriate */
  // return( (uint64_t)gpsnow->longitude * 1000000
  //			+ (uint64_t)gpsnow->latitude );
  return (782211451309); // return Percy Lake as gps location
}
