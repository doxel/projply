# projply

Convert georeferenced pointclouds from one coordinate reference system
to another using a PROJ4 pipeline, avoiding using an intermediate coordinate
reference system whenever possible.

# Dependencies

You need at least:

* git
* cmake
* pkg-config
* libproj

```
sudo apt-get install git cmake pkg-config libproj-dev
```

And optionally docker (recommended for the ease to use the latest PROJ4)

```
sudo apt-get install docker-ce
```

## Build and Install locally

```
mkdir build
cd build
cmake ..
make
sudo make install
```

## Build on top of the osgeo/proj docker container
The bin/projply script will run projply in the docker container transparently
mounting as volume the directories specified for --input and --output plys,
so the command syntax does not change.

```
make && sudo make install
```

## Synopsis
```
Usage: projply <options>
Options:
  -i|--input <ply_filename>     input file
  -o|--output <ply_filename>    output file (optional)
  -f|--from "opt[=arg] ..."     from coordinate reference system
  -t|--to "opt[=arg] ..."       to coordinate reference system
  -x <offset>                   optional: shift input coordinates
  -y <offset>                   optional: shift input coordinates
  -z <offset>                   optional: shift input coordinates
  -s|--shift                    optional: auto-shift output coordinates
```

## Example

```
projply -i mn95.ply -o geocentric.ply --from init=epsg:2056 --to init=epsg:4932 --shift
```

## COPYRIGHT

```
 Copyright (c) 2018-2019 ALSENET SA

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

