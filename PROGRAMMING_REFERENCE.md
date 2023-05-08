Table of Contents:
- [1. C/C++](#1-cc)
	- [1.1. API Reference](#11-api-reference)
	- [1.2. Supported Features by Compiler Version](#12-supported-features-by-compiler-version)
	- [1.3. C++ Libraries](#13-c-libraries)
		- [1.3.1. GNU libstdc++](#131-gnu-libstdc)
		- [1.3.2. List of Libraries](#132-list-of-libraries)
		- [1.3.3. GUI](#133-gui)
		- [1.3.4. Intel TBB](#134-intel-tbb)
	- [1.4. Replacing STL allocators:](#14-replacing-stl-allocators)
	- [1.5. Cheat sheets](#15-cheat-sheets)
- [2. Algorithms](#2-algorithms)
	- [2.1. Reference](#21-reference)
	- [2.2. Interviewing and Competitive Programming](#22-interviewing-and-competitive-programming)
- [3. Design Patterns and OO](#3-design-patterns-and-oo)
	- [3.1. UML syntax](#31-uml-syntax)
- [4. Python](#4-python)
	- [4.1. Tutorials](#41-tutorials)
	- [4.2. Cheat Sheets](#42-cheat-sheets)
	- [4.3. profiling](#43-profiling)
	- [4.4. Source Code:](#44-source-code)
	- [4.5. Embedding:](#45-embedding)
	- [4.6. Debugging:](#46-debugging)
	- [4.7. Package issues](#47-package-issues)
- [5. Debugging,Analysis and Performance Tuning](#5-debugginganalysis-and-performance-tuning)
	- [5.1. Automated](#51-automated)
	- [5.2. GDB (Linux or AIX)](#52-gdb-linux-or-aix)
		- [5.2.1. Reverse debugging](#521-reverse-debugging)
	- [5.3. dbx (AIX)](#53-dbx-aix)
	- [5.4. GDB or DBX AIX binary?](#54-gdb-or-dbx-aix-binary)
	- [5.5. Profiling](#55-profiling)
		- [5.5.1. Sample session:](#551-sample-session)
	- [5.6. What is the process doing?](#56-what-is-the-process-doing)
	- [5.7. What is in the executable?](#57-what-is-in-the-executable)
	- [5.8. Signals](#58-signals)
	- [5.9. gcc target options -march vs -mnative](#59-gcc-target-options--march-vs--mnative)
- [6. OS specific](#6-os-specific)
	- [6.1. Linux](#61-linux)
		- [6.1.1. how to install perf on ubuntu](#611-how-to-install-perf-on-ubuntu)
		- [6.1.2. system administration](#612-system-administration)
		- [6.1.3. Core files on ubuntu](#613-core-files-on-ubuntu)
	- [6.2. AIX](#62-aix)
		- [6.2.1. system administration](#621-system-administration)
			- [6.2.1.1. processors and configuration](#6211-processors-and-configuration)
			- [6.2.1.2. Disk expansion](#6212-disk-expansion)
			- [6.2.1.3. Upgrading to a new TL level:](#6213-upgrading-to-a-new-tl-level)
			- [6.2.1.4. Compilers:](#6214-compilers)
		- [6.2.2. Cloud server preparation](#622-cloud-server-preparation)
	- [6.3. Windows](#63-windows)
		- [6.3.1. CLI compilation and linking](#631-cli-compilation-and-linking)
		- [6.3.2. Debugging python modules](#632-debugging-python-modules)
	- [6.4. Containers](#64-containers)
		- [6.4.1. Docker Basic Commands](#641-docker-basic-commands)
		- [6.4.2. podman](#642-podman)
- [7. Unix tips](#7-unix-tips)
	- [7.1. Common Unix Commands in different UNIX OS:](#71-common-unix-commands-in-different-unix-os)
	- [7.2. Why is computer slow?](#72-why-is-computer-slow)
	- [7.3. Save terminal session with tmux](#73-save-terminal-session-with-tmux)
- [8. Editors](#8-editors)
	- [8.1. Emacs](#81-emacs)
		- [8.1.1. .emacs file:](#811-emacs-file)
		- [8.1.2. .dir-locals.el file:](#812-dir-localsel-file)
	- [8.2. visual studio code](#82-visual-studio-code)
- [9. Books](#9-books)
- [10. ICU/Unicode](#10-icuunicode)
- [11. REST APIs/JSON](#11-rest-apisjson)
- [12. SQL](#12-sql)
	- [12.1. Cheat Sheet](#121-cheat-sheet)
- [13. Statistical Analysis and Machine Learning](#13-statistical-analysis-and-machine-learning)
	- [13.1. Fundamentals of ML](#131-fundamentals-of-ml)
	- [13.2. Statistical measures in python:](#132-statistical-measures-in-python)
		- [13.2.1. Plotting:](#1321-plotting)
		- [13.2.2. Fitting:](#1322-fitting)
		- [13.2.3. Sampling:](#1323-sampling)
		- [13.2.4. Learning](#1324-learning)


# 1. C/C++
## 1.1. API Reference
 * CPP: :boom: https://en.cppreference.com/w/cpp
 * CPP: https://www.cplusplus.com/reference/unordered_map/unordered_map/
 * CPP How to Program: https://cs.fit.edu/~mmahoney/cse2050/how2cpp.html
 * C standard library: https://www.cplusplus.com/reference/clibrary/
## 1.2. Supported Features by Compiler Version
 * https://en.cppreference.com/w/cpp/compiler_support
## 1.3. C++ Libraries
### 1.3.1. GNU libstdc++
* Docs: https://gcc.gnu.org/onlinedocs/libstdc++/manual/index.html
* Library test suite for parallel sort: https://gcc.gnu.org/git/?p=gcc.git;a=blob;f=libstdc%2B%2B-v3/testsuite/25_algorithms/pstl/alg_sorting/sort.cc;h=e0c55df404f82dc360cea77ce411511bc2b4e619;hb=HEAD
### 1.3.2. List of Libraries
https://en.cppreference.com/w/cpp/links/libs
### 1.3.3. GUI
* SFML https://www.sfml-dev.org/index.php
* TGUI https://tgui.eu/
### 1.3.4. Intel TBB
Forms the basis of parallel algorithms in latest C++ versions

https://software.intel.com/content/www/us/en/develop/documentation/tbb-documentation/top.html

Introduction: https://www.youtube.com/watch?v=9Otq_fcUnPE

## 1.4. Replacing STL allocators:
* https://gcc.gnu.org/onlinedocs/libstdc++/manual/memory.html
* https://johnysswlab.com/the-price-of-dynamic-memory-allocation/
* 
## 1.5. Cheat sheets
 * Syntax cheat sheet: https://github.com/gibsjose/cpp-cheat-sheet/blob/master/C%2B%2B%20Syntax.md
 * Data Structures and Algorithms cheat sheet: https://github.com/gibsjose/cpp-cheat-sheet/blob/master/Data%20Structures%20and%20Algorithms.md
 * C++ STL quick reference: https://overapi.com/static/cs/STL%20Quick%20Reference%201.29.pdf

# 2. Algorithms
## 2.1. Reference
https://www.geeksforgeeks.org/
## 2.2. Interviewing and Competitive Programming
* :boom: https://www.hackerrank.com/
* https://www.hackerrank.com/philip_miloslav1
# 3. Design Patterns and OO
* SOLID:  https://scotch.io/bar-talk/s-o-l-i-d-the-first-five-principles-of-object-oriented-design
* Catalog: https://en.wikipedia.org/wiki/Software_design_pattern
* Cloud Patterns: https://docs.microsoft.com/en-us/azure/architecture/patterns/
* UML: https://www.guru99.com/uml-cheatsheet-reference-guide.html
## 3.1. UML syntax
<<interface>>  +public  -private    variable: type

Inheritance: from class: solid line black arrow, from abstract class: solid line white arrow, from interface: dashed line white arrow

Aggregation: white diamond

Composition: black diamond

# 4. Python
## 4.1. Tutorials
* :boom: Misc: https://realpython.com/
* python source: https://realpython.com/cpython-source-code-guide/
## 4.2. Cheat Sheets
* https://gto76.github.io/python-cheatsheet/ 
## 4.3. profiling
python -m cProfile -s time circuit.py
## 4.4. Source Code:
Tokens: https://github.com/python/cpython/blob/main/Python/generated_cases.c.h
## 4.5. Embedding:
    Py_Initialize(); 
    if ( !Py_IsInitialized() ) 
    { 
        return -1; 
    } 

    printf("doing import math in libbar.so:\n");
    PyRun_SimpleString("import math");
    printf("import done\n");

    Py_Finalize();
## 4.6. Debugging:
* sudo apt-get install python3-dbg
* sudo wget https://www.python.org/ftp/python/3.8.10/Python-3.8.10.tgz
## 4.7. Package issues
* on windows use py instead of python
* pip freeze   shows everything installed by pip
* pip install  uninstall   cache purge
# 5. Debugging,Analysis and Performance Tuning
## 5.1. Automated
* valgrind
* cachegrind (find cache thrashing)
## 5.2. GDB (Linux or AIX)
* SIGSEGV: http://unknownroad.com/rtfm/gdbtut/gdbsegfault.html
* Cheat sheets: https://darkdust.net/files/GDB%20Cheat%20Sheet.pdf
*               https://cs.brown.edu/courses/cs033/docs/guides/gdb.pdf
* step into s  step over n   finish   dir  b   d 1(delete first breakpoint) 
* p $r3 $eax  p/x $r3  x/8xw 0x110075770+27048 ([x]/(number)(format)(unit_size=bhwg) <address>)
* gdb tui   layout split asm src regs   layout next     focus next
* p    p/x 
* ptype       ptype /o  (offsets)
* p *(struct node *) 0x00000000004004fc
* x addr   x/h addr    x/20h addr
* gdb -batch -ex "disassemble/rs mainsub" mux.o | more
* If CTRL-C crashes gdb -> kill -TRAP <pid> from another window
* Sometimes you should do     (gdb)handle SIGUSR1 pass
* b vec_op_varith<int64_t>
### 5.2.1. Reverse debugging
* There is a record and replay mode:  https://sourceware.org/gdb/onlinedocs/gdb/Process-Record-and-Replay.html
* and there is also native reverse debugging https://www.sourceware.org/gdb/news/reversible.html
* To do reverse debugging you have to do (gdb) record before you do (gdb)rs  (reverse-step) 
## 5.3. dbx (AIX)
* multiproc child
* stop on load "pythonint.so"
* step,next,continue
* stop in PyInit_pythonint
* use /nethome/pmilosla/perforce/projects/python_new/modules/pythonint
* sudo dbx /home/pmilosla/iris/IBMCLANGD/bin/irisdb /home/pmilosla/iris/IBMCLANGD/mgr/user/core
* https://www.ibm.com/docs/en/aix/7.2?topic=overview-register-usage-conventions
* stepi listi stepi 4   p $r21
* (dbx) listi 0x100919884,0x100919994
* https://www.ibm.com/docs/en/aix/7.1?topic=d-dbx-command
## 5.4. GDB or DBX AIX binary?
* To see if binary was compiled with -ggdb do: sudo nm -X64 binary | grep "\.dwstr"
-	For -ggdb, by default it will use non-inline strings, means it will use `.dwstr` section and strings in `.dwinfo` section will use offsets to the `dwstr` section.
-	For -gdbx, by default it will use inline strings, means, it will not use `.dwstr` section, all strings in `.dwinfo` section will be inlined in the `.dwinfo` section.
## 5.5. Profiling
* with perf: https://www.brendangregg.com/FlameGraphs/cpuflamegraphs.html
* with valgrind: https://developer.mantidproject.org/ProfilingWithValgrind.html
### 5.5.1. Sample session:
* start your process doing a heavy loop of what you are interested in
* sudo perf record -F 999 -g -p 2277501 --call-graph dwarf    (creates perf.data file)
* sudo perf report
## 5.6. What is the process doing?
* lsof
* pmap
* truss
* strace
* rlimit files, semaphores, stack size
* ulimit
* pldd  (procldd -F on AIX)
* genld -ld (AIX)
## 5.7. What is in the executable?
* ldd
* file
* objdump   AIX: dump -X64 -t
* strings -a
* restore -qxvf libc++.rte.16.1.0.10.bff to see whats
## 5.8. Signals
* kill -l lists all signals
* echo $? gives 128+signum that says what killed you
* sudo kill -TRAP 10486072 to the process gdb is debugging causes gdb to break
## 5.9. gcc target options -march vs -mnative
* gcc -v
* gcc -dumpmachine
* gcc -march=native -Q --help=target
* gcc -march=aarch64-redhat-linux -Q --help=target
# 6. OS specific
## 6.1. Linux
* Search kernel source: https://elixir.bootlin.com/linux/v5.14.10/source
* Search kernel Docs: https://www.kernel.org/doc/html/latest/search.html
### 6.1.1. how to install perf on ubuntu
* sudo apt-get update
* sudo apt-get dist-upgrade
* sudo apt-get install --reinstall linux-tools-common linux-tools-generic linux-tools-`uname -r`
* sudo perf top
### 6.1.2. system administration
* apt list --installed
* yum update yum install yum-utils  yum provides gtar  yum info tar-1.33-1.ppc repoquery -l tar-1.33-1.ppc
* dnf is replacing yum
* sudo rpm -ivh python3-3.7.11-1.rpm
* yum list installed | grep python
* repoquery -l rh-python38-python-devel
*  dpkg-query -L libssh2-1
*  If you have fatal conflicts you may need to wget packages and do rpm -Uvh *.rpm
### 6.1.3. Core files on ubuntu
* if you dont want to struggle with apport you can do this:
	sudo emacs /etc/security/limits.conf to have:
	*		 -	 core		 -1
	sudo emacs /etc/sysctl.conf
	#kernel.sysrq=438
	# Core pattern (core.<executable>.<pid>.<time of core>)
	kernel.core_pattern = /var/crash/core.%e.%p.%t
	# Controls whether core dumps will append the PID to the core filename.
	kernel.core_uses_pid = 1
	sudo sysctl --system
## 6.2. AIX
### 6.2.1. system administration
* PTFs come from IBM's "Fix Central", while the GA images (with the embedded licenses) come from either Passport Advantage 
  (software sales) or ESS (hardware bundles). https://www.ibm.com/support/pages/fix-list-xl-cc-aix
* command security
```sudo lssecattr -c /usr/sbin/lsattr
1420-012 "/usr/sbin/lsattr" does not exist in the privileged command database.
sudo lssecattr -c /usr/pmapi/tools/pmcycles
/usr/pmapi/tools/pmcycles accessauths=aix.system.pmustat.global innateprivs=PV_PMU_CONFIG,PV_PMU_SYSTEM secflags=FSF_EPS
```
* lslpp -Lqc 
* remove web download pack python sudo installp -u python3.9.base python3.9.test
* remove an MRS package and replace with SP package:
  ```
  Hope you are using smitty installp command.  To downgrade using smitty installp, set below options. Select the “SOFTWARE to install” list by selecting these 3 from the list “openssl.base, openssl.license, openssl.man.en_US”. Bold ones are the ones changed from default values.
  INPUT device / directory for software               .
  SOFTWARE to install                              [openssl.base   ALL  @@I:openssl.base _all_filesets,openssl.license]
  PREVIEW only? (install operation will NOT occur)    no
  COMMIT software updates?                            yes
  SAVE replaced files?                                no
  AUTOMATICALLY install requisite software?           no
  EXTEND file systems if space needed?                yes
  OVERWRITE same or newer versions?                   yes
  VERIFY install and check file sizes?                no
  DETAILED output?                                    no
  Process multiple volumes?                           yes
  ACCEPT new license agreements?                      yes
  Preview new LICENSE agreements?                     yes
  ``` 
* OpenSSL version: You can grep for “grep OPENSSL_CONFIGURED_API config*“ which shows below output
  grep OPENSSL_CONFIGURED_API config*
  define OPENSSL_CONFIGURED_API 10002
Whereas in TL1 SP1 you would see output as 
  grep OPENSSL_CONFIGURED_API config*
  define OPENSSL_CONFIGURED_API 30000
* topas
* nmon   n,t, 0,1,2,3,4
* sudo   slibclean
* df -Pg  disk space left
* oslevel -s (what TL version)
* yum list installed | grep python
* yum install yum-utils   repoquery -l
* yum install emacs-nox
* dnf update
* dnf install emacs-nox
* dnf install dnf-utils
* path for all users is in /etc/environment
* dump -h libcrypto.a | grep :       to see what versions it supports
* errpnt -a
* ar -X64 -tv /usr/lib/libssl.a How to see whats in an AIX archive bundle
#### 6.2.1.1. processors and configuration
* lscfg -lproc\*
* lparstat -i | grep CPU
* also lparstat without args (shows smt4 sometimes)
* bindprocessor -q
* lsattr -El proc0
* sudo pmcycles -m
* sudo smtctl
* lsmcode -c
#### 6.2.1.2. Disk expansion
* lsvg rootvg
* df -g
* lspv
* chfs -a size=1G /opt
* chvg -g rootvg  (to detect growing disk)
* cfgmgr detects new added disks
* smitty jfs2
* mount /scratch
* umount /scratch
* Adding a volume to rootvg:
* chvg -t 16 rootvg
* extendvg -f rootvg hdisk0    (alternative)
* smitty mkusr      or
* mkuser -a pmilosla
* passwd pmilosla
* Expanding /home:
* lslv hd1
* chlv -x 6000 hd1
* chfs -a size=30G /home
* Using datavg:
* mkvg -y datavg -s 128 hdisk2   (the ones not in rootvg)
* mklv -y datalv -t jfs2 datavg 80G
* lslv datalv
* crfs -v jfs2 -d datalv -m /data
* mount /data
* varyoffvg datavg
* exportvg datavg
* rmdev -dl hdisk1
* importvg hdiskN
#### 6.2.1.3. Upgrading to a new TL level:
Terminology:
oslevel -s
7300-00-01-2148
00 is TL
01 is SP
21 is year
48 is the week of the year
 
remove interm fixes (like the openssl patch):
emgr -l
emgr -r -L <label>
emgr -r -L testifix2
 
 
https://www.ibm.com/docs/en/aix/7.2?topic=s-suma-command
http://www.unixmantra.com/2013/09/aix-technology-level-tl-update-methods.html
https://www.ibm.com/docs/en/power7?topic=upgrade-updating-aix-new-technology-level
https://cloud.ibm.com/docs/power-iaas?topic=power-iaas-downloading-fixes-updates
 
 
Before you start and OS upgrade make sure you expand these:
Check in-house systems, they have it expanded even more
 
chlv -x 6000 repo00
chfs -a size=20G /usr/sys/inst.images
chfs -a size=4G /var
chfs -a size=4G /opt
chfs -a size=4G /usr
 
The manual way (dont do this, use smit below)
This does the latest SP but not a new TL level:
date
suma -s "minutes hours * * *" -a RqType=Latest -a DisplayName="Latest fixes"
make it 5 minutes into the future:
suma -s "45 8 * * *" -a RqType=Latest -a DisplayName="Latest fixes"
this will do it every day
 
suma -l
...  until it downloads
 
After you install dont forget to remove the manual way:
suma -d 1
crontab -l root
 
This upgrades to a new TL the easy way:
smit suma
type in     7300-01
 
cd /usr/sys/inst.images/installp/ppc
ls -lart   (make sure you have new files  that suma got)
smitty update_all
shutdown -Fr
#### 6.2.1.4. Compilers:
* preprocessor defines: ibm-clang++_r -dM -E - < /dev/null
* dissasembly /opt/IBM/xlc/16.1.0/exe/dis  produces a .s file silently
### 6.2.2. Cloud server preparation
setenv CLOUD xx.xx.xx.xx
setenv IRIS IRIS-2023.2.0LDEV.133.0-ppc64.tar.gz
ssh -i id_rsa root@$CLOUD
 
openssl version
 
cfgmgr
lsvg
chvg -t 16 rootvg
extendvg -f rootvg hdisk1
chfs -a size=2G /opt
lslv hd1
chlv -x 6000 hd1
chfs -a size=30G /home
chfs -a size=1G /tmp
chfs -a size=+1G /usr
chfs -a size=+1G /var
 
mkuser -a pmilosla
passwd pmilosla
 
from laptop:
cd /home/pmilosla/projects/ibmcloud/ibm_downloads
scp -r -i id_rsa ibm_downloads root@${CLOUD}:/home/pmilosla
scp -r -i id_rsa $IRIS root@${CLOUD}:/home/pmilosla
scp -r -i id_rsa iris.key root@${CLOUD}:/home/pmilosla
 
on cloud machine:
export PATH=$PATH:/opt/freeware/bin
cd /home/pmilosla/ibm_downloads
DONT NEED TO DO DNF ON THERE on AIX73 #  ./dnf_aixtoolbox.sh -y
dnf update  (doesnt work because of openssl)
 
install openssl from bundle you put on there:
uncompress openssl-3.0.7.1000.tar.Z
tar xvf openssl-3.0.7.1000.tar
go to the dir and smitty install
(check accept license)
f10 to exit when done
cat /usr/include/openssl/opensslv.h
 
Install SSL3 ifix from bundle you put on there:
emgr -e testifix2.230116.epkg.Z
 
dnf update
dnf install emacs-nox tar tcsh
/opt/freeware/bin/tar  now exists
## 6.3. Windows
* procmon (https://docs.microsoft.com/en-us/sysinternals/downloads/procmon) can show what ddls are being loaded (you need to filter on pid or name)
* ListDlls  dlls in a process   /cygdrive/c/users/pmilosla/Downloads/ListDlls/Listdlls64.exe python.exe
* process Explorer http://live.sysinternals.com/  procexp for when you cant see pids in task manager
* DLL dependancy walker: https://github.com/lucasg/Dependencies
### 6.3.1. CLI compilation and linking
* open Visual studio native x64 shell
* copy in subordinate DLLs your DLL will need (may need to set PATH for run time DLL search)
* cl -c /Zi /W3 test.cpp
* link /debug /MACHINE:X64 test.obj C:\libfavoritedll.lib
* & 'C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\MSBuild\Current\Bin\MSBuild' .\XXXtest.vcxproj -p:Configuration="Debug" -p:Platform="x64" -maxcpucount
### 6.3.2. Debugging python modules
* Recompile your windows .pyd extension with debug See here: https://docs.microsoft.com/en-us/visualstudio/python/working-with-c-cpp-python-in-visual-studio?view=vs-2022    and here: https://stackoverflow.com/questions/28805401/debugging-my-python-c-extension-lead-to-pythreadstate-get-no-current-thread and here:
https://stackoverflow.com/questions/66162568/lnk1104cannot-open-file-python39-d-lib
* Click on your 3.9.5 python installer and install the debug libraries symbols. Copy python39_d.dll and python39_d.pdb to where your application is. (instance/bin)
* Get the 3.9.5 source code from here for later: https://www.python.org/downloads/source/
* start python3 do os.getpid(), Use procmon to start debugging it since task manager doesnt work https://docs.microsoft.com/en-us/sysinternals/downloads/procmon
## 6.4. Containers
### 6.4.1. Docker Basic Commands
Install docker:
https://docs.docker.com/engine/install/ubuntu/#installation-methods
Hello world doesn’t work at first – eventually it started working for me
```
sudo docker run -it docker/surprise
sudo docker ps -a
sudo docker ls
```


Networking:
https://docs.docker.com/network/network-tutorial-standalone/
```
sudo ip link set dev docker0 up/down
sudo docker exec -it iris ping -c 2 google.com
Sudo docker exec -it iris ip addr show
sudo docker network ls
sudo docker network inspect bridge
sudo docker network inspect host
sudo docker exec -it iris ls -la /usr/irissys/bin/libirisHLL.so
```
### 6.4.2. podman
bash
ps -p $$
. /xxx/.podman_setup
id
cat /etc/subuid
podman info

cap-add is to get gdb to work --cap-add=SYS_PTRACE in particular

podman run -name NAME -it -v /nethome/pmilosla:/nethome/pmilosla --cap-add=all docker_image bash

podman ps
podman rename c54467946b78 NAME

podman exec -it NAME bash
podman start -a -i NAME
podman stop <>
podman start NAME
podman inspect NAME
podman exec -it NAME bash
podman ps -a
podman rm <>

podman cp xxx.tar.gz dd978b62fdd9:/tmp

apt-get update
apt-get install gdb emacs strace

dnf install gdb strace

strace -p $$

# 7. Unix tips
## 7.1. Common Unix Commands in different UNIX OS:
* A Sysadmin's Unixersal Translator (ROSETTA STONE) : http://bhami.com/rosetta.html
## 7.2. Why is computer slow?
* top
* free
* dstat
* iostat
## 7.3. Save terminal session with tmux
* sudo apt install tmux
* tmux
* export TERM=xterm-256color
* setenv TERM xterm-256color
* tmux ls
* tmux attach-session -t 0
* CTRL-B D  (detach)
# 8. Editors
## 8.1. Emacs
* M-: is eval  you can then type what hook
* M-x is execute
* ESC is meta key in helm
* emacs -nw starts emacs in non windowed mode (macs homebrew)
### 8.1.1. .emacs file:
;; Added by Package.el.  This must come before configurations of
;; installed packages.  Don't delete this line.  If you don't want it,
;; just comment it out by adding a semicolon to the start of the line.
;; You may delete these explanatory comments.
(require 'package)
(add-to-list 'package-archives '("melpa" . "http://melpa.org/packages/") t)
(package-initialize)
 
;;(add-hook 'after-init-hook 'global-company-mode)
 
(setq package-selected-packages '(lsp-mode lsp-ui yasnippet treemacs lsp-treemacs helm-lsp
 projectile hydra flycheck company avy which-key helm-xref dap-mode))
 
(when (cl-find-if-not #'package-installed-p package-selected-packages)
  (package-refresh-contents)
  (mapc #'package-install package-selected-packages))
 
;; sample `helm' configuration use https://github.com/emacs-helm/helm/ for details
(helm-mode)
(require 'helm-xref)
(define-key global-map [remap find-file] #'helm-find-files)
(define-key global-map [remap execute-extended-command] #'helm-M-x)
(define-key global-map [remap switch-to-buffer] #'helm-mini)
 
; I become scared of what my emacs is doing with these settings
;(require 'eglot)
;(add-to-list 'eglot-server-programs '((c++-mode c-mode) "clangd"))
;(add-hook 'c-mode-hook 'eglot-ensure)
;(add-hook 'c++-mode-hook 'eglot-ensure)
 
;;ctl-c ctl-s shows syntactic element
(c-add-style "kernel"
         '(
           (tab-width . 8)
           (indent-tabs-mode . t)
           (c-basic-offset . 3)
           (c-offsets-alist . ((statement-block-intro . +)
                   (knr-argdecl-intro . 8)
                   (defun-open . -1000)
                   (defun-close . -1000)
                   (defun-block-intro . 8)
                   (case-label . +)
                   (label . -1000)
                   (statement-cont . +)
                   ))
           (hide-ifdef-mode . nil)
           (c-comment-only-line-offset . 0)
           (comment-column . 64)
           (fci-rule-column . 78)
           ))
 
(setq gc-cons-threshold (* 100 1024 1024)
      read-process-output-max (* 1024 1024)
      treemacs-space-between-root-nodes nil
      company-idle-delay 0.1
      company-minimum-prefix-length 1
      lsp-idle-delay 0.1)  ;; clangd is fast
 
(which-key-mode)
(add-hook 'c-mode-hook 'lsp)
(add-hook 'c++-mode-hook 'lsp)
 
(with-eval-after-load 'lsp-mode
  (add-hook 'lsp-mode-hook #'lsp-enable-which-key-integration)
  (require 'dap-cpptools)
  (yas-global-mode))
 
(custom-set-variables
 ;; custom-set-variables was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(column-number-mode t)
 '(global-display-line-numbers-mode t)
 '(inhibit-startup-screen t)
 ;'(lsp-ui-imenu-colors (quote ("sky blue" "green3")))
 ;'(lsp-ui-peek-enable nil)
 ;'(lsp-ui-sideline-show-hover nil)
 )
(custom-set-faces
 ;; custom-set-faces was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 )
 
(require 'whitespace)
 (setq whitespace-style '(face empty tabs lines-tail trailing))
(global-whitespace-mode t)
 
(dolist (hook '(text-mode-hook))
  (add-hook hook (lambda () (flyspell-mode 1))))


### 8.1.2. .dir-locals.el file:


((c-mode . ((c-file-style . "kernel")))
 (c++-mode . ((c-file-style . "kernel")))
 ;(c++-mode (eval . (message "Set C++ kernel style")))






## 8.2. visual studio code
* view -> command     Markdown All in One Add/update section numbers  open preview to the side
# 9. Books
* (Stroustrop’s paper about C++ evolution) https://dl.acm.org/doi/abs/10.1145/3386320
* Fedor G Pikus Hands on Design Patterns with C++
# 10. ICU/Unicode
* Example code: https://begriffs.com/posts/2019-05-23-unicode-icu.html#changing-case
# 11. REST APIs/JSON
* CRUD up some nouns (create/read/update/delete) (post/get/put/delete) 
# 12. SQL
## 12.1. Cheat Sheet
* https://dataschool.com/learn-sql/sql-cheat-sheet/
# 13. Statistical Analysis and Machine Learning
## 13.1. Fundamentals of ML
* https://github.com/ageron/handson-ml2
## 13.2. Statistical measures in python:
```
popSD = numpy.std(population)

def stdDev(X):

    return variance(X)**0.5

pylab.array or numpy.array.mean()

r_squared:

def rSquared(observed, predicted):

    error = ((predicted - observed)**2).sum()

    meanError = error/len(observed)

    return 1 - (meanError/numpy.var(observed))

If the samples are not random and independent don’t make conclusions……

SEM:

def sem(popSD, sampleSize):

    return popSD/sampleSize**0.5

Gaussian: numpy.random.normal(mu, sigma, size)

Area in standard deviation:

     for numStd in (1, 1.96, 3):

        area = scipy.integrate.quad(gaussian,

                                    mu-numStd*sigma,

                                    mu+numStd*sigma,

                                    (mu, sigma))[0]

        print(' Fraction within', numStd,

              'std =', round(area, 4))
```
### 13.2.1. Plotting:
pylab.plot   pylab.hist   pylab.table
```
    pylab.errorbar(xVals, sizeMeans,

                   yerr = 1.96*pylab.array(sizeSDs), fmt = 'o',

                   label = '95% Confidence Interval')
```
### 13.2.2. Fitting:
```
def genFits(xVals, yVals, degrees):

    models = []

    for d in degrees:

        model = pylab.polyfit(xVals, yVals, d)

 

estYVals = pylab.polyval(model, xVals)
```
### 13.2.3. Sampling:
```
random.sample(population, sampleSize)

def leaveOneOut(examples, method, toPrint = True):

def split80_20(examples):

If the samples are not random and independent don’t make conclusions……

Survivor bias, non response bias, cherry picking
```
### 13.2.4. Learning
Clustering (kNearestNeighbor) is Unsupervised Learning
Classification (Logistic Regresion and k-means(greedy)) is Supervised Learning:
Logistic Regression comes in 2 kinds:

L1: many weights – the point is to drive weights to zero to avoid overfitting if 2 variables are correlated, only one will have weight

L2: spreads weight evenly across correlated variables

```
def minkowskiDist(v1, v2, p):

    """Assumes v1 and v2 are equal-length arrays of numbers

       Returns Minkowski distance of order p between v1 and v2"""

    dist = 0.0

    for i in range(len(v1)):

        dist += abs(v1[i] - v2[i])**p

    return dist**(1.0/p)

 
import sklearn.linear_model

def buildModel(examples, toPrint = True):

    featureVecs, labels = [],[]

    for e in examples:

        featureVecs.append(e.getFeatures())

        labels.append(e.getLabel())

    LogisticRegression = sklearn.linear_model.LogisticRegression

    model = LogisticRegression().fit(featureVecs, labels)

    if toPrint:

        print('model.classes_ =', model.classes_)

        for i in range(len(model.coef_)):

            print('For label', model.classes_[1])

            for j in range(len(model.coef_[0])):

                print('   ', Passenger.featureNames[j], '=',

                      model.coef_[0][j])

    return model
```

Reciever Operating Characteristic

auroc = sklearn.metrics.auc(xVals, yVals, True)

The Area Under Curve (AUC) metric measures the performance of a binary classification.

In a regression classification for a two-class problem using a probability algorithm, you will capture the probability threshold changes in an ROC curve.

Normally the threshold for two class is 0.5. Above this threshold, the algorithm classifies in one class and below in the other class.

Accuracy, Sensitivity, Specificity, Pos. Predictive Value, Neg Predictive Value
