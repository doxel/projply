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

#include "tinyply.h"
#include <iostream>
#include <iomanip>
#include <proj_api.h>
#include <algorithm>
#include <math.h>
#include <sstream>
#include <fstream>
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

struct float3 { float x, y, z; };
struct double3 { double x, y, z; };


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
          std::cerr << "invalid parameter: " << to << std::endl;
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

  if (!input || !from || !to) {
    usage();
  }

  return !convert();
}


template <class T>
double3 *reproj(T points, size_t point_size, size_t pointCount) {

  double minx=std::numeric_limits<double>::max();
  double miny=std::numeric_limits<double>::max();
  double minz=std::numeric_limits<double>::max();

  double3 *result;
  if (point_size==sizeof(double3)) {
    result=(double3*)points;
  } else {
    result=(double3*)malloc(pointCount*sizeof(double3));
  }

  if (shift_output) {

    for (size_t i = 0; i < pointCount; ++i) {
      double *x=&result[i].x;
      double *y=&result[i].y;
      double *z=&result[i].z;
      *x=((double)points[i].x)+ox;
      *y=((double)points[i].y)+oy;
      *z=((double)points[i].z)+oz;
      int p = pj_transform(sourceProj, targetProj, 1, 1, x, y, z );
      if (p) {
        throw std::runtime_error(std::string("error: pj_transform returned error code ")+std::to_string(p));
      }
      minx=std::min(minx,*x);
      miny=std::min(miny,*y);
      minz=std::min(minz,*z);
    }

    ox=static_cast<long int>(minx/100)*100.0;
    oy=static_cast<long int>(miny/100)*100.0;
    oz=static_cast<long int>(minz/100)*100.0;

    for (size_t i = 0; i < pointCount; ++i) {
      result[i].x-=ox;
      result[i].y-=oy;
      result[i].z-=oz;
    }

  } else {
    for (size_t i = 0; i < pointCount; ++i) {
      double *x=&result[i].x;
      double *y=&result[i].y;
      double *z=&result[i].z;
      *x=((double)points[i].x)+ox;
      *y=((double)points[i].y)+oy;
      *z=((double)points[i].z)+oz;
      int p = pj_transform(sourceProj, targetProj, 1, 1, x, y, z );
      if (p) {
        throw std::runtime_error(std::string("error: pj_transform returned error code ")+std::to_string(p));
      }
    }
  }

  if (point_size==sizeof(float3)) {
    for (size_t i = 0; i < pointCount; ++i) {
      points[i].x=result[i].x;
      points[i].y=result[i].y;
      points[i].z=result[i].z;
    }
  }

  std::cerr << std::setprecision(10) << std::noshowpoint << "origin " << ox << " " << oy << " " << oz << std::endl;
  return result;
}

int convert() {
  std::ifstream ss(input, std::ios::binary);
  if (ss.fail()) throw std::runtime_error(std::string("failed to open ") + input);

  std::cerr << "Opening " << input << " ..." << std::endl;
  tinyply::PlyFile file;
  file.parse_header(ss);

  for (auto e : file.get_elements()) {
    std::cerr << "element - " << e.name << " (" << e.size << ")" << std::endl;
    for (auto p : e.properties) {
      std::cerr << "\tproperty - " << p.name << " (" << tinyply::PropertyTable[p.propertyType].str << ")" << std::endl;
      if (p.name==std::string("x") || p.name==std::string("y") || p.name==std::string("z")) {
        if (e.name==std::string("vertex")) continue;
      }
      (void)file.request_properties_from_element(e.name.c_str(), { p.name.c_str() }); 
    }

  }

  std::shared_ptr<tinyply::PlyData> vertices;

  try { vertices = file.request_properties_from_element("vertex", { "x", "y", "z" }); }
  catch (const std::exception & e) { std::cerr << "error: " << input << ": " << e.what() << std::endl; return false; }

  std::cerr << "Reading ..." << std::endl;
  file.read(ss);


  std::cerr << "Reprojecting ..." << std::endl;
  double3 *result;

  switch(vertices->t) {
    case tinyply::Type::FLOAT32:
      result=reproj((float3*)vertices->buffer.get(),sizeof(float3),vertices->count);
      break;
    case tinyply::Type::FLOAT64:
      result=reproj((double3*)vertices->buffer.get(),sizeof(double3),vertices->count);
      break;
    default:
      throw std::runtime_error("unhandled vertices numeric type");
      break;
  }

  /* TODO: use result to convert point type to double in ouput ply when input ply contains floats */

  std::cerr << "Saving " << output << std::endl;
  if (output) {
    std::filebuf fb_ascii;
    fb_ascii.open(output, std::ios::out);
    std::ostream outstream_ascii(&fb_ascii);
    if (outstream_ascii.fail()) throw std::runtime_error(std::string("failed to open ") + output + " for writing");
    outstream_ascii << std::fixed;
    file.write(outstream_ascii,false);
  } else {

    std::cout << std::fixed;
    file.write(std::cout,false);
  }
  std::cerr << "Done !" << std::endl;



  return true;

}
