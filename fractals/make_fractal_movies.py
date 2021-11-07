#!/usr/bin/env python3
import os
import math
import subprocess
from ctypes import *
from PIL import Image
import glob
import moviepy.video.io.ImageSequenceClip

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
     ('xstart',c_double),
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
key_version=".fractal_key_version_1"
seedkey="movie_base" + key_version
changedkey = "changed_key" + key_version
base_frame_name="shadow_"
png_basename="movie"
finalname="shadow"

g = open(seedkey,"rb")
q = SavedFractal()
g.readinto(q)
g.close()

print("\n\nBase fractal_key:")
for field_name, field_type in q._fields_:
    print(field_name, getattr(q, field_name))

def create_shadow_frames(end, seedkey, hide=False):
    count = end - 1
    if end > 50:
        if (end - 1) % 50 != 0:
            return

    g = open(seedkey,"rb")
    basekey = SavedFractal()
    g.readinto(basekey)
    g.close()

    basename=base_frame_name

    for j in range(0,end):
        # get the basekey 
        if j !=0 :
            g = open(changedkey,"rb")
            basekey = SavedFractal()
            g.readinto(basekey)
            g.close()

        keyname=basename + str(j) + key_version
        f = open(keyname,"wb")
        #basekey.current_max_iters0=1500
        #basekey.current_escape_r=50

        # if you zoom it will change xstart so be careful

        # pan (fractal coordinates)
        basekey.xstart = basekey.xstart + 5*basekey.xdelta
        
        # zoom
        # if count > 50:
        #     basekey.requested_zoom=basekey.requested_zoom/(1.0 + (1/50)) #zoom in fast
        # else:
        #     basekey.requested_zoom=basekey.requested_zoom/(1.0 + (0.1/count)) #zoom in slowly
        

        # rotate light
        if end > 50:
            angle=(end/50)*j*2*math.pi/(count) # several light rotations
        else:
            angle=j*2*math.pi/(count) # one light rotation

        #angle=j*2*math.pi/(count) # full circle around full mandelbrot
        radius=2
        x = radius * math.cos(angle)
        y = radius * math.sin(angle)
        basekey.light_pos_r = x
        basekey.light_pos_i = y

        f.write(basekey)
        f.close()
        pngname=png_basename + str(j) + ".png"
        if (hide == False):
            subprocess.run(['fractals_cuda.exe', 'save_and_exit', keyname, pngname], shell=True)
        else:
            subprocess.run(['fractals_cuda.exe', 'save_and_exit', keyname, pngname, "hide"], shell=True)
        
        # this exports the new basekey into a file


def create_gif(end, time):
    # Create the frames
    frames = []
    imgs = sorted(glob.glob(png_basename+"*.png"), key=os.path.getmtime)
    print(imgs)

    for i in imgs:
        new_frame = Image.open(i)
        frames.append(new_frame)

    # Save into a GIF file that loops forever
    # duration -> display time of each frame in ms
    frames[0].save(finalname + ".gif", format='GIF',
               append_images=frames[0:len(imgs)],
               save_all=True,
               duration=time, loop=0)    

def create_movie(end, fps, basename):
    imgs = sorted(glob.glob(png_basename+"*.png"), key=os.path.getmtime)
    clip = moviepy.video.io.ImageSequenceClip.ImageSequenceClip(imgs, fps=fps)
    clip.write_videofile(basename + ".mp4")

#total frames to go around circle with light
total_frames=201
if total_frames > 6:
    create_shadow_frames(total_frames, seedkey, True)
else:
    create_shadow_frames(total_frames, seedkey)

create_gif(total_frames, 100)

create_movie(total_frames, 5, finalname + "_slow")
create_movie(total_frames, 30, finalname + "_fast")

imgs = sorted(glob.glob(png_basename+"*.png"), key=os.path.getmtime)
for i in range(len(imgs)):
    os.remove(imgs[i]) 
    keyname=base_frame_name + str(i) + key_version
    os.remove(keyname)