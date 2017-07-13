Copyright (c) 2013 YR
Copyright (c) 2013 by The University of Warwick

SN-IEgenerator: A package for generating idealized events for SuperNEMO
-----------------------------------------------------------------------

Documentation is provided through the docstrings.

Contents
--------
The SN-IE-generator scripts produce several types of idealized events
for SuperNEMO. These should serve as events data for validation of the 
reconstruction.

Idealized events here mean tracker and calorimeter calibrated data from 
purely geometric objects: lines and helices.

Events from this generator take no account of experimental challenges in 
the two detector types. These are not(!) events using any form of physics 
transport of objects. 

Lines are geometric lines and all cells traversed 
with a distance less than their nominal radius are kept as tracker hits
with the simplest possible radius, i.e. shortest distance of line to 
anode wire. 

Likewise, calorimeter hits are simply determined from 
intersections of the lines with the front surface of the respective 
calorimeter block. No scattering, a single default dummy energy and time
is scored. The main point for validation here is to test whether the 
reconstruction extrapolates to the correct calorimeter block.

Installed geometry event types
------------------------------
- Illumination: single line on random tracker halfs
- Double Left/Right: single line on the each tracker half from single vertex on foil
- V vertex: double line from single vertex at an angle to form a V event in 
  single, random tracker half.
- Single helix: single helix in a random tracker half and constant 25G B-field.
- Double Helix: helix in each tracker half starting at common vertex on foil.
- Single Scatter: single line in random tracker half with a small random location 
  break along it, mimicking multiple scattering in gas with small (3 degr std.dev.) angle 
  scattering at the break.



Requirements
------------
- ROOT 6 (for File I/O)
- NumPy/SciPy (throughout)


Init
----
No installation as such but informing the system of pySNemo existance is required.
Setting the Python path to the package, for instance as:

setenv PYTHONPATH ${PYTHONPATH}:${HOME}/Code/pySNemo/python
