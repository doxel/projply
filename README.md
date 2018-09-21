# projply

Convert georeferenced pointclouds from one geographic coordinate system to another using the proj library

# Dependencies

You need at least:

* libpcl-1.8
* libeigen3
* libboost-system
* libproj

```
sudo apt-get install libpcl-dev libeigen3-dev libboost-dev libproj-dev
```

## Build and Install

```
mkdir build
cd build
cmake ..
make
sudo make install
```
## Synopsis
```
Usage: projply <options>
Options:
  -i|--input <ply_filename>     input file
  -o|--output <ply_filename>    output file
  -f|--from "+opt[=arg] ..."    from cartographic parameters
  -t|--to "+opt[=arg] ..."      to cartographic parameters
  -x <offset>                   optional: shift input coordinates
  -y <offset>                   optional: shift input coordinates
  -z <offset>                   optional: shift input coordinates
  -s|--shift                    optional: auto-shift output coordinates
```

## Example

```
projply -i mn95.ply -o geocentric.ply --from '+init=epsg:2056' --to '+init=epsg:4932' --shift
```

## COPYRIGHT

```
 Copyright (c) 2018 ALSENET SA

 Author(s):

      Luc Deschenaux <luc.deschenaux@freesurf.ch>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
```

