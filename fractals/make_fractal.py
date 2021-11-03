#!/usr/bin/env python3
import subprocess
from ctypes import *

class Point(Structure):
     _fields_ = [ ('x',c_double), ('y',c_double), ('z',c_double) ]
p = Point(2,3.5,6)
f = open("foo","wb")
f.write(p)       
f.close()
g = open("foo","rb")
q = Point()
g.readinto(q)
g.close()
print(q.x,q.y,q.z)

subprocess.run(['fractals_cuda.exe', 'save_and_exit', 'movie_base.fractal_key_version_1','movie1.png'], 
                shell=True, cwd='C:/Users/Philip/Documents/GitHub/demo/fractals/fractals_cuda/x64/Release')