/*
 * @file vle/extension/decision/Resources.hpp
 *
 * This file is part of VLE, a framework for multi-modeling, simulation
 * and analysis of complex dynamical systems
 * http://www.vle-project.org
 *
 * Copyright (c) 2003-2007 Gauthier Quesnel <quesnel@users.sourceforge.net>
 * Copyright (c) 2003-2011 ULCO http://www.univ-littoral.fr
 * Copyright (c) 2007-2015 INRA http://www.inra.fr
 *
 * See the AUTHORS or Authors.txt file for copyright owners and contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef VLE_EXT_DECISION_RESOURCES_HPP
#define VLE_EXT_DECISION_RESOURCES_HPP

#include <string>
#include <map>
#include <set>

namespace vle { namespace extension { namespace decision {

typedef std::pair < std::string, std::string > Resource;

typedef std::map < std::string, std::set < std::string > > Resources;

typedef  Resources::iterator ResourcesIterator;
typedef  Resources::const_iterator ResourcesConstIterator;

typedef std::pair < ResourcesIterator, ResourcesIterator> ResourcesIteratorPair;
typedef std::pair < ResourcesConstIterator, ResourcesConstIterator> ResourcesConstIteratorPair;

typedef std::map < std::string, bool> ResourceAvailability;

typedef std::pair < std::string, std::string > ActivityResource;

typedef std::multimap < std::string, std::string > ActivitiesResources;

typedef  ActivitiesResources::iterator ActivitiesResourcesIterator;
typedef  ActivitiesResources::const_iterator ActivitiesResourcesConstIterator;

typedef std::pair < ActivitiesResourcesIterator,
                    ActivitiesResourcesIterator> ActivitiesResourcesIteratorPair;
typedef std::pair < ActivitiesResourcesConstIterator,
                    ActivitiesResourcesConstIterator> ActivitiesResourcesConstIteratorPair;

}}} // namespace vle model decision

#endif
