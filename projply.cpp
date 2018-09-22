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

#include <pcl/io/auto_io.h>
#include <pcl/point_types.h>
#include <iostream>
#include <proj_api.h>
#include <algorithm>
#include <math.h>
#include <sstream>
#include <limits>
#include <getopt.h>
#include "projply.h"

double ox,oy,oz;
char *appName;
int shift_output;
char *input;
char *output;
projPJ sourceProj;
projPJ targetProj;

int convert();

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
  char *from=0;
  char *to=0;

  appName=argv[0];

  while(1) {
    static struct option long_options[] = {
      {"shift", no_argument, &shift_output, 1},
      {"input", required_argument, 0, 'i'},
      {"output", required_argument, 0, 'o'},
      {"from", required_argument, 0, 'f'},
      {"to", required_argument, 0, 't'},
      {"help", required_argument, 0, 'h'},
      {"version", no_argument, 0, 'v'},
      {0, 0, 0, 0}
    };

    int option_index = 0;

    c = getopt_long (argc, argv, "si:o:f:t:x:y:z:hv", long_options, &option_index);

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
        from=optarg;
        sourceProj = pj_init_plus(from);
        if (sourceProj==NULL) {
          std::cerr << "invalid parameter: " << from << std::endl;
          return false;
        }
        break;

      case 't':
        to=optarg;
        targetProj = pj_init_plus(to);
        if (targetProj==NULL) {
          std::cerr << "invalid parameter: " << from << std::endl;
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

  if (!input || !output || !from || !to) {
    usage();
  }

  return !convert();
}


int convert() {
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZ>);
  pcl::PLYReader Reader;

  int err=pcl::io::load(input, *cloud);
  if (err) {
    exit(1);
  }
 
  size_t pointCount = cloud->points.size();

  double minx=std::numeric_limits<double>::max();
  double miny=std::numeric_limits<double>::max();
  double minz=std::numeric_limits<double>::max();

  if (shift_output) {
    double *dx=(double*)malloc(pointCount*sizeof(double));
    double *dy=(double*)malloc(pointCount*sizeof(double));
    double *dz=(double*)malloc(pointCount*sizeof(double));

    for (size_t i = 0; i < pointCount; ++i) {
      double x,y,z;
      x=cloud->points[i].x+ox;
      y=cloud->points[i].y+oy;
      z=cloud->points[i].z+oz;
      int p = pj_transform(sourceProj, targetProj, 1, 1, &x, &y, &z );
      if (p) {
        std::cerr << "error: pj_transform returned error code " << p << std::endl;
        return false;
      }
      minx=std::min(minx,x);
      miny=std::min(miny,y);
      minz=std::min(minz,z);
      dx[i]=x;
      dy[i]=y;
      dz[i]=z;
    }

    ox=static_cast<int>(minx/100)*100.0;
    oy=static_cast<int>(miny/100)*100.0;
    oz=static_cast<int>(minz/100)*100.0;

    for (size_t i = 0; i < pointCount; ++i) {
      cloud->points[i].x=dx[i]-ox;
      cloud->points[i].y=dy[i]-oy;
      cloud->points[i].z=dz[i]-oz;
    }

  } else {
    for (size_t i = 0; i < pointCount; ++i) {
      double x,y,z;
      x=cloud->points[i].x+ox;
      y=cloud->points[i].y+oy;
      z=cloud->points[i].z+oz;
      int p = pj_transform(sourceProj, targetProj, 1, 1, &x, &y, &z );
      if (p) {
        std::cerr << "error: pj_transform returned error code " << p << std::endl;
        return false;
      }
      cloud->points[i].x=x;
      cloud->points[i].y=y;
      cloud->points[i].z=z;
    }
  }

  pcl::io::save(output, *cloud);

  std::cout << std::setprecision(std::numeric_limits<double>::digits10 + 1) << "origin " << ox << " " << oy << " " << oz << std::endl;

  return true;

}
