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


#include "qpar/qpar_netlist.hh"
#include "qpar/qpar_target.hh"
#include "qpar/qpar_utils.hh"
#include "qpar/qpar_routing_graph.hh"
#include "qpar/qpar_route.hh"

#include "hw_target/hw_object.hh"

#include "syn/netlist.h"
#include "utils/qlog.hh"

#include <vector>
#include <algorithm>
#include <unordered_set>

bool ParWireCmp::operator()(const ParWire* wire1, const ParWire* wire2) const {
  return wire1->getUniqId() < wire2->getUniqId();
}

bool ParWireTargetCmp::operator()(const ParWireTarget* tgt1, const ParWireTarget* tgt2) const {
  return tgt1->getUniqId() < tgt2->getUniqId();
}

bool ParElementCmp::operator()(const ParElement* ele1, const ParElement* ele2) const {
  return ele1->getUniqId() < ele2->getUniqId();
}

void ParElement::setGrid(ParGrid* grid) {
  _grid.setStatus(grid);
}

void ParElement::save() { _grid.saveStatus(); }

void ParElement::restore() { 
  _grid.restoreStatus(); 
}

COORD ParElement::getX() const {
  QASSERT(_grid.getStatus());
  return _grid.getStatus()->getLoc().getLocX();
}


COORD ParElement::getY() const {
  QASSERT(_grid.getStatus());
  return _grid.getStatus()->getLoc().getLocY();
}

ParGrid* ParElement::getCurrentGrid() const {
  return _grid.getStatus();
}

//std::pair<COORD,COORD> ParElement::getCurrentPlacement() const {
//  return std::make_pair(getX(),getY());
//}

ParNetlist* ParNetlist::_top_netlist = NULL;

ParNetlist::ParNetlist(SYN::Model* model) :
  _syn_netlist(model) {
    buildParNetlist();
}


ParNetlist::~ParNetlist() {
  ParElementSet::iterator e_iter = _elements.begin();
  for (; e_iter != _elements.end(); ++e_iter)
    delete *e_iter;
  _elements.clear();

  ParWireSet::iterator w_iter = _wires.begin();
  for (; w_iter != _wires.end(); ++w_iter)
    delete *w_iter;
  _wires.clear();
}


void ParNetlist::buildParNetlist() {
  QASSERT(_syn_netlist);

  std::vector<SYN::Gate*> gates;
  std::unordered_map<SYN::Gate*, ParElement*> gate_to_par_element;
  std::unordered_map<SYN::Net*, ParWire*> net_to_par_wire;
  gates = _syn_netlist->getModelGates();

  for (size_t i = 0; i < gates.size(); ++i) {
    SYN::Gate* syn_gate = gates[i];
    QASSERT(syn_gate);
    ParElement* element = new ParElement(syn_gate);
    gate_to_par_element.insert(std::make_pair(syn_gate, element));
    _elements.insert(element);
    //qlog.speak("debug", "insert %lu", _elements.size());
  }

  typedef std::vector<SYN::Pin*>::iterator PIN_ITER;
  for (size_t i = 0; i < gates.size(); ++i) {
    SYN::Gate* syn_gate = gates[i];
    QASSERT(gate_to_par_element.count(syn_gate));
    ParElement* element = gate_to_par_element.at(syn_gate);
    PIN_ITER pin_iter = syn_gate->begin();
    for (; pin_iter != syn_gate->end(); ++pin_iter) {
      SYN::Pin* pin = *pin_iter;
      QASSERT(!pin->isBidir());
      ParWire* parwire = NULL;

      SYN::Net* net = pin->net();
      if (net_to_par_wire.count(net)) {
        parwire = net_to_par_wire.at(net);
      } else {
        if (net == NULL || net->isDummy()) {
          qlog.speak("Design", "Gate %s Pin %s is dangling",
              pin->getGate()->getName().c_str(),
              net->name().c_str());
          continue;
        }
        parwire = new ParWire(net);

        net_to_par_wire.insert(std::make_pair(net, parwire));
      }
      element->addWire(parwire);
    }
  }


  std::unordered_map<SYN::Net*, ParWire*>::iterator w_iter = net_to_par_wire.begin();
  for (; w_iter != net_to_par_wire.end(); ++w_iter) {
    ParWire* wire = w_iter->second;
    std::vector<ParWireTarget*> targets = wire->buildWireTarget(gate_to_par_element, _elements);
    _all_targets.insert(_all_targets.end(), targets.begin(), targets.end());
    if (wire->getElementNumber() <= 1) {
      _model_wires.insert(wire);
      if (wire->getElementNumber() == 0) continue;

      ParElement* ele_uniq = wire->getUniqElement();
      ele_uniq->disconnectWire(wire);
      SYN::Pin* u_pin = wire->getUniqElementPin();
      QASSERT(u_pin);
      ele_uniq->assignPin(u_pin);
    }
    else
      _wires.insert(wire);
  }
  //qlog.speak("debug", "size %lu", _elements.size());



  qlog.speak("Design", "%u wires, %u model wires, %u elements has been constructed",
      (unsigned)_wires.size(),
      (unsigned)_model_wires.size(),
      (unsigned)_elements.size()
      );

  //WIRE_ITER wi_iter = wire_begin();
  //for (; wi_iter != wire_end(); ++wi_iter)
  //  (*wi_iter)->printSelf();


}

unsigned int ParElement::_element_index_counter = 0;
ParElement::ParElement(SYN::Gate* gate) : 
_gate(gate),
_pin(NULL),
_sink(NULL),
_movable(true)
{
  //qlog.speak("debug", "new gate %u", _element_index_counter);
  _element_index = _element_index_counter;
  ++_element_index_counter;
}

ParElement::ParElement(SYN::Pin* pin) :
_gate(NULL),
_pin(pin),
_sink(NULL),
_movable(true)
{
  //qlog.speak("debug", "new pin %u", _element_index_counter);
  _element_index = _element_index_counter;
  ++_element_index_counter;
}

void ParElement::addWire(ParWire* wire) {
  _wires.push_back(wire);
  wire->addElement(this);
}

void ParElement::disconnectWire(ParWire* wire) {
  std::vector<ParWire*> new_wires;
  for (WIRE_ITER_V w_iter = begin(); w_iter != end(); ++w_iter) {
    if ((*w_iter) != wire)
      new_wires.push_back(*w_iter);
  }
  _wires = new_wires;
}

void ParElement::updatePlacement() {
  QASSERT(_grid.getStatus());
  _x = (_grid.getStatus())->getLoc().getLocX();
  _y = (_grid.getStatus())->getLoc().getLocY();

}

std::string ParElement::getName() const {
  if (_gate)
    return _gate->getName();
  else if (_pin)
    return _pin->getName() + ".(model pin)";
  QASSERT(0);
}


void ParElement::assignPin(SYN::Pin* pin) {
  std::set<COORD> locs;
  for (COORD i = 0; i < 4; ++i)
    locs.insert(i);

  std::unordered_map<SYN::Pin*, COORD>::iterator pin_iter = _pin_to_loc.begin();
  for (; pin_iter != _pin_to_loc.end(); ++pin_iter) {
    QASSERT(locs.count(pin_iter->second));
    locs.erase(pin_iter->second);
  }

  QASSERT(locs.size());
  _pin_to_loc.insert(std::make_pair(pin, *(locs.begin())));
  _used_loc.insert(*(locs.begin()));
}

unsigned int ParWire::_wire_index_counter = 0;
ParWire::ParWire(SYN::Net* wire) : 
_net(wire),
_source(NULL) {
  _wire_index = _wire_index_counter;
  ++_wire_index_counter;
}


ParWire::~ParWire() {
  for (unsigned i = 0; i < _targets.size(); ++i) {
    delete _targets[i];
  }
}


std::string ParWire::getName() const {
  return _net->name();
}

std::vector<ParWireTarget*>& ParWire::buildWireTarget(
    const std::unordered_map<SYN::Gate*, ParElement*>& gate_to_par_element,
    ParElementSet& elements) {
 _targets.clear();
  SYN::Pin* source = _net->uniqSource();
  QASSERT(source);
  std::vector<SYN::Pin*> sink;
  QASSERT(_net->numSink());
  for (unsigned i = 0; i <  (unsigned)_net->numSink(); ++i)
    sink.push_back(_net->getSink(i));
  
  bool src_is_model_pin = source->isModelPin();


  if (src_is_model_pin) {
    SYN::Pin* gate1 = NULL;
    for (unsigned i = 0; i < sink.size(); ++i) {
      if (sink[i]->isGatePin()) {
        gate1 = sink[i];
        break;
      }
    }

    if (gate1) {

      for (unsigned i = 0; i < sink.size(); ++i) {
        SYN::Pin* sk = sink[i];
        if (sk == gate1) {
          ParElement* src_ele = gate_to_par_element.at(gate1->getGate());
          ParWireTarget* target = new ParWireTarget(src_ele, src_ele, source, sk, this);
          target->setDontRoute(true);
          _targets.push_back(target);
          continue;
        }

        if (sk->isModelPin()) {
          ParElement* src_ele = gate_to_par_element.at(gate1->getGate());
          ParWireTarget* target = new ParWireTarget(src_ele, NULL, source, sk, this);
          target->setDontRoute(true);
          _targets.push_back(target);
        } else {
          ParElement* src_ele = gate_to_par_element.at(gate1->getGate());
          ParElement* tgt_ele = gate_to_par_element.at(sk->getGate());
          ParWireTarget* target = new ParWireTarget(src_ele, tgt_ele, gate1, sk, this);
          _targets.push_back(target);
        }
      }
    } else {
      for (unsigned i = 0; i < sink.size(); ++i) {
        SYN::Pin* sk = sink[i];
        ParWireTarget* target = new ParWireTarget(NULL, NULL, source, sk, this);
        target->setDontRoute(true);
        _targets.push_back(target);
      }
    }
  } else {
    for (unsigned i = 0; i < sink.size(); ++i) {
      SYN::Pin* sk = sink[i];
      ParElement* src_ele = gate_to_par_element.at(source->getGate());
      if (sk->isModelPin()) {
        ParElement* element = new ParElement(sk);
        element->addWire(this);
        ParWireTarget* target = new ParWireTarget(src_ele, element, source, sk, this);
        elements.insert(element);
        _targets.push_back(target);
      } else {
        ParElement* tgt_ele = gate_to_par_element.at(sk->getGate());
        ParWireTarget* target = new ParWireTarget(src_ele, tgt_ele, source, sk, this);
        _targets.push_back(target);
      }
    }
  }

  return _targets;
}

void ParWire::recomputeBoundingBox() {

  int xl = std::numeric_limits<int>::max();
  int xr = -1;
  int yt = std::numeric_limits<int>::max();
  int yb = -1;

  int xle = 0;
  int xre = 0;
  int yte = 0;
  int ybe = 0;

  ELE_ITER ele_iter = _elements.begin();
  for (; ele_iter != _elements.end(); ++ele_iter) {
    ParElement* ele = *ele_iter;
    COORD coordX = ele->getX();
    COORD coordY = ele->getY();

    if (coordX < xl) {
      xl = (int)coordX;
      xle = 1;
    } else if (coordX == xl) {
      ++xle;
    }

    if (coordX > xr) {
      xr = (int)coordX;
      xre = 1;
    } else if (coordX == xr) {
      ++xre;
    }

    if (coordY > yb) {
      yb = (int)coordY;
      ybe = 1;
    } else if (coordY == yb) {
      ++ybe;
    }

    if (coordY < yt) {
      yt = (int)coordY;
      yte = 1;
    } else if (coordY == yt) {
      ++yte;
    }

  }

  Box box1(xl, xr, yt, yb);
  Box box2(xle,xre,yte,ybe);
  _bounding_box.getStatus().setBoundingBox(box1);
  _bounding_box.getStatus().setEdgeBox(box2);

}

void ParWire::initializeBoundingBox() {
  recomputeBoundingBox();
  _bounding_box.saveStatus();
}

void ParWire::saveCost() {
  _cost.saveStatus();
}

double ParWire::getCurrentCost() const {
  return _cost.getStatus();
}

double ParWire::getSavedCost() const {
  return _cost.getSavedStatus();
}

void ParWire::setCost(double val) {
  _cost.setStatus(val);
}

bool ParWire::sanityCheck() {
  int xl = std::numeric_limits<int>::max();
  int xr = -1;
  int yt = std::numeric_limits<int>::max();
  int yb = -1;

  int xle = 0;
  int xre = 0;
  int yte = 0;
  int ybe = 0;

  ELE_ITER ele_iter = _elements.begin();
  for (; ele_iter != _elements.end(); ++ele_iter) {
    ParElement* ele = *ele_iter;
    COORD coordX = ele->getX();
    COORD coordY = ele->getY();

    if (coordX < xl) {
      xl = (int)coordX;
      xle = 1;
    } else if (coordX == xl) {
      ++xle;
    }

    if (coordX > xr) {
      xr = (int)coordX;
      xre = 1;
    } else if (coordX == xr) {
      ++xre;
    }

    if (coordY > yb) {
      yb = (int)coordY;
      ybe = 1;
    } else if (coordY == yb) {
      ++ybe;
    }

    if (coordY < yt) {
      yt = (int)coordY;
      yte = 1;
    } else if (coordY == yt) {
      ++yte;
    }

  }

  Box box = getCurrentBoundingBox().getBoundBox();
  if (box.xr() != xr) {
    qlog.speak("Place", "Sanity Checking Net: BoundingBox xr %u is not equal to incr %u", box.xr(), xr);
    return false;
  }
  if (box.xl() != xl) {
    qlog.speak("Place", "Sanity Checking Net: BoundingBox xl %u is not equal to incr %u", box.xl(), xl);
    return false;
  }
  if (box.yb() != yb) {
    qlog.speak("Place", "Sanity Checking Net: BoundingBox yb %u is not equal to incr %u", box.yb(), yb);
    return false;
  }
  if (box.yt() != yt) {
    qlog.speak("Place", "Sanity Checking Net: BoundingBox yt %u is not equal to incr %u", box.yt(), yt);
    return false;
  }

  return true;


}

void ParWire::restore() {
  _bounding_box.restoreStatus();
  _cost.restoreStatus();
}

BoundingBox ParWire::getCurrentBoundingBox() const {
  return _bounding_box.getStatus();
}

void ParWire::saveBoundingBox() {
  _bounding_box.saveStatus();
}

void ParWire::updateBoundingBox(COORD from_x, COORD from_y, COORD to_x, COORD to_y) {
  Box& bbox = _bounding_box.getStatus().getBoundBox();
  Box& ebox = _bounding_box.getStatus().getEdgeBox();
  bool recal = false ;

  if (to_x < bbox.xl()) {
    bbox.set_xl((int)to_x) ;
    ebox.set_xl(1) ;
  } else if (to_x == bbox.xl()) {
    ebox.incr_xl() ;
    if (from_x == bbox.xl())
      ebox.decr_xl() ;
  } else if (from_x == bbox.xl()) {
    if (ebox.xl() > 1)
      ebox.decr_xl() ;
    else
      recal = recal || true ;
  }

  if (to_x > bbox.xr()) {
    bbox.set_xr((int)to_x) ;
    ebox.set_xr(1) ;
  } else if (to_x == bbox.xr()) {
    ebox.incr_xr() ;
    if (from_x == bbox.xr())
      ebox.decr_xr() ;
  } else if (from_x == bbox.xr()) {
    if (ebox.xr() > 1)
      ebox.decr_xr() ;
    else
      recal = recal || true ;
  }

  if (to_y < bbox.yt()) {
    bbox.set_yt((int)to_y) ;
    ebox.set_yt(1) ;
  } else if (to_y == bbox.yt()) {
    ebox.incr_yt() ;
    if (from_y == bbox.yt())
      ebox.decr_yt() ;
  } else if (from_y == bbox.yt()) {
    if (ebox.yt() > 1)
      ebox.decr_yt() ;
    else
      recal = recal || true ;
  }


  if (to_y > bbox.yb()) {
    bbox.set_yb((int)to_y) ;
    ebox.set_yb(1) ;
  } else if (to_y == bbox.yb()) {
    ebox.incr_yb() ;
    if (from_y == bbox.yb())
      ebox.decr_yb() ;
  } else if (from_y == bbox.yb()) {
    if (ebox.yb() > 1)
      ebox.decr_yb() ;
    else
      recal = recal || true ;
  }

  if (recal)
    recomputeBoundingBox();

}

ParElement* ParWire::getUniqElement() {
  if (_elements.size() != 1) return NULL;
  return (*_elements.begin());
}

SYN::Pin* ParWire::getUniqElementPin() const {
  SYN::Pin* pin = NULL;
  SYN::Net::PIN_ITER_CONST pin_iter = _net->begin();
  for (; pin_iter != _net->end(); ++pin_iter) {
    SYN::Pin* pin_t = *pin_iter;
    if (pin_t->isGatePin() && pin == NULL)
      pin = pin_t;
    else if (pin_t->isGatePin() && pin)
      QASSERT(0);
    else continue;
  }

  return pin;
}


void ParWire::updateWireRoute(RoutePath* route) {

  for (size_t i = 0; i < route->size(); ++i) {
    RoutingNode* node = route->at(i);
    if (!node->getCurrentlyUsed()) {
      node->addLoad(1);
      node->setCurrentlyUsed(true);
    }

    if (_routing_nodes.count(node))
      ++_routing_node_usage[node];
    else {
      _routing_nodes.insert(node);
      _routing_node_usage[node] = 1;
    }
  }

}

void ParWire::removeUsedRoutingNode(RoutingNode* node) {
  QASSERT(_routing_nodes.count(node));
  if (_routing_node_usage[node] == 1) {
    _routing_node_usage.erase(node);
    _routing_nodes.erase(node);
  } else
    --_routing_node_usage[node];
}

void ParWire::markUsedRoutingResource() {
  std::unordered_set<RoutingNode*>::iterator node_iter = _routing_nodes.begin();
  for (; node_iter != _routing_nodes.end(); ++node_iter) {
    RoutingNode* node = *node_iter;
    node->setCurrentlyUsed(true);
  }

}

void ParWire::unmarkUsedRoutingResource() {
  std::unordered_set<RoutingNode*>::iterator node_iter = _routing_nodes.begin();
  for (; node_iter != _routing_nodes.end(); ++node_iter) {
    RoutingNode* node = *node_iter;
    node->setCurrentlyUsed(false);
  }
}

void ParWire::printSelf() const {
  std::stringstream ss;
  for (size_t i = 0; i < _targets.size(); ++i)
    ss << _targets[i]->getName() << " | ";

  qlog.speak("WIRE", "Name:%s, Targets: %s",
      getName().c_str(),
      ss.str().c_str());
}

void ParWire::checkRoutingNodeUsage() {


  std::unordered_map<RoutingNode*, unsigned> routing_node_usage;
  std::unordered_set<RoutingNode*> routing_nodes;
  for (size_t i = 0; i < _targets.size(); ++i) {
    ParWireTarget* target = _targets[i];
    RoutePath* route = target->getRoutePath();
    if (!route) continue;
    for (size_t i = 0; i < route->size(); ++i) {
      RoutingNode* node = route->at(i);
      if (routing_node_usage.count(node))
        ++routing_node_usage[node];
      else
        routing_node_usage[node] = 1;
      routing_nodes.insert(node);
    }
  }

  QASSERT(routing_nodes == _routing_nodes);
  QASSERT(routing_node_usage == _routing_node_usage);

}



unsigned ParWireTarget::_wire_target_counter = 0;

ParWireTarget::ParWireTarget(ParElement* source, ParElement* dest,
    SYN::Pin* src_pin, SYN::Pin* tgt_pin, ParWire* wire) :
  _source(source),
  _target(dest),
  _src_pin(src_pin),
  _tgt_pin(tgt_pin),
  _wire(wire),
  _dontRoute(false),
  _route(NULL) {
    _target_index = _wire_target_counter;
    ++_wire_target_counter;
  }

void ParWireTarget::ripupTarget() {
  if (!_route) return;

  for (size_t i = 0; i < _route->size(); ++i) {
    RoutingNode* node = _route->at(i);
    if (_wire->getRoutingNodeUsage().at(node) == 1) {
      node->subLoad(1);
      node->setCurrentlyUsed(false);
    }
    _wire->removeUsedRoutingNode(node);
  }
}

double ParWireTarget::getSlack() const {
  return _wire->getSlack();
}

std::string ParWireTarget::getName() const {
  if (_tgt_pin->isModelPin())
    return _tgt_pin->getName();
  else
    return _tgt_pin->getGate()->getName() + "." + _tgt_pin->getName();
}

void ParWireTarget::printRoute(std::ostream& out) const {
  if (getDontRoute()) return;

  out << "Wire: " << _wire->getName() << " ";

  for (size_t i = 0; i < _route->size(); ++i) {
    RoutingNode* node = _route->at(i);

    if (node->isPin()) {
      out << "(";
      if (node->getPin()->isModelPin())
        out << "model." + node->getPin()->getName();
      else
        out << node->getPin()->getGate()->getName() + "." + node->getPin()->getName();
      out << ")";
      continue;
    }

    if (node->isInteraction()) {
      out << "->";
      continue;
    }

    if (node->isQubit() && node->isLogic()) {
      out << "(";
      out << node->getQubit()->getLoc().getLocX() << ",";
      out << node->getQubit()->getLoc().getLocY() << ",";
      out << node->getQubit()->getLoc().getLocalIndex() << ", is_logic)";
      continue;
    }

    if (node->isQubit() && !node->isLogic()) {
      out << "(";
      out << node->getQubit()->getLoc().getLocX() << ",";
      out << node->getQubit()->getLoc().getLocY() << ",";
      out << node->getQubit()->getLoc().getLocalIndex() << ")";
      continue;
    }

  }

  out << std::endl;
}








