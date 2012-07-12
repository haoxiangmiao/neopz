/**
 * @file
 * @brief Contains the TPZIdentifyRefPattern class which identifies the refinement pattern given the father element and their sons.\n
 * Also contains TSide structure.
 */
/***************************************************************************
 pzidentifyrefpattern.h  -  description
 -------------------
 begin                : Mon Mar 8 2004
 copyright            : (C) 2004 by cesar
 email                : cesar@labmec.fec.unicamp.br
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PZIDENTIFYREFPATTERN_H
#define PZIDENTIFYREFPATTERN_H

#include "pzvec.h"
#include "pzgeoelside.h"
#include <set>
#include <string>
class TPZRefPattern;

/**
 * \addtogroup pre
 * @{
 */

/**
 * @brief Identifies the refinement pattern given the father element and their sons. \ref pre "Getting Data"
 * @author Edimar Cesar Rylo
 * @since March 8, 2004
 */
class TPZIdentifyRefPattern {
public: 
	TPZIdentifyRefPattern(std::string &path);
	~TPZIdentifyRefPattern();
	/** @brief Returns the refinement pattern that generates the given refinement */
	TPZAutoPointer<TPZRefPattern> GetRefPattern (TPZGeoEl *father, TPZVec<TPZGeoEl *> subelem);
	
protected: // Protected methods
	/** @brief Identify the side of the refinement pattern */
	int IdentifySide(TPZGeoEl *father, TPZVec<TPZGeoEl *> subelem);
	
	/** @brief Returns the number of subelements of a uniform refinement   \
	 pattern for the specified element type.
	 */
	int UniformSubElem(int eltype) ;
	TPZAutoPointer<TPZRefPattern> GetUniform( TPZGeoEl * gel);
	TPZAutoPointer<TPZRefPattern> GetSideRefPattern (TPZGeoEl *gel, int side);
	
protected:
	std::string fPath;
};

/**
 * @brief To store a side and its nodes indexes. \ref pre "Getting data"
 * @ingroup pre
 */
struct TSide {
	std::set<int> fNodes;
	int fSide;
	int operator<(const TSide &other) const{
		return fNodes < other.fNodes;
	}
	TSide &operator=(const TSide &copy) {
		fSide = copy.fSide;
		fNodes = copy.fNodes;
		return *this;
	}
	TSide &operator=(const TPZGeoElSide &gelside) {
		fSide = gelside.Side();
		int i;
		for (i=0;i<gelside.NSideNodes();i++){
			fNodes.insert(gelside.SideNodeIndex(i));
		}
		return *this;
	}
	TSide (const TPZGeoElSide &gelside) {
		fSide = gelside.Side();
		int i;
		for (i=0;i<gelside.NSideNodes();i++){
			fNodes.insert(gelside.SideNodeIndex(i));
		}
	}   
};

/** @} */

#endif
