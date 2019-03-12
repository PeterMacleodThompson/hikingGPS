/**
 * DOC: --  SDL2init.c  -- Initialize display
 * Peter Thompson   -- 2015
 *
 * Initialize global variables
 *   - globalwindow - we do NOT use X
 *   - globalrenderer - we use directfb - depends on linux proper setup!!
 *   - globaltexture - must vary depending on display hardware
 *                     use -D BBB etc to specify display hardware when compiling
 *                     default compile is HDMI
 *   - datapath - to maps, images, sprites
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 500

#define TRUE 1
#define FALSE 0

/* GLOBAL VARIABLES FOR WINDOW */
SDL_Window *globalwindow;     /* Display window we'll be rendering to */
SDL_Renderer *globalrenderer; /* The window renderer */
SDL_Texture *globaltexture;   /* texture for display window */

/**
 * initSDL2() - initialize global variables and SDL2
 * Return: True or False depending on whether everything initialized ok
 */
int initSDL2() {
  int success = TRUE;

  /* informational & debugging variables */
  SDL_version compiled;
  SDL_version linked;
  SDL_RendererInfo info;
  int r, i;

  /* print machine video driver information */
  r = SDL_GetNumVideoDrivers();
  printf("Number of Video Drivers = %d\n", r);
  i = 0;
  while (i < r) {
    printf("  video driver %d = %s\n", i, SDL_GetVideoDriver(i));
    i++;
  }

  /* debug SDL2 with messages to stderr */
  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
  SDL_Log("SDL log is verbose\n");

  /* Initialize SDL2 - video only */
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    success = FALSE;
  } else
    printf("Current Video Driver = %s\n", SDL_GetCurrentVideoDriver());

  /* display SDL version. see https://wiki.libsdl.org/SDL_VERSION  */
  SDL_VERSION(&compiled);
  SDL_GetVersion(&linked);
  printf("SDL2 compiler version: %d.%d.%d ...\n", compiled.major,
         compiled.minor, compiled.patch);
  printf("SDL2 linked version: %d.%d.%d.\n", linked.major, linked.minor,
         linked.patch);

  /* Create GLOBAL window */
  if (success == TRUE) {
    globalwindow = SDL_CreateWindow("hiker!!", SDL_WINDOWPOS_UNDEFINED,
                                    SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                    SCREEN_HEIGHT, 0);
    if (globalwindow == NULL) {
      printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
      success = FALSE;
    }
  }

  /*
   * Create GLOBAL renderer, use "software renderer" for now
   * see SDL_render.c in SDL2sources
   */
  if (success == TRUE) {
    /* print available renderer drivers */
    r = SDL_GetNumRenderDrivers();
    printf("NumRenderDrivers = %d\n", r);
    i = 0;
    while (i < r) {
      if (SDL_GetRenderDriverInfo(i, &info) == 0) {
        printf("  render driver %d = %s ", i, info.name);
        printf("    flags(hex) = %x\n", info.flags);
        printf("    num texture formats = %d\n", info.num_texture_formats);
        printf("    texture max width, height = %d, %d\n",
               info.max_texture_width, info.max_texture_height);
      }
      i++;
    }

    /* initialize renderer */
    globalrenderer = SDL_CreateRenderer(globalwindow, -1, 0);
    if (globalrenderer == NULL) {
      printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
      success = FALSE;
    } else {
      /* print selected renderer */
      SDL_GetRendererInfo(globalrenderer, &info);
      printf(" Selected Renderer = %s\n", info.name);
    }
  }

  /* Initialize PNG, TIF, JPG loading */
  if (success == TRUE) {
    int imgFlags = IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
      printf("SDL_image could not initialize! SDL_image Error: %s\n",
             IMG_GetError());
      success = FALSE;
    }
  }

  /*
   * create global textures
   *       for X86 use  SDL_PIXELFORMAT_ARGB8888,
   *       for beaglebone black use SDL_PIXELFORMAT_RGB565
   */
  if (success == TRUE) {
#if BBB /* compile with -D BBB for beaglebone black */
    globaltexture = SDL_CreateTexture(globalrenderer, SDL_PIXELFORMAT_RGB888,
                                      SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH,
                                      SCREEN_HEIGHT);
#else /* default back to X86 without -D specified */
    globaltexture = SDL_CreateTexture(globalrenderer, SDL_PIXELFORMAT_ARGB8888,
                                      SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH,
                                      SCREEN_HEIGHT);
#endif
    if (globaltexture == NULL) {
      printf("Texture could not be created! SDL Error: %s\n", SDL_GetError());
      success = FALSE;
    }

    SDL_SetRenderDrawColor(globalrenderer, 255, 0, 0,
                           255); /* for drawing, clearing; RGBA */
    SDL_SetRenderDrawBlendMode(globalrenderer, SDL_BLENDMODE_BLEND);
    /* FIXME remove the above line after alpha works - installed for alpha */
  }

  /* Initialize SDL_ttf */
  if (success && TTF_Init() == -1) {
    printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
    success = FALSE;
  }

  printf("SDL2init complete\n\n\n");
  return (success);
}

/**
 * closeSDL2() - close SDL2, set global variables to NULL
 * Return: nothing
 */
void closeSDL2() {

  /* Destroy window */
  SDL_DestroyWindow(globalwindow);
  SDL_DestroyRenderer(globalrenderer);
  SDL_DestroyTexture(globaltexture);

  globalwindow = NULL;
  globalrenderer = NULL;
  /* Quit SDL subsystems IMG_Load */
  SDL_Quit();
}
