/*
 * Copyright (c) 2014-2014 INRA http://www.inra.fr
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/*
 * @@tagdynamic@@
 * @@tagdepends: vle.discrete-time @@endtagdepends
 */

#include <vle/DiscreteTime.hpp>
#include <vle/devs/DynamicsDbg.hpp>
#include <iostream>

namespace vd = vle::devs;
namespace vv = vle::value;

namespace vle {
namespace discrete_time {
namespace test {



class executiveDT_B : public DiscreteTimeDyn
{
public:
    executiveDT_B(const vd::DynamicsInit& init, const vd::InitEventList& events)
        :  DiscreteTimeDyn(init,events)
    {
        a.init(this, "a", events);
        b.init(this, "b", events);
    }

    virtual ~executiveDT_B()
    {
    }


    void compute(const vd::Time& /*time*/)
    {
        b = a();
    }

    Var a;
    Var b;
};

DECLARE_DYNAMICS_DBG(executiveDT_B)

}}}

