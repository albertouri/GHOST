/*
 * GHOST (General meta-Heuristic Optimization Solving Tool) is a C++ library 
 * designed for StarCraft: Brood war. 
 * GHOST is a meta-heuristic solver aiming to solve any kind of combinatorial 
 * and optimization RTS-related problems represented by a CSP/COP. 
 * It is a generalization of the project Wall-in.
 * Please visit https://github.com/richoux/GHOST for further information.
 * 
 * Copyright (C) 2014 Florian Richoux
 *
 * This file is part of GHOST.
 * GHOST is free software: you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as published 
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * GHOST is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with GHOST. If not, see http://www.gnu.org/licenses/.
 */


#pragma once

#include <vector>

#include "domain.hpp"
#include "../variables/action.hpp"

using namespace std;

namespace ghost
{
  class BuildOrderDomain : public Domain<Action>
  {
  public:
    BuildOrderDomain( int, vector<Action>* );

    void add( const Action& );
    void clear( const Action& );

    void moveTo( int from, int to );
    void addAction( Action &, bool );
    
    friend ostream& operator<<( ostream&, const BuildOrderDomain& );

  private:
    void v_restart( vector<Action> *variables );

    vector<Action> *order;
  };
}
