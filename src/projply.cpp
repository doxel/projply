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

  readFile();
  initProj();
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
  sourceProj=pj_init_plus(fromProj);
  if (sourceProj==NULL) throw std::runtime_error(std::string("Invalid projection: ")+fromProj);

  targetProj=pj_init_plus(toProj);
  if (sourceProj==NULL) throw std::runtime_error(std::string("Invalid projection: ")+toProj);
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
  if (sizeof(points->x)==sizeof(double3)) {
    result=(double3*)points;
  } else {
    result=(double3*)malloc(pointCount*sizeof(double3));
  }

  double ox=offset.x;
  double oy=offset.y;
  double oz=offset.z;
  if (shift_output) {
    for (size_t i = 0; i < pointCount; ++i) {
      double *x=&result[i].x;
      double *y=&result[i].y;
      double *z=&result[i].z;
      *x=static_cast<double>(points[i].x)+ox;
      *y=static_cast<double>(points[i].y)+oy;
      *z=static_cast<double>(points[i].z)+oz;
      int p = pj_transform(sourceProj, targetProj, 1, 1, x, y, z );
      if (p) {
        throw std::runtime_error(std::string("error: pj_transform returned error code ")+std::to_string(p));
      }
      minx=std::min(minx,*x);
      miny=std::min(miny,*y);
      minz=std::min(minz,*z);
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

  } else {
    for (size_t i = 0; i < pointCount; ++i) {
      double *x=&result[i].x;
      double *y=&result[i].y;
      double *z=&result[i].z;
      *x=static_cast<double>(points[i].x)+ox;
      *y=static_cast<double>(points[i].y)+oy;
      *z=static_cast<double>(points[i].z)+oz;
      int p = pj_transform(sourceProj, targetProj, 1, 1, x, y, z );
      if (p) {
        throw std::runtime_error(std::string("error: pj_transform returned error code ")+std::to_string(p));
      }
    }
  }

  if (sizeof(points->x)==sizeof(float3)) {
    for (size_t i = 0; i < pointCount; ++i) {
      points[i].x=result[i].x;
      points[i].y=result[i].y;
      points[i].z=result[i].z;
    }
  }

  std::cerr << std::setprecision(10) << std::noshowpoint << "origin " << ox << " " << oy << " " << oz << std::endl;
  return result;
}
