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

#include <vector>
#include <utility>
#include <iostream>
#include <sstream>
#include <fstream>
#include "tinyply.h"
#include "ply_io.h"

RequestedProperties::RequestedProperties(std::shared_ptr<tinyply::PlyData> *tinyPlyDataPtr, std::string elementName, std::initializer_list<std::string>properties_list) {
  this->tinyPlyDataPtr=tinyPlyDataPtr;
  this->elementName=elementName;
  this->properties_list=properties_list;
  this->remaining=properties_list.size();
  for (auto prop : properties_list) {
    this->properties.push_back(std::pair<std::string,bool>(prop,false));
  }
}

int ply_read(
  const char *filename,
  tinyply::PlyFile &file, // output file
  std::vector<RequestedProperties> requestedPropertiesList,
  int verbose
) {
  std::vector<std::shared_ptr<tinyply::PlyData>> other_properties;
  return ply_read(filename, file, requestedPropertiesList, other_properties, verbose);
}

int ply_read(
  const char *filename,
  tinyply::PlyFile &file, // output file
  std::vector<RequestedProperties> requestedPropertiesList,
  std::vector<std::shared_ptr<tinyply::PlyData>> &other_properties,
  int verbose
) {

  int total=0;
  std::ifstream ss(filename, std::ios::binary);
  if (verbose) std::cerr << "Opening " << filename << " ..." << std::endl;
  if (ss.fail()) throw std::runtime_error(std::string("failed to open ") + filename);

  file.parse_header(ss);

  for (auto const &e : file.get_elements()) {
    if (verbose) std::cerr << "element - " << e.name << " (" << e.size << ")" << std::endl;
    for (auto p : e.properties) {
      bool found=false;
      if (verbose) std::cerr << "\tproperty - " << p.name << " (" << tinyply::PropertyTable[p.propertyType].str << ")" << std::endl;
      for(auto &req : requestedPropertiesList) {
        if (*req.tinyPlyDataPtr) continue;
        if (e.name==std::string(req.elementName)) {
          for (auto &prop : req.properties) {
            if (p.name==std::string(prop.first)) {
              if (prop.second) throw std::runtime_error(std::string("property defined twice ") + prop.first);
              prop.second=true;
              found=true;
              --req.remaining;
              break;
            }
          }
          if (!req.remaining) {
            if (verbose) {
              std::cerr << "found ";
              int i=req.properties_list.size();
              for (auto const &name : req.properties_list) {
                std::cerr << name << ((--i)?", ":"");
              }
              std::cerr << std::endl;
            }
            *req.tinyPlyDataPtr=file.request_properties_from_element(e.name.c_str(), req.properties_list);
            ++total;
          }
          if (found) break;
        }
      }
      if (!found) {
        other_properties.push_back(file.request_properties_from_element(e.name.c_str(), { p.name.c_str() }));
      }
    }
  }

  if (verbose) std::cerr << "Reading ply..." << std::endl;
  file.read(ss);

  return total;

}
