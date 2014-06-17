/*
 * GHOST (General meta-Heuristic Optimization Solving Tool) is a C++ library 
 * designed for StarCraft: Brood war. 
 * GHOST is a meta-heuristic solver aiming to solve any kind of combinatorial 
 * and optimization RTS-related problems represented by a CSP. 
 * It is an extension of the project Wall-in.
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


#include "../../include/constraints/wallinConstraint.hpp"

namespace ghost
{
  WallinConstraint::WallinConstraint( const vector< Building > &variables, const WallinGrid &domain )
    : Constraint<Building, WallinGrid>(variables, domain) { }
    
  bool WallinConstraint::isWall() const
  {
    auto startingBuildings = domain.buildingsAt( domain.getStartingTile() );
    if( startingBuildings.size() != 1)
      return false;

    auto targetBuildings = domain.buildingsAt( domain.getTargetTile() );
    if( targetBuildings.size() != 1)
      return false;

    // if same building on both the starting and target tile
    if( *startingBuildings.begin() == *targetBuildings.begin() )
      return true;

    int nberTarget = *( targetBuildings.begin() );

    int nberCurrent = *( startingBuildings.begin() );
    Building current = variables.at( nberCurrent );
    set< Building > toVisit = domain.getBuildingsAround( current, variables );
    set< Building > visited;
    set< Building > neighbors;
    
    visited.insert( current );

    if( toVisit.find( variables.at( nberTarget ) ) != toVisit.end() )
      return true;
    
    while( !toVisit.empty() )
    {
      auto first = *( toVisit.begin() );
      current = first;
      toVisit.erase( first );
      neighbors = domain.getBuildingsAround( current, variables );
      
      for( auto &n : neighbors )
      {
	if( n.getId() == nberTarget )
	  return true;
	if( visited.find( n ) == visited.end() )
	  toVisit.insert( n );
      }

      visited.insert( current );
    }

    return false;
  }


  /***********/
  /* Overlap */
  /***********/  
  Overlap::Overlap(const vector< Building > &variables, const WallinGrid &domain) 
    : WallinConstraint(variables, domain) { }

  double Overlap::cost( vector<double> &varCost ) const
  {
    // version 1: 1 failure = 1 cost
    // return double( domain.failures().size() );

    // version 2: 1 conflict = 1 cost (may have several conflicts into one failure)
    double conflicts = 0.;

    for( auto &failures : domain.failures() )
    {
      int nbConflict = failures.second.size() - 1;
      if( nbConflict > 0 && failures.second.find( "###" ) == string::npos )
      {
	conflicts += nbConflict;
	set<int> setBuildings = domain.buildingsAt( failures.first );
	for( auto &id : setBuildings )
	  varCost[ id ] += nbConflict;
      }
    }

    return conflicts;    
  }

  vector<double> Overlap::simulateCost( Building &oldBuilding, const vector<int> &newPosition, vector< vector<double> > &vecVarSimCosts )
  {
    vector<double> simCosts( domain.getSize(), -1. );
    int backup = oldBuilding.getValue();
    int previousPos = -1;
    int diff;

    for( auto &pos : newPosition )
    {
      if( pos >= 1 && pos == previousPos + 1 )
      {
	vecVarSimCosts[pos + 1] = vecVarSimCosts[pos];
	
	diff = domain.shift( oldBuilding ).first;
	if( diff != 0 )
	{
	  set<int> setBuildings = domain.buildingsAt( pos + 1 );
	  for( auto &id : setBuildings )
	    vecVarSimCosts[pos + 1][ id ] += diff;
	}

	simCosts[pos + 1] = simCosts[pos] + diff;
      }
      else
      { 
	domain.clear( oldBuilding );
	oldBuilding.setValue( pos );
	domain.add( oldBuilding );
	
	simCosts[pos + 1] = cost( vecVarSimCosts[pos + 1] );
      }

      previousPos = pos;
    }

    domain.clear( oldBuilding );
    oldBuilding.setValue( backup );
    domain.add( oldBuilding );
    
    return simCosts;
  }


  /*************/
  /* Buildable */
  /*************/  
  Buildable::Buildable(const vector< Building > &variables, const WallinGrid &domain) 
    : WallinConstraint(variables, domain) { }

  double Buildable::cost( vector<double> &varCost ) const
  {
    // count number of buildings misplaced on unbuildable tiles (denoted by ###)
    double conflicts = 0.;
    int nbConflict;

    for( auto &failures : domain.failures() )
    {
      if( failures.second.find( "###" ) != string::npos )
      {
	nbConflict = failures.second.size() - 3;
	conflicts += nbConflict;
	set<int> setBuildings = domain.buildingsAt( failures.first );
	for( auto &id : setBuildings )
	  varCost[ id ] += nbConflict;
      }
    }

    return conflicts;    
  }

  vector<double> Buildable::simulateCost( Building &oldBuilding, const vector<int> &newPosition, vector< vector<double> > &vecVarSimCosts )
  {
    vector<double> simCosts( domain.getSize(), -1. );
    int backup = oldBuilding.getValue();
    int previousPos = -1;
    int diff;

    for( auto &pos : newPosition )
    {
      if( pos >= 1 && pos == previousPos + 1 )
      {
	vecVarSimCosts[pos + 1] = vecVarSimCosts[pos];
	
	diff = domain.shift( oldBuilding ).second;
	if( diff != 0 )
	{
	  set<int> setBuildings = domain.buildingsAt( pos + 1 );
	  for( auto &id : setBuildings )
	    vecVarSimCosts[pos + 1][ id ] += diff;
	}

	simCosts[pos + 1] = simCosts[pos] + diff;
      }
      else
      { 
	domain.clear( oldBuilding );
	oldBuilding.setValue( pos );
	domain.add( oldBuilding );
	
	simCosts[pos + 1] = cost( vecVarSimCosts[pos + 1] );
      }

      previousPos = pos;
    }

    domain.clear( oldBuilding );
    oldBuilding.setValue( backup );
    domain.add( oldBuilding );
    
    return simCosts;
  }


  /**********/
  /* NoGaps */
  /**********/  
  NoGaps::NoGaps(const vector< Building > &variables, const WallinGrid &domain) 
    : WallinConstraint(variables, domain) { }

  double NoGaps::cost( vector<double> &varCost ) const
  {
    // cost = |buildings with one neighbor| - 1 + |buildings with no neighbors|
    double conflicts = 0.;

    if( !isWall() )
    {
      int nberNeighbors;
      vector<int> oneNeighborBuildings;

      for( auto &building : variables )
      {
	if( building.isSelected() )
	{
	  // if we don't have a wall, penalise all buildings on the domain.
	  ++conflicts;
	  ++varCost[ building.getId() ];
	  
	  nberNeighbors = domain.countAround( building, variables );

	  if( nberNeighbors == 0 || nberNeighbors > 2 ) // to change with Protoss and pylons
	  {
	    ++conflicts;
	    ++varCost[ building.getId() ];
	  }
	  else
	  {
	    if( nberNeighbors == 1 )
	      oneNeighborBuildings.push_back( building.getId() );
	  }
	}
      }

      if( oneNeighborBuildings.size() > 2 ) // for latter: pylons can be alone, or have 1 neighbor only
      {
	for( auto &b : oneNeighborBuildings )
	  if( ! domain.isStartingOrTargetTile( b ) )
	  {
	    ++conflicts;
	    ++varCost[ b ];
	  }
      }
    }
    
    return conflicts;    
  }


  /***********************/
  /* StartingTargetTiles */
  /***********************/  
  StartingTargetTiles::StartingTargetTiles(const vector< Building > &variables, const WallinGrid &domain) 
    : WallinConstraint(variables, domain)
  {
    for( auto &b : variables )
      mapBuildings[b.getId()] = b;
  }

  double StartingTargetTiles::cost( vector<double> &varCost ) const
  {
    // no building on one of these two tiles: cost of the tile = 6
    // a building with no or with 2 or more neighbors: cost of the tile = 3
    // two or more buildings on one of these tile: increasing penalties.
    double conflicts = 0.;

    set<int> startingBuildings = domain.buildingsAt( domain.getStartingTile() );
    set<int> targetBuildings = domain.buildingsAt( domain.getTargetTile() );

    Building b;
    int neighbors;

    // if same building on both the starting and target tile
    if( startingBuildings.size() == 1 && targetBuildings.size() == 1 && *startingBuildings.begin() == *targetBuildings.begin() )
      return 0.;

    if( startingBuildings.empty() )
    {
      // penalize buildings not placed on the domain
      for( auto &v : variables )
	if( !v.isSelected() )
	{
	  varCost[ v.getId() ] += 2;
	  conflicts += 2;
	}
    }
    else
    {
      //int penalty = 0;
      for( int bId : startingBuildings )
      {
	b = mapBuildings.at(bId);
	neighbors = domain.countAround( b, variables );

	if( neighbors != 1 )
	{
	  conflicts += 2;
	  varCost[ bId ] += 2;
	}

	//conflicts += penalty++;
      }
    }

    if( targetBuildings.empty() )
    {      
      // penalize buildings not placed on the domain
      for( auto &v : variables )
	if( !v.isSelected() )
	{
	  varCost[ v.getId() ] += 2;
	  conflicts += 2;
	}
    }
    else
    {
      //int penalty = 0;
      for( int bId : targetBuildings )
      {
	b = mapBuildings.at(bId);
	neighbors = domain.countAround( b, variables );

	if( neighbors != 1 )
	{
	  conflicts += 2;
	  varCost[ bId ] += 2;
	}

	//conflicts += penalty++;	
      }
      
    }

    return conflicts;    
  }
}
