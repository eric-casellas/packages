/*
 * @file vle/examples/decision/OnlyAgentWakeUp.cpp
 *
 * This file is part of VLE, a framework for multi-modeling, simulation
 * and analysis of complex dynamical systems
 * http://www.vle-project.org
 *
 * Copyright (c) 2003-2007 Gauthier Quesnel <quesnel@users.sourceforge.net>
 * Copyright (c) 2003-2011 ULCO http://www.univ-littoral.fr
 * Copyright (c) 2007-2011 INRA http://www.inra.fr
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

/*
 * @@tagdynamic@@
 * @@tagdepends: vle.extension.decision @@endtagdepends
 */

#include <vle/extension/Decision.hpp>
#include <sstream>

namespace vd = vle::devs;
namespace vv = vle::value;
namespace vmd = vle::extension::decision;

namespace vle { namespace examples { namespace decision {

using namespace std::placeholders;

class OnlyAgentWakeUp : public vmd::Agent
{
public:
    OnlyAgentWakeUp(const vd::DynamicsInit& mdl, const vd::InitEventList& evts)
        : vmd::Agent(mdl, evts), mWakeup(false)
    {
        addFact("wakeup", std::bind(&OnlyAgentWakeUp::wakeup, this,
                                    std::placeholders::_1));

        vmd::Activity& a = addActivity("A", 1.0, 100.0);

        a.addOutputFunction(std::bind(
                &OnlyAgentWakeUp::aout, this,
                std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3));

        vmd::Rule& r = addRule("r1");
        r.add(std::bind(&OnlyAgentWakeUp::isWakeup, this));

        a.addRule("r1", r);

        vmd::Activity& b = addActivity("B", 6.0, 100.0);
        b.addOutputFunction(std::bind(
                &OnlyAgentWakeUp::aout, this,
                std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3));
    }

    virtual ~OnlyAgentWakeUp()
    {
    }

    void wakeup(const vv::Value& val)
    {
        if (not mWakeup) {
            mWakeup = val.toBoolean().value();
        }
    }

    bool isWakeup() const
    {
        return mWakeup;
    }

    void aout(const std::string& /*name*/, const vmd::Activity& activity,
              vd::ExternalEventList& out)
    {
        if (activity.isInStartedState()) {
            out.emplace_back("out");
        }
    }

private:
    bool mWakeup;
};

}}} // namespace vle examples decision

DECLARE_DYNAMICS(vle::examples::decision::OnlyAgentWakeUp)
