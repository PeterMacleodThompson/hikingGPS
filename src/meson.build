
cc = meson.get_compiler('c')

# Maths functions might be implemented in libm
libm = cc.find_library('m', required: false)

libsdl2 = dependency('sdl2')
libsdl2_image = dependency('SDL2_image')
libsdl2_gfx = dependency('SDL2_gfx')
libsdl2_ttf = dependency('SDL2_ttf')

# Configuration options
config = configuration_data()

with_sensors = get_option('with-sensors')

if with_sensors == 'true'
	add_global_arguments('-DHAVE_SENSORS', language : 'c')
endif


incdir = include_directories('include')

src = ['ozdisplay/displaymap.c',
       'ozdisplay/SDL2init.c',
       'ozdisplay/SDL2misc.c',
       'ozmaps/initmaps.c',
       'ozmaps/mapmanager.c',
       'ozmaps/mapmisc.c',
       'ozmaps/readsensors.c'
]

executable('oz2image', src,
	dependencies : [libm, libsdl2, libsdl2_image, libsdl2_gfx, libsdl2_ttf],
	include_directories : incdir)
