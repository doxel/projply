/*
* Copyright (c) 2018-2019 ALSENET SA
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
#include <proj.h>
#include <algorithm>
#include <math.h>
#include <sstream>
#include <fstream>
#include <limits>
#include <getopt.h>
#include <cstring>
#include "tinyply.h"
#include "projply.h"
#include "ply_io.h"


ProjPly::ProjPly(
  const char *input,
  const char *fromProj,
  const char *toProj,
  const double3 &offset,
  const int shift_output,
  const char *output,
  const int verbose
){
  this->input=(char*)input;
  this->fromProj=(char*)fromProj;
  this->toProj=(char*)toProj;
  this->offset=offset;
  this->shift_output=shift_output;
  this->output=(char*)output;
  this->verbose=verbose;

  initProj();
  readFile();
  proj();
  updateComments();
  save();
};

void ProjPly::readFile(){
  std::vector<RequestedProperties> requestList;
  requestList.push_back(RequestedProperties(&vertices,"vertex",{"x","y","z"}));
  ply_read(input, file, requestList, otherProperties, verbose);
  if (!vertices) throw std::runtime_error(std::string("failed to extract vertice properties from ") + input);
}

void ProjPly::initProj(){
  P = proj_create_crs_to_crs(0, fromProj, toProj, 0);
  if (!P) throw std::runtime_error(std::string("error: proj_trans_generic returned error code ")+proj_errno_string(proj_errno(0)));
}

double3 *ProjPly::proj() {
  if (verbose) std::cerr << "Reprojecting ..." << std::endl;

  switch(vertices->t) {
    case tinyply::Type::FLOAT32:
      std::cerr << "WARNING: " << (input?input:"ply file") << ": you should set the input coordinates property type to \"double\"" << std::endl;
      result=doProj((float3*)vertices->buffer.get());
      break;

    case tinyply::Type::FLOAT64:
      result=doProj((double3*)vertices->buffer.get());
      break;

    default:
      throw std::runtime_error("unhandled vertices numeric type");
      break;
  }

  return result;
}

void ProjPly::updateComments(){
  std::vector<std::string> &comments=file.get_comments();
  comments.push_back(std::string("projply: ")+fromProj+" +to "+toProj);
}

void ProjPly::save(){
  /* TODO: use result to convert point type to double in ouput ply when input ply contains floats */
  if (verbose) std::cerr << "Saving " << output << std::endl;
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
  if (verbose) std::cerr << "Done !" << std::endl;
}

template<class T> double3 *ProjPly::doProj(T points) {
  size_t pointCount=vertices->count;

  double minx=std::numeric_limits<double>::max();
  double miny=std::numeric_limits<double>::max();
  double minz=std::numeric_limits<double>::max();

  double3 *result;
  if (sizeof(points->x)==sizeof(double)) {
    result=(double3*)points;
  } else {
    result=(double3*)malloc(pointCount*sizeof(double3));
  }

  double ox=offset.x;
  double oy=offset.y;
  double oz=offset.z;
  for (size_t i = 0; i < pointCount; ++i) {
    result[i].x=static_cast<double>(points[i].x)+ox;
    result[i].y=static_cast<double>(points[i].y)+oy;
    result[i].z=static_cast<double>(points[i].z)+oz;
  }
  size_t r = proj_trans_generic(
    P,
    PJ_FWD,
    &result[0].x, sizeof(double3), pointCount,
    &result[0].y, sizeof(double3), pointCount,
    &result[0].z, sizeof(double3), pointCount,
    0, 0, 0
  );
  if (!r) {
    throw std::runtime_error(std::string("error: proj_trans_generic returned error code ")+proj_errno_string(proj_errno(P)));
  }

  if (shift_output) {
    for (size_t i = 0; i < pointCount; ++i) {
      minx=std::min(minx,result[i].x);
      miny=std::min(miny,result[i].y);
      minz=std::min(minz,result[i].z);
    }
    if (pointCount) {
      ox=static_cast<long int>(minx/100)*100.0;
      oy=static_cast<long int>(miny/100)*100.0;
      oz=static_cast<long int>(minz/100)*100.0;
    } else {
      ox=oy=oz=0;
    }
    for (size_t i = 0; i < pointCount; ++i) {
      result[i].x-=ox;
      result[i].y-=oy;
      result[i].z-=oz;
    }
  }

  if (sizeof(points->x)!=sizeof(double)) {
    for (size_t i = 0; i < pointCount; ++i) {
      points[i].x=result[i].x;
      points[i].y=result[i].y;
      points[i].z=result[i].z;
    }
  }

  std::cerr << std::setprecision(10) << std::noshowpoint << "origin " << ox << " " << oy << " " << oz << std::endl;
  return result;
}
