/*
 * @file vle/extension/decision/test/ss.cpp
 *
 * This file is part of VLE, a framework for multi-modeling, simulation
 * and analysis of complex dynamical systems
 * http://www.vle-project.org
 *
 * Copyright (c) 2003-2007 Gauthier Quesnel <quesnel@users.sourceforge.net>
 * Copyright (c) 2003-2010 ULCO http://www.univ-littoral.fr
 * Copyright (c) 2007-2012 INRA http://www.inra.fr
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

#include <vle/utils/unit-test.hpp>
#include <iostream>
#include <iterator>
#include <vle/vle.hpp>
#include <vle/value/Double.hpp>
#include <vle/utils/Context.hpp>
#include <vle/extension/decision/KnowledgeBase.hpp>

namespace vmd = vle::extension::decision;

namespace vle { namespace extension { namespace decision { namespace ex {
    class Resourcetest_0: public vmd::KnowledgeBase
    {
    public:
        Resourcetest_0(vle::utils::ContextPtr ctxp)
            : vmd::KnowledgeBase(ctxp)
        {
            vmd::Activity& A = addActivity("A", 1.0, 10.0);
            vmd::Activity& B = addActivity("B", 1.0, 10.0);

            addResources("Farmer", "Bob");

            A.addResources(extendResources("Farmer"));
            B.addResources(extendResources("Farmer"));

        }

        virtual ~Resourcetest_0() {}
    };

    class Resourcetest_1: public vmd::KnowledgeBase
    {
    public:
        Resourcetest_1(vle::utils::ContextPtr ctxp)
            : vmd::KnowledgeBase(ctxp)
        {
            vmd::Activity& A = addActivity("A", 1.0, 10.0);
            vmd::Activity& B = addActivity("B", 1.0, 10.0);

            addResources("Farmer", "Bob");

            A.addResources(extendResources("Farmer"));
            B.addResources(extendResources("Farmer"));

            B.setPriority(1.);
        }

        virtual ~Resourcetest_1() {}
    };

    class Resourcetest_2: public vmd::KnowledgeBase
    {
    public:
        Resourcetest_2(vle::utils::ContextPtr ctxp)
            : vmd::KnowledgeBase(ctxp)
        {
            vmd::Activity& A = addActivity("A", 1.0, 10.0);
            vmd::Activity& B = addActivity("B", 1.0, 10.0);

            addResources("Farmer", "Bob");

            A.addResources(extendResources("Bob"));
            B.addResources(extendResources("Bob"));

        }

        virtual ~Resourcetest_2() {}
    };

    class Resourcetest_3: public vmd::KnowledgeBase
    {
    public:
        Resourcetest_3(vle::utils::ContextPtr ctxp)
            : vmd::KnowledgeBase(ctxp)
        {
            vmd::Activity& A = addActivity("A", 1.0, 10.0);
            vmd::Activity& B = addActivity("B", 1.0, 10.0);
            vmd::Activity& C = addActivity("C", 1.0, 10.0);

            addResources("Farmer", "Bob");
            addResources("Farmer", "Bill");
            addResources("Worker", "Tim");

            A.addResources(extendResources("Bob|Bill"));
            B.addResources(extendResources("Bob|Bill"));
            C.addResources(extendResources("Tim|Bob|Bill"));

        }

        virtual ~Resourcetest_3() {}
    };

    class Resourcetest_4: public vmd::KnowledgeBase
    {
    public:
        Resourcetest_4(vle::utils::ContextPtr ctxp)
            : vmd::KnowledgeBase(ctxp)
        {
            addRes("resFunc_1",  boost::bind(&Resourcetest_4::resFunc_1, this, _1, _2));
            addRes("resFunc_2", boost::bind(&Resourcetest_4::resFunc_2, this, _1, _2));

            vmd::Activity& A = addActivity("A", 1.0, 10.0);
            vmd::Activity& B = addActivity("B", 1.0, 10.0);
            vmd::Activity& C = addActivity("C", 1.0, 10.0);

            addResources("Farmer", "Bob");
            addResources("Farmer", "Bill");
            addResources("Worker", "Tim");

            A.addResources(extendResources(applyRes("resFunc_1","A",A)));
            B.addResources(extendResources(applyRes("resFunc_1","B",B)));
            C.addResources(extendResources(applyRes("resFunc_2","C",C)));

        }

        virtual ~Resourcetest_4() {}

        std::string resFunc_1(const std::string& /*name*/,
                              const Activity& /*activity*/) const
        {
            return "Bob|Bill";
        }


        std::string resFunc_2(const std::string& /*name*/,
                              const Activity& /*activity*/) const
        {
            return "Tim|Bob|Bill";
        }

    };

}}}} // namespace vle extension decision ex

/**
 * This test is here to exibit resources managment.
 *
 * Expected behavior :
 * A receive an ack "done" at 2
 * A : 0--WAIT--1--STARTED--2--DONE----------10
 * B : 0--WAIT-------------------6--STARTED--10
 *
 */
void resource_0()
{
    vle::utils::ContextPtr ctxp =  vle::utils::make_context();
    vmd::ex::Resourcetest_0 base(ctxp);
    vmd::Activities::result_t lst;

    base.processChanges(0.0);
    {
        const vmd::Activity& A =  base.activities().get("A")->second;
        const vmd::Activity& B =  base.activities().get("B")->second;
        EnsuresEqual(A.isInWaitState(), true);
        EnsuresEqual(B.isInWaitState(), true);
    }
    base.processChanges(1.0);
    {
        const vmd::Activity& A =  base.activities().get("A")->second;
        const vmd::Activity& B =  base.activities().get("B")->second;
        EnsuresEqual(A.isInStartedState(), true);
        vmd::ActivitiesResourcesConstIteratorPair pit;
        pit = base.activities().resources("A");
        EnsuresEqual(std::distance(pit.first, pit.second), 1);
        EnsuresEqual((*(pit.first)).second, "Bob");
        EnsuresEqual(B.isInWaitState(), true);
    }
    base.setActivityDone("A",2.0);
    base.processChanges(2.0);
    {
        const vmd::Activity& A =  base.activities().get("A")->second;
        const vmd::Activity& B =  base.activities().get("B")->second;
        EnsuresEqual(A.isInDoneState(), true);
        EnsuresEqual(B.isInStartedState(), true);
    }
}

void resource_1()
{
    vle::utils::ContextPtr ctxp =  vle::utils::make_context();
    vmd::ex::Resourcetest_1 base(ctxp);
    vmd::Activities::result_t lst;

    base.processChanges(0.0);
    {
        const vmd::Activity& A =  base.activities().get("A")->second;
        const vmd::Activity& B =  base.activities().get("B")->second;
        EnsuresEqual(A.isInWaitState(), true);
        EnsuresEqual(B.isInWaitState(), true);
    }
    base.processChanges(1.0);
    {
        const vmd::Activity& A =  base.activities().get("A")->second;
        const vmd::Activity& B =  base.activities().get("B")->second;
        EnsuresEqual(B.isInStartedState(), true);
        EnsuresEqual(A.isInWaitState(), true);
    }
    base.setActivityDone("B",2.0);
    base.processChanges(2.0);
    {
        const vmd::Activity& A =  base.activities().get("A")->second;
        const vmd::Activity& B =  base.activities().get("B")->second;
        EnsuresEqual(B.isInDoneState(), true);
        EnsuresEqual(A.isInStartedState(), true);
    }
}

void resource_2()
{
    vle::utils::ContextPtr ctxp =  vle::utils::make_context();
    vmd::ex::Resourcetest_2 base(ctxp);
    vmd::Activities::result_t lst;

    base.processChanges(0.0);
    {
        const vmd::Activity& A =  base.activities().get("A")->second;
        const vmd::Activity& B =  base.activities().get("B")->second;
        EnsuresEqual(A.isInWaitState(), true);
        EnsuresEqual(B.isInWaitState(), true);
    }
    base.processChanges(1.0);
    {
        const vmd::Activity& A =  base.activities().get("A")->second;
        const vmd::Activity& B =  base.activities().get("B")->second;
        EnsuresEqual(A.isInStartedState(), true);
        vmd::ActivitiesResourcesConstIteratorPair pit;
        pit = base.activities().resources("A");
        EnsuresEqual(std::distance(pit.first, pit.second), 1);
        EnsuresEqual((*(pit.first)).second, "Bob");
        EnsuresEqual(B.isInWaitState(), true);
    }
    base.setActivityDone("A",2.0);
    base.processChanges(2.0);
    {
        const vmd::Activity& A =  base.activities().get("A")->second;
        const vmd::Activity& B =  base.activities().get("B")->second;
        EnsuresEqual(A.isInDoneState(), true);
        EnsuresEqual(B.isInStartedState(), true);
    }
}

void resource_3()
{
    vle::utils::ContextPtr ctxp =  vle::utils::make_context();
    vmd::ex::Resourcetest_3 base(ctxp);
    vmd::Activities::result_t lst;

    base.processChanges(0.0);
    {
        const vmd::Activity& A =  base.activities().get("A")->second;
        const vmd::Activity& B =  base.activities().get("B")->second;
        const vmd::Activity& C =  base.activities().get("C")->second;
        EnsuresEqual(A.isInWaitState(), true);
        EnsuresEqual(B.isInWaitState(), true);
        EnsuresEqual(C.isInWaitState(), true);
    }

    base.processChanges(1.0);
    {
        const vmd::Activity& A =  base.activities().get("A")->second;
        const vmd::Activity& B =  base.activities().get("B")->second;
        const vmd::Activity& C =  base.activities().get("C")->second;
        EnsuresEqual(A.isInStartedState(), true);
        {
            vmd::ActivitiesResourcesConstIteratorPair pit;
            pit = base.activities().resources("A");
            EnsuresEqual(std::distance(pit.first, pit.second), 1);
            EnsuresEqual((*(pit.first)).second, "Bob");
        }
        EnsuresEqual(B.isInStartedState(), true);
        {
            vmd::ActivitiesResourcesConstIteratorPair pit;
            pit = base.activities().resources("B");
            EnsuresEqual(std::distance(pit.first, pit.second), 1);
            EnsuresEqual((*(pit.first)).second, "Bill");
        }
        EnsuresEqual(C.isInStartedState(), true);
        {
            vmd::ActivitiesResourcesConstIteratorPair pit;
            pit = base.activities().resources("C");
            EnsuresEqual(std::distance(pit.first, pit.second), 1);
            EnsuresEqual((*(pit.first)).second, "Tim");
        }
    }
}

void resource_4()
{
    vle::utils::ContextPtr ctxp =  vle::utils::make_context();
    vmd::ex::Resourcetest_4 base(ctxp);
    vmd::Activities::result_t lst;

    base.processChanges(0.0);
    {
        const vmd::Activity& A =  base.activities().get("A")->second;
        const vmd::Activity& B =  base.activities().get("B")->second;
        const vmd::Activity& C =  base.activities().get("C")->second;
        EnsuresEqual(A.isInWaitState(), true);
        EnsuresEqual(B.isInWaitState(), true);
        EnsuresEqual(C.isInWaitState(), true);
    }

    base.processChanges(1.0);
    {
        const vmd::Activity& A =  base.activities().get("A")->second;
        const vmd::Activity& B =  base.activities().get("B")->second;
        const vmd::Activity& C =  base.activities().get("C")->second;
        EnsuresEqual(A.isInStartedState(), true);
        {
            vmd::ActivitiesResourcesConstIteratorPair pit;
            pit = base.activities().resources("A");
            EnsuresEqual(std::distance(pit.first, pit.second), 1);
            EnsuresEqual((*(pit.first)).second, "Bob");
        }
        EnsuresEqual(B.isInStartedState(), true);
        {
            vmd::ActivitiesResourcesConstIteratorPair pit;
            pit = base.activities().resources("B");
            EnsuresEqual(std::distance(pit.first, pit.second), 1);
            EnsuresEqual((*(pit.first)).second, "Bill");
        }
        EnsuresEqual(C.isInStartedState(), true);
        {
            vmd::ActivitiesResourcesConstIteratorPair pit;
            pit = base.activities().resources("C");
            EnsuresEqual(std::distance(pit.first, pit.second), 1);
            EnsuresEqual((*(pit.first)).second, "Tim");
        }
    }
}

int main()
{
    resource_0();
    resource_1();
    resource_2();
    resource_3();
    resource_4();

    return unit_test::report_errors();
}
