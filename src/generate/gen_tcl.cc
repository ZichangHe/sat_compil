/****************************************************************************
 * Copyright (C) 2017 by Juexiao Su                                         *
 *                                                                          *
 * This file is part of QSat.                                               *
 *                                                                          *
 *   QSat is free software: you can redistribute it and/or modify it        *
 *   under the terms of the GNU Lesser General Public License as published  *
 *   by the Free Software Foundation, either version 3 of the License, or   *
 *   (at your option) any later version.                                    *
 *                                                                          *
 *   QSat is distributed in the hope that it will be useful,                *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU Lesser General Public License for more details.                    *
 *                                                                          *
 *   You should have received a copy of the GNU Lesser General Public       *
 *   License along with QSat.  If not, see <http://www.gnu.org/licenses/>.  *
 ****************************************************************************/

#include "generate/gen_tcl.hh"
#include "generate/system_gen.hh"
#include "qpar/qpar_system.hh"
#include "utils/qlog.hh"

std::string QCOMMAND_generate::help() const {
  const std::string msg = "generate";
  return msg;
}

int QCOMMAND_generate::execute(int argc, const char** argv, std::string& result, ClientData clientData) {

  result = "OK";

  if (!checkOptions(argc, argv)) {
    printHelp();
    return TCL_OK;
  }

  ParNetlist* netlist = ParSystem::getParSystem()->getParNetlist();
  DeviceGen gen(netlist);
  gen.doGenerate();
  gen.dumpDwaveConfiguration("dwave.config");
  qlog.speak("Generate", "Ground Energy is %4.4f", gen.getGroundEnergy());

  return TCL_OK;

}
