#!/usr/bin/env python3
import os
import math
import subprocess
from ctypes import *

class SavedFractal(Structure):
     _fields_ = [ 
     ('version',c_int),
     ('valid',c_int),
     ('current_fractal',c_uint),
     ('current_max_iters0',c_uint),
     ('current_max_iters1',c_uint),
     ('current_max_iters2',c_uint),
     ('current_power',c_double),
     ('current_zconst0',c_double), #complex
     ('current_zconst1',c_double), #complex
     ('current_escape_r',c_double),
     #RF
     ('theta',c_float),
     ('xtart',c_double),
     ('ystart',c_double),
     ('xdelta',c_double),
     ('ydelta',c_double),
     ('current_width',c_double),
     ('current_height',c_double),
     ('displayed_zoom',c_double),
     ('requested_zoom',c_double),
     ('show_selection',c_bool),
     ('coloring_algo',c_int),
     ('color_cycle_size',c_int),
     ('palette',c_uint),
     ('reflect_palette',c_bool),
     ('escape_image_w',c_uint),
     ('escape_image_h',c_uint),
     ('image_loaded',c_bool),
     ('light_pos_r',c_double),
     ('light_pos_i',c_double),
     ('light_angle',c_double),
     ('light_height',c_double),
     ('random_sample',c_bool),
     ('original_width',c_double),
     ('original_height',c_double),
     #('wtf',c_uint*2)
     ]


os.chdir(r"Documents/GitHub/demo/fractals/fractals_cuda/x64/Release")
print(os.getcwd())

g = open("movie_base.fractal_key_version_1","rb")
q = SavedFractal()
g.readinto(q)
g.close()

print("\n\nBase fractal_key:")
for field_name, field_type in q._fields_:
    print(field_name, getattr(q, field_name))

def create_shadow_series(i,e, basekey):
    basename="shadow_"
    for j in range(i,e):
        keyname=basename + str(j) + ".fractal_key_version_1"
        f = open(keyname,"wb")
        basekey.current_max_iters0=1500
        basekey.current_escape_r=50
        angle=j*2*math.pi/(e-1)
        radius=2
        x = radius * math.cos(angle)
        y = radius * math.sin(angle)
        basekey.light_pos_r = x
        basekey.light_pos_i = y
        f.write(basekey)
        f.close()
        pngname="movie" + str(j) + ".png"
        subprocess.run(['fractals_cuda.exe', 'save_and_exit', keyname, pngname], shell=True)

e=21
create_shadow_series(0, e, q)

from PIL import Image
import glob

# Create the frames
frames = []
imgs = sorted(glob.glob("movie*.png"), key=os.path.getmtime)
print(imgs)

for i in imgs:
    new_frame = Image.open(i)
    frames.append(new_frame)

# Save into a GIF file that loops forever
frames[0].save('shadow.gif', format='GIF',
               append_images=frames[0:e],
               save_all=True,
               duration=10, loop=0)

for i in range(e):
    os.remove(imgs[i])
    keyname="shadow_" + str(i) + ".fractal_key_version_1"
    os.remove(keyname)

