#!/usr/bin/env python3
import subprocess
from ctypes import *

# class Point(Structure):
#      _fields_ = [ ('x',c_double), ('y',c_double), ('z',c_double) ]
# p = Point(2,3.5,6)
# f = open("foo","wb")
# f.write(p)       
# f.close()
# g = open("foo","rb")
# q = Point()
# g.readinto(q)
# g.close()
# print(q.x,q.y,q.z)


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

g = open("C:/Users/Philip/Documents/GitHub/demo/fractals/fractals_cuda/x64/Release/movie_base.fractal_key_version_1","rb")
q = SavedFractal()
g.readinto(q)
g.close()

print("fractal_key:")
for field_name, field_type in q._fields_:
    print(field_name, getattr(q, field_name))

q.current_max_iters0=100

f = open("C:/Users/Philip/Documents/GitHub/demo/fractals/fractals_cuda/x64/Release/movie_base_mod.fractal_key_version_1","r+b")
f.seek(0)
f.write(q)
f.close()

g = open("C:/Users/Philip/Documents/GitHub/demo/fractals/fractals_cuda/x64/Release/movie_base_mod.fractal_key_version_1","rb")
q = SavedFractal()
g.readinto(q)
g.close()

print("modified fractal_key:")
for field_name, field_type in q._fields_:
    print(field_name, getattr(q, field_name))

subprocess.run(['fractals_cuda.exe', 'save_and_exit', 'movie_base_mod.fractal_key_version_1','movie1.png'], 
                shell=True, cwd='C:/Users/Philip/Documents/GitHub/demo/fractals/fractals_cuda/x64/Release')