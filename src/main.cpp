/*
* Copyright (c) 2018 ALSENET SA
*
* Author(s):
*
*      Luc Deschenaux <luc.deschenaux@freesurf.ch>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Affero General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Affero General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/




/*
 * Copyright (c) 2018 ALSENET SA
 *
 * Author(s):
 *
 *      Luc Deschenaux <luc.deschenaux@freesurf.ch>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <iostream>
#include <iomanip>
#include <proj_api.h>
#include <algorithm>
#include <math.h>
#include <sstream>
#include <fstream>
#include <limits>
#include <getopt.h>
#include <cstring>
#include "tinyply.h"
#include "projply.h"

double ox,oy,oz;
char *appName;
int shift_output;
int verbose=1;
char *input;
char *output;
projPJ sourceProj;
projPJ targetProj;

char *fromProj;
char *toProj;


void version() {
  std::cerr << appName << " " \
    << " Version " << projply_VERSION_MAJOR << "." << projply_VERSION_MINOR << "." << projply_VERSION_PATCH \
    << ", branch " << projply_GIT_BRANCH << ", commit " << projply_GIT_COMMIT << std::endl;
}

void usage() {
  version();
  std::cerr << "Usage: " << appName << " <options>" << std::endl;
  std::cerr << "Options:" << std::endl;
  std::cerr << "  -i|--input <ply_filename>     input file" << std::endl;
  std::cerr << "  -o|--output <ply_filename>    output file" << std::endl;
  std::cerr << "  -f|--from \"+opt[=arg] ...\"    from cartographic parameters" << std::endl;
  std::cerr << "  -t|--to \"+opt[=arg] ...\"      to cartographic parameters" << std::endl;
  std::cerr << "  -x <offset>                   optional: shift input coordinates" << std::endl;
  std::cerr << "  -y <offset>                   optional: shift input coordinates" << std::endl;
  std::cerr << "  -z <offset>                   optional: shift input coordinates" << std::endl;
  std::cerr << "  -s|--shift                    optional: auto-shift output coordinates" << std::endl;
  exit(1);
}

int main(int argc, char **argv) {
  int c;

  appName=argv[0];

  while(1) {
    static struct option long_options[] = {
      {"shift", no_argument, &shift_output, 1},
      {"input", required_argument, 0, 'i'},
      {"output", required_argument, 0, 'o'},
      {"from", required_argument, 0, 'f'},
      {"to", required_argument, 0, 't'},
      {"help", required_argument, 0, 'h'},
      {"quiet", no_argument, 0, 'q'},
      {"version", no_argument, 0, 'v'},
      {0, 0, 0, 0}
    };

    int option_index = 0;

    c = getopt_long (argc, argv, "si:o:f:t:x:y:z:hvq", long_options, &option_index);

    if (c == -1)
      break;

    switch(c){
      case 'i':
        input=optarg;
        break;

      case 'o':
        output=optarg;
        break;

      case 'f':
        fromProj=optarg;
        sourceProj = pj_init_plus(fromProj);
        if (sourceProj==NULL) {
          std::cerr << "invalid parameter: " << fromProj << std::endl;
          return false;
        }
        break;

      case 't':
        toProj=optarg;
        targetProj = pj_init_plus(toProj);
        if (targetProj==NULL) {
          std::cerr << "invalid parameter: " << toProj << std::endl;
          return false;
        }
        break;

      case 'x':
        ox=atof(optarg);
        break;

      case 'y':
        oy=atof(optarg);
        break;

      case 'z':
        oz=atof(optarg);
        break;

      case 's':
        shift_output=1;
        break;

      case 'v':
        version();
        exit(0);
        break;

      case 'q':
        verbose=0;
        break;

      default:
        usage();
        break;
    }
  }

  /* Print any remaining command line arguments (not options). */
  if (optind < argc) {
    std::cerr << "invalid arguments:";
    while (optind < argc) {
      std::cerr << " " << argv[optind++];
    }
    std::cerr << std::endl;
    usage();
  }

  if (!input || !fromProj || !toProj) {
    usage();
  }

  try {
    projPJ sourceProj=pj_init_plus(fromProj);
    if (sourceProj==NULL) throw std::runtime_error(std::string("Invalid projection: ")+fromProj);

    projPJ targetProj=pj_init_plus(toProj);
    if (targetProj==NULL) throw std::runtime_error(std::string("Invalid projection: ")+toProj);

    double3 offset;
    offset.x=ox;
    offset.y=oy,
    offset.z=oz;
    ProjPly(input,fromProj,toProj,offset,shift_output,output,verbose);

    return 0;

  } catch(std::runtime_error &e) {
    std::cerr << "error: " << e.what() << std::endl;
    return 1;
  }
}
