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

#ifndef QPAR_ROUTE_HH
#define QPAR_ROUTE_HH

/*!
 * \file qpar_route.hh
 * \author Juexiao Su
 * \date 18 Dec 2017
 * \brief routing
 */

#include "qpar_graph.hh"

class RoutingNode;
class RoutingEdge;
class RoutingGraph;


class FastRoutingGraph : public qpr_graph<RoutingNode*, RoutingEdge*> {
typedef qpr_graph<RoutingNode*, RoutingEdge*> SUPER;

public:
  FastRoutingGraph(RoutingGraph* graph);
  friend class RoutingTester;

};




#endif
