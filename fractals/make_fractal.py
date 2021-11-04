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
     #('version',c_int),
     ('valid',c_int),
     ('current_fractal',c_uint),
     ('current_max_iters',c_uint*3), #array
     ('current_power',c_double),
     ('current_zconst',c_double*2), #complex
     ('current_escape_r',c_double),
     ('wtf',c_uint*40)
     ]

g = open("C:/Users/Philip/Documents/GitHub/demo/fractals/fractals_cuda/x64/Release/movie_base.fractal_key_version_1","rb")
q = SavedFractal()
g.readinto(q)
g.close()

print("fractal_key:")
print(q.valid,q.current_fractal,q.current_max_iters[0],q.current_max_iters[1],q.current_max_iters[2],
      q.current_power,q.current_zconst[0],q.current_zconst[1],q.current_escape_r)

q.current_max_iters[0]=100

f = open("C:/Users/Philip/Documents/GitHub/demo/fractals/fractals_cuda/x64/Release/movie_base_mod.fractal_key_version_1","r+b")
f.seek(0)
f.write(q)
f.close()

g = open("C:/Users/Philip/Documents/GitHub/demo/fractals/fractals_cuda/x64/Release/movie_base_mod.fractal_key_version_1","rb")
q = SavedFractal()
g.readinto(q)
g.close()

print("modified fractal_key:")
print(q.valid,q.current_fractal,q.current_max_iters[0],q.current_max_iters[1],q.current_max_iters[2],
      q.current_power,q.current_zconst[0],q.current_zconst[1],q.current_escape_r)

subprocess.run(['fractals_cuda.exe', 'save_and_exit', 'movie_base_mod.fractal_key_version_1','movie1.png'], 
                shell=True, cwd='C:/Users/Philip/Documents/GitHub/demo/fractals/fractals_cuda/x64/Release')