#!/usr/bin/env python3
import os
import math
import sys
import subprocess
import argparse
from ctypes import *
from tabnanny import filename_only
from PIL import Image
import glob
import moviepy.video.io.ImageSequenceClip

class SavedFractal(Structure):
    """Shared between C and Python via ctypes and a .key file. Controls Fractal Generation. This is sensitive to small changes in the C code."""
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
    # Reference Frame - see C code for details
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
    ]


def os_setup():
    """Go to where the fractals generator is."""
    #os.chdir(r"Documents/GitHub/demo/fractals/fractals_cuda/x64/Release")
    os.chdir(r"fractals_cuda/x64/Release")
    print("Currently in dir: ", os.getcwd())

def read_fractal_key(input_key_name):
    """Use ctypes to create a python data structure from the binary file."""
    g = open(input_key_name,"rb")
    output_key = SavedFractal()
    g.readinto(output_key)
    g.close()
    return output_key

def fileio_setup():
    """Set up input files, output files and temporary filenames."""
    global key_version, seedkey_name, changedkey_name, png_basename, frame_basename, final_basename, key_location
    key_version = ".fractal_key_version_1"
    # full file names
    seedkey_name="movie_base" + key_version
    changedkey_name = "changed_key" + key_version
    # partial file names
    frame_basename="shadow_"
    png_basename="movie"
    final_basename="shadow"
    key_location="../../../"
    
    fkey = read_fractal_key(key_location + seedkey_name)
    print("\n\n{} data layout from ctypes:".format(key_version))
    for field_name, field_type in fkey._fields_:
        print(field_name, getattr(fkey, field_name))

def create_evolved_frames():
    count = args.tf

    currentkey = read_fractal_key(key_location + seedkey_name)

    for j in range(count):
        """Create each png frame of the gif"""
        if j != 0 :
            currentkey = read_fractal_key(changedkey_name)
            print("Reading key: ",changedkey_name)

        framekey_name=frame_basename + str(j) + key_version
        f = open(key_location + framekey_name,"wb")

        # if you zoom it will change xstart so be careful

        # pan (fractal coordinates)
        #changedkey.xstart = changedkey.xstart + 5*changedkey.xdelta
        
        # zoom
        currentkey.requested_zoom=currentkey.requested_zoom/(1.0 + (args.z/count))

        # rotate light
        angle=(args.lr)*j*2*math.pi/(count) # several light rotations
        
        # light location in mandelbrot
        radius=2
        x = radius * math.cos(angle)
        y = radius * math.sin(angle)
        currentkey.light_pos_r = x
        currentkey.light_pos_i = y

        f.write(currentkey)
        f.close()
        print("Wrote evolved key: ",key_location + framekey_name)
        pngname=png_basename + str(j) + ".png"
        hide = "hide"   # hide the C++ GUI
        key_name = key_location + framekey_name
        print("Running fractal generation from : ",key_name)
        subprocess.run(['fractals_cuda.exe', 'save_and_exit', key_name, pngname, hide], shell=True)
        # this exports the new changedkey into a file


def create_gif():
    # Create the frames
    frames = []
    imgs = sorted(glob.glob(png_basename+"*.png"), key=os.path.getmtime)
    print(imgs)

    for i in imgs:
        new_frame = Image.open(i)
        frames.append(new_frame)

    # Save into a GIF file that loops forever
    # duration -> display time of each frame in ms
    frames[0].save(final_basename + ".gif", format='GIF',
               append_images=frames[0:len(imgs)],
               save_all=True,
               duration=args.tg, loop=0)    

def create_movie(fps, movie_name):
    imgs = sorted(glob.glob(png_basename+"*.png"), key=os.path.getmtime)
    clip = moviepy.video.io.ImageSequenceClip.ImageSequenceClip(imgs, fps=fps)
    clip.write_videofile(movie_name + ".mp4")

def main():
    global args
    parser = argparse.ArgumentParser(description="Create gif and 2 movies from fractal evolution")
    parser.add_argument("lr", type=float, help="total light full cricle rotations")
    parser.add_argument("z", type=float, help="zoom in z times")
    parser.add_argument("tf", type=int, help="total frames")
    parser.add_argument("--tg", type=int, default=20,  help="time in seconds for gif")    
    args = parser.parse_args()
    
    for i in range(len(sys.argv)):
        print(sys.argv[i])
        
    os_setup()
    fileio_setup()
 

    #starts with movie_base.fractal_key_version_1 in demo/fractals directory
    #finishes with 3 output files in fractals_cuda/x64/Release
    # check into top level dir if they look good
    
    create_evolved_frames()

    create_gif(args.tg)

    create_movie(5, final_basename + "_slow")
    create_movie(30, final_basename + "_fast")

    imgs = sorted(glob.glob(png_basename+"*.png"), key=os.path.getmtime)
    for i in range(len(imgs)):
        os.remove(imgs[i]) 
        framekey_name=key_location + frame_basename + str(i) + key_version
        os.remove(framekey_name)
    os.remove(changedkey_name)

if __name__=="__main__":
    main()