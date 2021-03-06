/*
 *
 * This file is part of VLE, a framework for multi-modeling, simulation
 * and analysis of complex dynamical systems
 * http://www.vle-project.org
 *
 * Copyright (c) 2013-2013 INRA http://www.inra.fr
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


#ifndef VLE_DISCRETE_TIME_COMPUTE_INTERFACE_HPP
#define VLE_DISCRETE_TIME_COMPUTE_INTERFACE_HPP 1

#include <vle/devs/Time.hpp>
#include <vle/devs/Dynamics.hpp>

namespace vle {
namespace discrete_time {

class ComputeInterface
{
public:
    ComputeInterface()
    {
    }
    virtual ~ComputeInterface(){}
    virtual void compute(const vle::devs::Time& t) = 0;
    virtual void handleExtEvt(const vle::devs::Time& t,
                              const vle::devs::ExternalEventList& ext) = 0;
    virtual void outputVar(const vle::vpz::AtomicModel& model,
                           const vle::devs::Time& time,
                           vle::devs::ExternalEventList& output) = 0;
    virtual vle::devs::Dynamics* toDynamics() = 0;
};

}} // namespaces

#endif
