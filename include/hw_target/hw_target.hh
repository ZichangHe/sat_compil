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

#ifndef HW_TARGET_HH
#define HW_TARGET_HH

#include "hw_loc.hh"
#include "hw_param.hh"

#include <boost/functional/hash.hpp>

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>

class HW_Qubit;
class HW_Cell;
class HW_Interaction;

class HW_Target_abstract {

public:

  typedef std::unordered_map<COORD, HW_Qubit*> LocToQubit;
  typedef std::unordered_map<std::pair<COORD,COORD>, HW_Interaction*,
          boost::hash<std::pair<COORD,COORD> > > LocToCellInteraction;
  typedef std::unordered_map<std::pair<COORD, COORD>, HW_Cell*,
          boost::hash<std::pair<COORD,COORD> > > LocToCell;


  typedef std::unordered_map<COORD, HW_Qubit*>::iterator Q_ITER;
  typedef std::unordered_map<std::pair<COORD,COORD>, HW_Interaction*, 
          boost::hash<std::pair<COORD,COORD> > >::iterator I_ITER;
  typedef std::unordered_map<std::pair<COORD, COORD>, HW_Cell*,
          boost::hash<std::pair<COORD,COORD> > >::iterator C_ITER;
  

  /*! \brief default constructor
   */
  HW_Target_abstract() :
  _maxX(0), _maxY(0) {}

  /*! \brief default destructor
   */
  virtual ~HW_Target_abstract();

  /*! \brief get qubit by its location
   *  \return a pointer of HW_Qubit
   */
  virtual HW_Qubit* getQubit(const HW_Loc& loc) const = 0;

  /*! \brief get interaction by its location
   *  \return a pointer of HW_Interaction
   */
  virtual HW_Interaction* getInteraction(const HW_Loc& loc1, const HW_Loc& loc2) const = 0;

  /*! \brief get x limit
   */
  COORD getXLimit() const { return _maxX; } 

  /*! \brief get y limit
   */
  COORD getYLimit() const { return _maxY; }




protected:

  LocToQubit            _loc_to_qubit;         //!< a map between location and qubit
  LocToCellInteraction  _loc_to_interaction;   //!< a map between location and interaction

  COORD _maxX; //!< max x coordinate
  COORD _maxY; //!< max y coordinate
  
};


class HW_Target_Dwave : public HW_Target_abstract {

public:
  /*! \brief default constructor
   *  \param hw_param hardware related paramter
   */
  HW_Target_Dwave(HW_Param* hw_param) : _hw_param(hw_param) {}

  /*! \brief default destructor
   */
  virtual ~HW_Target_Dwave();

  /*! \brief get the qubit by its location
   *  \param loc the qubit location
   */
  virtual HW_Qubit* getQubit(const HW_Loc& loc) const;

  /*! \brief add qubit into dwave
   *  \param x global index
   */
  void addQubit(COORD x, HW_Qubit* qubit);

  /*! \brief get the interaction by its location
   *  \param loc the interaction location
   */
  virtual HW_Interaction* getInteraction(const HW_Loc& loc1, const HW_Loc& loc2) const;

  /*! \brief get the interaction by its location
   *  \param qubit1 the global index of qubit1
   *  \param qubit2 the global index of qubit2
   */
  HW_Interaction* getInteraction(const COORD qubit1, const COORD qubit2) const;

  /*! \brief add new interation into hw target
   *  \param x coordinate global x 
   *  \param y coordinate y
   */
  void addInteraction(COORD x, COORD y, HW_Interaction* interac);

  /*! \brief get the cell by its location
   *  \param loc the cell location
   */
  HW_Cell* getCell(const HW_Loc& loc) const;

  /*! \brief set hardware target pointer
   *  \param target the pointer to HW_Target_Dwave;
   */
  static void setHwTarget(HW_Target_Dwave* target) {
    _self = target;
  }

  /*! \brief get hardware target
   *  \return target pointer
   */
  static HW_Target_Dwave* getHwTarget() {
    return _self;
  }

  /*! \brief initialize hardware
   */
  void initializeTarget();

  C_ITER cell_begin()       { return _loc_to_cell.begin(); }
  //C_ITER cell_begin() const { return _loc_to_cell.begin(); }
  C_ITER cell_end()         { return _loc_to_cell.end();}
  //C_ITER cell_end() const   { return _loc_to_cell.end();}

  I_ITER inter_cell_interac_begin() { return _inter_cell_interaction.begin(); }
  I_ITER inter_cell_interac_end()   { return _inter_cell_interaction.end(); }
  //I_ITER inter_cell_interac_begin() const { return _inter_cell_interaction.begin(); }
  //I_ITER inter_cell_interac_end()   const { return _inter_cell_interaction.end(); }



private:


  /*! \brief build adjacent cell interactions
   *  \param x1 coordinate x for cell1
   *  \param y1 coordinate y for cell1
   *  \param x2 coordinate x for cell2
   *  \param y2 coordinate y for cell2
   */
  void buildInterCellInteractions(COORD x1, COORD y1, COORD x2, COORD y2, bool vertical);

  HW_Param* _hw_param;              //!< a paramter class which holds all hw info
  LocToCell  _loc_to_cell;          //!< a map between location and cell
  static HW_Target_Dwave* _self;    //!< a pointer for itself
  LocToCellInteraction  _inter_cell_interaction;   //!< a container stores all the inter cell interaction


};



#endif


