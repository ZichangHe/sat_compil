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

#include "hw_target/hw_target.hh"
#include "hw_target/hw_object.hh"
#include "hw_target/hw_param.hh"

#include "utils/qlog.hh"

HW_Target_abstract::~HW_Target_abstract() {
  for (Q_ITER qiter = _loc_to_qubit.begin(); 
        qiter != _loc_to_qubit.end(); ++qiter) {
    delete qiter->second;
  }

  _loc_to_qubit.clear();

  for (I_ITER iter = _loc_to_interaction.begin();
        iter != _loc_to_interaction.end(); ++iter)
    delete iter->second;

  _loc_to_interaction.clear();

}

HW_Target_Dwave* HW_Target_Dwave::_self = NULL;


void HW_Target_Dwave::initializeTarget() {
  
  COORD x_num = _hw_param->getMaxRangeX();
  COORD y_num = _hw_param->getMaxRangeY();
  _maxX = x_num;
  _maxY = y_num;


  //1) initialize all qubits and interactions in cells
  for (COORD x = 0; x < x_num; ++x) {
    for (COORD y = 0; y < y_num; ++y) {
      HW_Cell* cell = new HW_Cell(x, y, this);
      _loc_to_cell.insert(std::make_pair(std::make_pair(x, y), cell));
    }
  }

  //2) initialize inter-cell interactions
  for (COORD x = 0; x < x_num; ++x) {
    for (COORD y = 0; y < y_num; ++y) {
      //2.1) build vertical cells
      if (y != (y_num - 1))
        buildInterCellInteractions(x, y, x, y + 1, true);
      //2.2) build horizontal cells
      if (x != (x_num - 1))
        buildInterCellInteractions(x, y, x + 1, y, false);
    }
  }

  qlog.speak("HW_Target", "Target has been initialized with %ld x %ld cells, %lu qubits and %lu interactions",
     _maxX, _maxY, _loc_to_qubit.size(), _loc_to_interaction.size());

}

void HW_Target_Dwave::addInteraction(COORD x, COORD y, HW_Interaction* interac) {
  QASSERT(x != y);
  if (x < y)
    _loc_to_interaction.insert(std::make_pair(std::make_pair(x,y),
        interac));
  else 
    _loc_to_interaction.insert(std::make_pair(std::make_pair(y,x),
        interac));
}


void HW_Target_Dwave::buildInterCellInteractions(COORD x1, COORD y1, COORD x2, COORD y2, bool vertical) {
  if (vertical) {
    for (COORD local = 0; local < 4; ++local) {
      COORD qbit_index1 = HW_Loc::toGlobalIndex(x1,y1,local);
      COORD qbit_index2 = HW_Loc::toGlobalIndex(x2,y2,local);
      HW_Qubit* qubit1 = _loc_to_qubit.at(qbit_index1);
      HW_Qubit* qubit2 = _loc_to_qubit.at(qbit_index2);
      HW_Interaction* interac = new HW_Interaction(qubit1,qubit2,NULL);
      if (qbit_index1 < qbit_index2)
        _inter_cell_interaction.insert(std::make_pair(std::make_pair(qbit_index1, qbit_index2), interac));
      else
        _inter_cell_interaction.insert(std::make_pair(std::make_pair(qbit_index2, qbit_index1), interac));
      addInteraction(qbit_index1,qbit_index2,interac);
    }
  } else {
    for (COORD local = 4; local < 8; ++local) {
      COORD qbit_index1 = HW_Loc::toGlobalIndex(x1,y1,local);
      COORD qbit_index2 = HW_Loc::toGlobalIndex(x2,y2,local);
      HW_Qubit* qubit1 = _loc_to_qubit.at(qbit_index1);
      HW_Qubit* qubit2 = _loc_to_qubit.at(qbit_index2);
      HW_Interaction* interac = new HW_Interaction(qubit1,qubit2,NULL);
      if (qbit_index1 < qbit_index2)
        _inter_cell_interaction.insert(std::make_pair(std::make_pair(qbit_index1, qbit_index2), interac));
      else
        _inter_cell_interaction.insert(std::make_pair(std::make_pair(qbit_index2, qbit_index1), interac));
      addInteraction(qbit_index1,qbit_index2,interac);
    }
  }
}

HW_Interaction* HW_Target_Dwave::getInteraction(const HW_Loc& loc1, const HW_Loc& loc2) const {
  COORD gindex1 = loc1.getGlobalIndex();
  COORD gindex2 = loc2.getGlobalIndex();

  COORD index1 = (gindex1 < gindex2) ? gindex1 : gindex2;
  COORD index2 = (gindex1 < gindex2) ? gindex2 : gindex1;

  if (_loc_to_interaction.count(std::make_pair(index1,
                                               index2)))
    return _loc_to_interaction.at(std::make_pair(index1,
                                                 index2));
  else
    return NULL;
}

/*
HW_Interaction* HW_Target_Dwave::getInteraction(const COORD qubit1, const COORD qubit2) const {
  COORD gindex1 = qubit1;
  COORD gindex2 = qubit2;

  COORD index1 = (gindex1 < gindex2) ? gindex1 : gindex2;
  COORD index2 = (gindex1 < gindex2) ? gindex2 : gindex1;

  if (_loc_to_interaction.count(std::make_pair(index1,
                                               index2)))
    return _loc_to_interaction.at(std::make_pair(index1,
                                                 index2));
  else
    return NULL;
}
*/

HW_Qubit* HW_Target_Dwave::getQubit(const HW_Loc& loc) const {
  if (_loc_to_qubit.count(loc.getGlobalIndex()))
    return _loc_to_qubit.at(loc.getGlobalIndex());
  else
    return NULL;
}

void HW_Target_Dwave::addQubit(COORD x, HW_Qubit* qubit) {
  _loc_to_qubit.insert(std::make_pair(x, qubit));

}

HW_Target_Dwave::~HW_Target_Dwave() {
 for (Q_ITER qiter = _loc_to_qubit.begin(); 
        qiter != _loc_to_qubit.end(); ++qiter) {
    delete qiter->second;
  }

  _loc_to_qubit.clear();

  for (I_ITER iter = _loc_to_interaction.begin();
        iter != _loc_to_interaction.end(); ++iter)
    delete iter->second;

  for (C_ITER iter = cell_begin(); iter != cell_end(); ++iter)
    delete iter->second;

  _loc_to_interaction.clear();
  _inter_cell_interaction.clear();

}



