Copyright (c) 2013 YR
Copyright (c) 2013 by The University of Warwick

SN-IEgenerator: A package for generating idealized events for SuperNEMO
-----------------------------------------------------------------------

Documentation is provided through the docstrings.

Contents
--------
The SN-IE-generator scripts produce several types of idealized events
for SuperNEMO. These should serve as events data for validation of the 
reconstruction. Note that they output ROOT files containing a single TTree
with branches holding std::vectors of double or int.

Idealized events here mean tracker and calorimeter calibrated data from 
purely geometric objects: lines and helices.

Events from this generator take no account of experimental challenges in 
the two detector types. These are not(!) events using any form of physics 
transport of particles.

Lines are geometric lines and all cells traversed 
with a distance less than their nominal radius are kept as tracker hits
with the simplest possible radius, i.e. shortest distance of line to 
anode wire. 

Likewise, calorimeter hits are simply determined from 
intersections of the lines with the front surface of the respective 
calorimeter block. The main point for validation here is to test whether the 
reconstruction extrapolates to the correct calorimeter block.

The geometry is faithful to the SuperNEMO geometry V4.0 in placing all the 
tracker wire cells and calorimeter blocks. Note that these use the hard-coded
numbers in 3D geometry, nothing 'automatic'. Given that these events are to 
test and validate reconstruction algorithms on idealized events, no perfect
matching to reality is required. Algorithms should do extremely well on 
these events, ideally achieving close to and including 100% perfect
reconstruction since these are very simple events. Fully simulated events 
can only get worse. That is the idea behind this generator.

Note that all missing information for a proper Falaise calibrated hit needs
to be set to some default value, like calo energy and time. There are no
dummy entries for all these in the ROOT output file. Again, the simpler
the better for the purpose of reconstruction validation on these simple geometric events.

FLTranslate is a small application to translate the simulated toy data sets to
Falaise BRIO files which then contain filled 'Calibrated Data' data banks
as required for reconstruction modules. The application needs to be built
with cmake using the Bayeux and Falaise libraries, similar to building
FLSimulate. Once built, try ./build/fltranslate --help to see the minimal
command line options. The resulting BRIO files should then be able to serve as 
input files for reconstruction modules. This currently hasn't been tested but
the translation itself appears to work.


Installed geometry event types
------------------------------
- Illumination: single line on random tracker halfs
- Double Left/Right: single line on the each tracker half from single vertex on foil
                     no slope in xz plane, i.e. flat in xy plane only.
- V vertex: double line from single vertex at an angle to form a V event in 
            single, random tracker half. The angle is permitted to be small such that 
            the two lines can overlap and appear as one track, hitting a single calo.
- Single helix: single helix in a random tracker half and constant 25G B-field.
- Double Helix: helix in each tracker half starting at common vertex on foil.
- Single Scatter: single line in random tracker half with a small random location 
  break along it, mimicking multiple scattering in gas with small (3 degr std.dev.) angle 
  scattering at the break.

More types can be constructed from the examples above and inspecting the geometric object
creation functions in the multiline.py script.


Requirements
------------
- ROOT 6 (for File I/O)
- NumPy/SciPy (throughout)


Init
----
No installation as such but informing the system of pySNemo existance is required.
Setting the Python path to the package, for instance as:

setenv PYTHONPATH ${PYTHONPATH}:${HOME}/Code/Sandbox/github/supernemo/SN-IEgenerator


ROOT output file description
----------------------------
TTree with 25 branches, name "hit_tree", default storage in /tmp/filename.tsim

The calo entries are all integers, tagging each single calorimeter detector
similar to a geom id in Falaise.

calo_id: counter

calo_row: counter of block in wall

calo_column: see calo_row

calo_type: 0 is the main-wall, 1 the x-wall and 2 the gamma veto

calo_side: 0 is left of foil, i.e. negative x-coordinate;
           1 is right with positive x-coordinates

calo_wall: -1 for the main-wall = has no meaning; 
           0 and 1 for the x-wall and gamma veto with: 
           0 - negative y or z coordinates respectively; 
           1 - for the positive y or z coordinates

pointx,y,z: are the impact coordinates on the calorimeter block face

---
the outgoing object, line or helix, is recorded at its start
with direction vector coordinates

dirx,y,z: line slopes in xy and xz planes with dirx=1 by construction; 
          helix momentum vector coordinates in x=0 plane

charge: 0 for line; 1 or -1 for helix

---
tracker cell data is collected in double and int numbers

wirex,y,z: the anode wire coordinate of a triggered cell with
           z coordinate determined from shortest geometric
           distance to gometry object passing by.

radius: the shortest geometric distance of the geometry object
        to the wire anode coordinate as measured in 3D.

grid_id: counter

grid_layer: 0-8 the tracker layers

grid_column: 113 are in the tracker

grid_side: 0 left tracker, 1 right tracker


BRIO file from fltranslate:
---------------------------
Contains only a filled 'CD' data bank of calibrated hit objects
for reconstruction modules from translating the toy simulation
results. There are calibrated tracker hits in collections as well
as corresponding calibrated calorimeter hits, each with their 
required data, hit id's and geometry id's.
