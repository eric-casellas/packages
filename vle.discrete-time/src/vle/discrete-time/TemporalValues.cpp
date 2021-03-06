/*
 * This file is part of VLE, a framework for multi-modeling, simulation
 * and analysis of complex dynamical systems
 * http://www.vle-project.org
 *
 * Copyright (c) 2013-2014 INRA http://www.inra.fr
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


#include <vle/discrete-time/TemporalValues.hpp>
#include <vle/devs/InitEventList.hpp>
#include <vle/value/Tuple.hpp>
#include <vle/value/Table.hpp>
#include <vle/value/Double.hpp>
#include <vle/value/Boolean.hpp>
#include <vle/value/Integer.hpp>
#include <vle/utils/Exception.hpp>
#include <vle/utils/Tools.hpp>
#include <iostream>
#include <iomanip>      // std::setprecision
#include <sstream>

namespace vle {
namespace discrete_time {

namespace vu = vle::utils;

VarUpdate::VarUpdate(const vle::devs::Time& t, double val):
                        timeOfUpdate(t), value(val)
{
}


VectUpdate::VectUpdate(const vle::devs::Time& t, unsigned int dim):
    timeOfUpdate(t), value(dim, 0.0), complete(true)
{
}

VectUpdate::VectUpdate(const vle::devs::Time& t, unsigned int dim,
        bool complet):
    timeOfUpdate(t), value(dim, 0.0), complete(complet)
{
}

VectUpdate::VectUpdate(const vle::devs::Time& t, const vle::value::Tuple& val):
        timeOfUpdate(t), value(val.value()), complete(true)
{
}

VectUpdate::VectUpdate(const devs::Time& t, const value::Table& tab,
        unsigned int col):
        timeOfUpdate(t), value(), complete(true)
{
    value.resize(tab.height());
    for (unsigned int r=0; r<tab.height(); r++) {
        value[r]=tab.get(col,r);
    }
}

VectUpdate::~VectUpdate()
{
}

void
VectUpdate::assign(const vle::value::Tuple& v)
{
    if (v.size() != value.size()) {
        throw vle::utils::InternalError(
                "VectUpdate::assign v.size() != value.size()\n");
    }
    vle::value::Tuple::const_iterator itb = v.value().begin();
    vle::value::Tuple::const_iterator ite = v.value().end();
    std::vector<double>::iterator itin = value.begin();
    for (; itb!=ite; itb++) {
        *itin = *itb;
    }

}

VarValueUpdate::VarValueUpdate(const VarValueUpdate& v):
        timeOfUpdate(v.timeOfUpdate), value(v.value->clone())
{

}

VarValueUpdate::VarValueUpdate(const vle::devs::Time& t,
        std::unique_ptr<vle::value::Value> val):
                timeOfUpdate(t), value(std::move(val))
{

}

VarValueUpdate::~VarValueUpdate()
{
    value.reset();
}

VarInterface::VarInterface(TemporalValuesProvider* tvpin):
            error_no_sync(false), allow_update(false),
            history_size_given(false), history_size(2), init_value(nullptr),
            tvp(tvpin)
{
}

VarInterface::~VarInterface()
{
    init_value.reset();
}

bool VarInterface::isVarMono() const
{
    return getType() == MONO;
}
bool VarInterface::isVarMulti() const
{
    return getType() == MULTI;
}
bool VarInterface::isVarValue() const
{
    return getType() == VALUE_VLE;
}
const VarMono& VarInterface::toVarMono() const
{
    return static_cast < const VarMono& >(*this);
}
const VarMulti& VarInterface::toVarMulti() const
{
    return static_cast < const VarMulti& >(*this);
}
const VarValue& VarInterface::toVarValue() const
{
    return static_cast < const VarValue& >(*this);
}
VarMono& VarInterface::toVarMono()
{
    return static_cast <VarMono& >(*this);
}
VarMulti& VarInterface::toVarMulti()
{
    return static_cast <VarMulti& >(*this);
}
VarValue& VarInterface::toVarValue()
{
    return static_cast <VarValue& >(*this);
}

void
VarInterface::initHistoryVar(const std::string& varName,
        const vle::devs::Time& t)
{
    switch (getType()) {
    case MONO: {
        VarMono* itVar = dynamic_cast<VarMono*>(this);
        if (itVar->init_value) {
            if (itVar->init_value->isTuple()) {
                const vle::value::Tuple& tuple =
                        itVar->init_value->toTuple();
                if (itVar->history_size_given and
                        (tuple.size() != itVar->history_size)) {
                    throw vle::utils::ModellingError(
                            vle::utils::format("[%s] Error initialization of variable"
                                " '%s' (history size not eq tuple size)\n",
                                tvp->get_model_name().c_str(), varName.c_str()));
                }
                for (unsigned int h = 0; h < tuple.size() ; h++) {
                    itVar->history.push_front(
                            new VarUpdate(t-h*tvp->getDelta(), tuple.at(h)));
                }
            } else if (itVar->init_value->isDouble()) {
                itVar->history.push_front(
                        new VarUpdate(t,
                                itVar->init_value->toDouble().value()));
            } else if (itVar->init_value->isInteger()) {
                itVar->history.push_front(
                        new VarUpdate(t, (double)
                                itVar->init_value->toInteger().value()));
            } else {
                throw vle::utils::ModellingError(
                        vu::format("[%s] Error initialisation of variable '%s'"
                                " (expect Tuple, Double or Integer)\n",
                                tvp->get_model_name().c_str(), varName.c_str()));
            }
        } else {
            for (unsigned int h = 0; h < itVar->history_size ; h++) {
                itVar->history.push_front(
                        new VarUpdate(t-h * tvp->getDelta(), 0.0));
            }
        }
        break;
    } case MULTI: {
        VarMulti* itVar = dynamic_cast<VarMulti*>(this);
        if (itVar->init_value) {
            if (itVar->init_value->isTuple()) {
                const vle::value::Tuple& tuple = itVar->init_value->toTuple();
                itVar->history.push_back(new VectUpdate(t, tuple));
            } else if (itVar->init_value->isTable()) {
                const vle::value::Table& tab = itVar->init_value->toTable();
                if (itVar->history_size_given and
                        (tab.width() != itVar->history_size)) {
                    throw utils::ModellingError(utils::format(
                            "[%s] Error initialization of variable"
                            " '%s' (history size not eq table height)\n",
                            tvp->get_model_name().c_str(), varName.c_str()));
                }
                if (itVar->dim != tab.height()) {
                    throw utils::ModellingError(utils::format(
                            "[%s] Error initialization of variable"
                            " '%s' (dim not eq table height)\n",
                            tvp->get_model_name().c_str(), varName.c_str()));
                }
                for (unsigned int c=0; c<tab.width(); c++) {
                    itVar->history.push_front(new VectUpdate(
                            t-c*tvp->getDelta(), tab, c));
                }
            } else {

                throw vle::utils::ModellingError(
                        vu::format("[%s] Error initialisation of variable '%s'"
                                " (expect Tuple or Table)\n",
                                tvp->get_model_name().c_str(), varName.c_str()));
            }
        } else {
            itVar->history.push_back(new VectUpdate(t, itVar->dim));
        }
        break;
    } case VALUE_VLE: {
        VarValue* itVar = dynamic_cast<VarValue*>(this);
        if (itVar->init_value) {
            for (unsigned int h = 0; h < itVar->history_size ; h++) {
                itVar->history.push_front(
                        new VarValueUpdate(t-h * tvp->getDelta(),
                                itVar->init_value->clone()));
            }
        } else {
            for (unsigned int h = 0; h < itVar->history_size ; h++) {
                itVar->history.push_front(
                        new VarValueUpdate(t-h * tvp->getDelta(),
                                std::unique_ptr<vle::value::Value>(
                                        new vle::value::Double(0.0))));
            }
        }
        break;
    }}
}

double
VarMono::getDefaultInit()
{
    return 0.0;
}

VarMono::VarMono(TemporalValuesProvider* tvpin):
        VarInterface(tvpin), history(), snapshot(0)
{
}

VarMono::~VarMono()
{
    History::iterator itb = history.begin();
    History::iterator ite = history.end();
    for (; itb!=ite; itb++) {
        delete *itb;
    }
    delete snapshot;
}

double
VarMono::getVal(const vle::devs::Time& t, double delay) const
{
    //TODO only constant piecewise function
    double reqTime = t+delay;
    History::const_reverse_iterator itb = history.rbegin();
    History::const_reverse_iterator ite = history.rend();

    for (;itb != ite ; itb++){
        if((*itb)->timeOfUpdate <= reqTime + std::pow(10, -9)){
            return (*itb)->value;
        }
    }
    throw vle::utils::InternalError(
        vu::format("[%s] getVal not found, called "
                " with t='%f', delay='%f' \n",
                tvp->get_model_name().c_str(), t, delay ));
}

VAR_TYPE
VarMono::getType() const
{
    return MONO;
}

void
VarMono::update(const vle::devs::Time& t, double val)
{
    VarUpdate& varUpdate = *history.back();
    if (varUpdate.timeOfUpdate == t) {
        if (allow_update) {
            varUpdate.value = val;
        }
    } else {
        history.push_back(new VarUpdate(t,val));
        if (history.size() > history_size) {
            delete history.front();
            history.pop_front();
        }
    }
}

void
VarMono::update(const vle::devs::Time& t,
        const vle::value::Value& val)
{
    switch (val.getType()) {
    case value::Value::DOUBLE:
        this->update(t,val.toDouble().value());
        break;
    case value::Value::MAP:
        //compatibility with old behavior: deprecated
        this->update(t,val.toMap().getDouble("value"));
        break;
    default:
        throw vle::utils::ModellingError(vu::format("[%s] bad value type as "
                "input of vle discrete time model",
                tvp->get_model_name().c_str()));
    }
}



vle::devs::Time VarMono::lastUpdateTime() const
{
    return history.back()->timeOfUpdate;
}

double
VarMono::lastVal(const vle::devs::Time& beg, const vle::devs::Time& end)
{
    History::const_reverse_iterator itb = history.rbegin();
    History::const_reverse_iterator ite = history.rend();

    for (;itb != ite ; itb++){
        if(((*itb)->timeOfUpdate >= beg) and
            ((*itb)->timeOfUpdate < end)){
            return (*itb)->value;
        }
    }
    throw vle::utils::ModellingError(
            vu::format("[%s] lastVal wrong interval [%f;%f[ \n",
            tvp->get_model_name().c_str(), beg, end));
}


void
VarMono::addSnapshot(SNAPSHOT_ID idSnap,  double val)
{
    if (snapshot == 0) {
        snapshot = new Snapshot();
    }
    (*snapshot)[idSnap] = val;
}

bool
VarMono::hasSnapshot(SNAPSHOT_ID idSnap)
{
    if (snapshot == 0) {
        return false;
    }
    return (snapshot->find(idSnap) != snapshot->end());
}

double
VarMono::getSnapshot(SNAPSHOT_ID idSnap)
{
    if (snapshot == 0) {
        throw "error";
    }
    Snapshot::const_iterator itf = snapshot->find(idSnap);
    if (itf == snapshot->end()) {
        throw "error";
    }
    return itf->second;
}

void VarMono::clearSnapshot()
{
    if (snapshot) {
        snapshot->clear();
    }
}

VarMulti::VarMulti(TemporalValuesProvider* tvpin, unsigned int dimension):
        VarInterface(tvpin), history(),  snapshot(0), dim(dimension)
{
}

VarMulti::~VarMulti()
{
    History::iterator itb = history.begin();
    History::iterator ite = history.end();
    for (; itb!=ite; itb++) {
        delete *itb;
    }
    delete snapshot;
}

VAR_TYPE
VarMulti::getType() const
{
    return MULTI;
}

double
VarMulti::getVal(unsigned int i, const vle::devs::Time& t,
        double delay) const
{
    if (i >= dim) {
        throw vle::utils::ModellingError(vu::format("[%s] tried to access "
                "to index `%u` of a Vect of size `%u`.",
                tvp->get_model_name().c_str(), i, dim));
    }

    const std::vector<double>& value = getVal(t,delay);
    return value[i];
}

const std::vector<double>&
VarMulti::getVal(const vle::devs::Time& t, double delay) const
{
    double reqTime = t+delay;
    History::const_reverse_iterator itb = history.rbegin();
    History::const_reverse_iterator ite = history.rend();
    for (;itb != ite ; itb++){
        if((*itb)->timeOfUpdate <= reqTime){
            return (*itb)->value;
        }
    }
    throw vle::utils::InternalError(
            vu::format("[%s] getVal not found, called with t='%f', "
                    "delay='%f'\n",
                tvp->get_model_name().c_str(), t, delay));
}

void
VarMulti::update(const vle::devs::Time& t, const vle::value::Value& val)
{
    VectUpdate& vectUpdate = *history.back();
    if (vectUpdate.timeOfUpdate == t) {
        if (allow_update) {
            vectUpdate.assign(val.toTuple());
        }
    } else {
        history.push_back(new VectUpdate(t,val.toTuple()));
        if(history.size() > history_size){
            delete history.front();
            history.pop_front();
        }
    }
}

void
VarMulti::update(const vle::devs::Time& t, unsigned int d, double val)
{
    VectUpdate& vectUpdate = *history.back();
    if (vectUpdate.timeOfUpdate == t) {
        if (allow_update or not vectUpdate.complete) {
            vectUpdate.value[d] = val;
        }
    } else {
        VectUpdate* vu = new VectUpdate(t, dim, false);
        vu->value[d] = val;
        history.push_back(vu);
        if(history.size() > history_size){
            delete history.front();
            history.pop_front();
        }
    }
}

vle::devs::Time
VarMulti::lastUpdateTime() const
{
    return history.back()->timeOfUpdate;
}

void
VarMulti::addSnapshot(SNAPSHOT_ID idSnap, const std::vector<double>& val)
{
    if (snapshot == 0) {
        snapshot = new Snapshot();
    }
    std::vector<double>& v = (*snapshot)[idSnap];
    v.clear();
    std::vector<double>::const_iterator itb = val.begin();
    std::vector<double>::const_iterator ite = val.end();
    for (; itb != ite; itb++) {
        v.push_back(*itb);
    }
}

bool
VarMulti::hasSnapshot(SNAPSHOT_ID idSnap)
{
    if (snapshot == 0) {
        return false;
    }
    Snapshot::iterator itf = snapshot->find(idSnap);
    return (itf != snapshot->end());
}

const std::vector<double>&
VarMulti::getSnapshot(SNAPSHOT_ID idSnap)
{
    if (snapshot == 0) {
        throw "error";
    }
    Snapshot::iterator itf = snapshot->find(idSnap);
    if (itf == snapshot->end()) {
        throw "error";
    }
    return itf->second;
}

void
VarMulti::clearSnapshot()
{
    if (snapshot == 0) {
       snapshot->clear();
    }
}

VarValue::VarValue(TemporalValuesProvider* tvpin):
        VarInterface(tvpin), history(), snapshot(0)
{

}

VarValue::~VarValue()
{
    {
        History::iterator itb = history.begin();
        History::iterator ite = history.end();
        for (; itb!=ite; itb++) {
            delete *itb;
        }
    }
    {
        if (snapshot) {
            clearSnapshot();
            delete snapshot;
        }
    }



}

VAR_TYPE
VarValue::getType() const
{
    return VALUE_VLE;
}

const vle::value::Value&
VarValue::getVal(const vle::devs::Time& t, double delay) const
{
    double reqTime = t+delay;
    History::const_reverse_iterator itb = history.rbegin();
    History::const_reverse_iterator ite = history.rend();
    for (;itb != ite ; itb++){
        if((*itb)->timeOfUpdate <= reqTime){
            return *((*itb)->value);
        }
    }
    throw vle::utils::InternalError(
            vu::format("[%s] getVal not found, called "
                    " with t='%f', delay='%f' \n",
                      tvp->get_model_name().c_str(), t, delay));
}

void
VarValue::update(const vle::devs::Time& t, const vle::value::Value& val)
{
    VarValueUpdate& varUpdate = *history.back();
    if (varUpdate.timeOfUpdate == t) {
        if (allow_update) {
            varUpdate.value.reset();
            varUpdate.value = val.clone();
        }
    } else {
        history.push_back(new VarValueUpdate(t,val.clone()));
        if(history.size() > history_size){
            history.pop_front();
        }
    }
}

vle::devs::Time
VarValue::lastUpdateTime() const
{
    return history.back()->timeOfUpdate;
}

const vle::value::Value&
VarValue::lastVal(const vle::devs::Time& beg, const vle::devs::Time& end)
{
    History::const_reverse_iterator itb = history.rbegin();
    History::const_reverse_iterator ite = history.rend();

    for (;itb != ite ; itb++){
        if(((*itb)->timeOfUpdate >= beg) and
                ((*itb)->timeOfUpdate < end)){
            return *((*itb)->value);
        }
    }
    throw vle::utils::ModellingError(
            vu::format("[%s] lastVal wrong interval [%f;%f[ \n",
            tvp->get_model_name().c_str(), beg, end));
}

void
VarValue::addSnapshot(SNAPSHOT_ID idSnap, const vle::value::Value& val)
{
    if (snapshot == 0) {
        snapshot = new Snapshot();
    }
    Snapshot::iterator itf = snapshot->find(idSnap);
    if (itf == snapshot->end()) {
        (*snapshot)[idSnap] = std::unique_ptr<vle::value::Value>(val.clone());
    } else {
        itf->second.reset();
        itf->second = std::unique_ptr<vle::value::Value>(val.clone());
    }
}

bool
VarValue::hasSnapshot(SNAPSHOT_ID idSnap)
{
    if (snapshot == 0) {
        return false;
    }
    Snapshot::iterator itf = snapshot->find(idSnap);
    return (itf != snapshot->end());
}

const vle::value::Value&
VarValue::getSnapshot(SNAPSHOT_ID idSnap)
{
    if (snapshot == 0) {
        throw "error";
    }
    Snapshot::iterator itf = snapshot->find(idSnap);
    if (itf == snapshot->end()) {
        throw "error";
    }
    return *(itf->second);
}

void
VarValue::clearSnapshot()
{
    if (snapshot != 0) {
        Snapshot::iterator itb = snapshot->begin();
        Snapshot::iterator ite = snapshot->end();
        for (; itb!=ite; itb++) {
            itb->second.reset();
        }
        snapshot->clear();
    }
}

Var::Var(): name(""), itVar(0)
{
}

Var::~Var()
{
}


void
Var::init(TemporalValuesProvider* tvpin, const std::string& varName,
        const devs::InitEventList& initMap, const devs::Time& currentTime)
{
    Var::init(tvpin, varName, initMap);
    itVar->initHistoryVar(varName, currentTime);
}

void
Var::init(TemporalValuesProvider* tvpin, const std::string& varName,
        const devs::InitEventList& initMap)
{
    devs::InitEventList::const_iterator ite = initMap.end();
    devs::InitEventList::const_iterator itf = ite;
    std::string port;

    name.assign(varName);
    std::pair<Variables::iterator, bool> resInsert =
            tvpin->getVariables().insert(
                    std::make_pair(name, new VarMono(tvpin)));
    if (not resInsert.second) {
        throw vle::utils::ModellingError(vu::format("[%s] Var '%s' "
                "already declared \n",
                tvpin->get_model_name().c_str(),  name.c_str()));
    }
    itVar = dynamic_cast<VarMono*>(resInsert.first->second);

    port.assign("error_no_sync_");
    port += name;
    itf = initMap.find(port);
    if (itf != ite) {
        itVar->error_no_sync = itf->second->toBoolean().value();
    } else {
        itVar->error_no_sync = false;//Default
    }

    port.assign("keep_first_value_");
    port += name;
    itf = initMap.find(port);
    if (itf != ite) {
        itVar->allow_update = itf->second->toBoolean().value();
    } else {
        itVar->allow_update = false;//Default
    }

    port.assign("history_size_");
    port += name;
    itf = initMap.find(port);
    itVar->history_size = 3;
    itVar->history_size_given = false;
    if (itf != ite) {
        itVar->history_size = itf->second->toInteger().value();
        itVar->history_size_given = true;
    }

    port.assign("init_value_");
    port += name;
    itf = initMap.find(port);
    if (itf != ite) {
        itVar->init_value = itf->second->clone();
    }

}

void
Var::init_value(double v)
{
    if (!itVar) {
        throw vle::utils::ModellingError("init_value for Var can be called "
                " only after init function \n");
    }
    itVar->init_value.reset();
    itVar->init_value = std::unique_ptr<vle::value::Value>(
            new vle::value::Double(v));
}

void
Var::init_history(const vle::devs::Time& t)
{
    if (!itVar) {
        throw vle::utils::ModellingError("init_history for Var "
                "can be called only after init function \n");
    }
    itVar->initHistoryVar(name,t);
}

void
Var::history_size(unsigned int s)
{
    itVar->history_size = s;
}

double
Var::operator()(double delay) const
{
    if (itVar->history.size() == 0) {
        throw vle::utils::ModellingError(
                vu::format("[%s] operator(delay) for variable '%s' cannot be"
                        " used because history is empty \n",
                        itVar->tvp->get_model_name().c_str(), name.c_str()));
    }
    return itVar->getVal(itVar->tvp->getCurrentTime(),
                         delay * itVar->tvp->getDelta());
}

double
Var::operator()() const
{

    if (itVar->history.size() == 0) {
        if (itVar->init_value) {
            return itVar->init_value->toDouble().value();
        } else {
            return VarMono::getDefaultInit();
        }
    }
    return itVar->history.back()->value;
}

double
Var::lastVal(double delayBeg, double delayEnd) const
{
    return itVar->lastVal(
            itVar->tvp->getCurrentTime() + delayBeg * itVar->tvp->getDelta(),
            itVar->tvp->getCurrentTime() + delayEnd * itVar->tvp->getDelta());
}

void
Var::operator=(double v)
{
    if (!itVar) {
        throw vle::utils::ModellingError("Operator= cannot be used before "
                " init \n");
    }
    if (itVar->history.size() == 0) {
        itVar->init_value.reset();
        itVar->init_value = std::unique_ptr<vle::value::Value>(
                new vle::value::Double(v));
    } else {
        itVar->update(itVar->tvp->getCurrentTime(), v);
    }
}

Vect_i::Vect_i(VarMulti* itv, unsigned int d):
                    itVar(itv), dim(d)
{

}

double
Vect_i::operator()(double delay)
{
    return itVar->getVal(dim, itVar->tvp->getCurrentTime(), delay);
}

double
Vect_i::operator()()
{
    if (itVar->history.size() == 0) {
        //get init value
        if (!(itVar->init_value && itVar->init_value->isTuple()
                && itVar->init_value->toTuple().size() == itVar->dim)) {
            return VarMono::getDefaultInit();
        } else {
           return itVar->init_value->toTuple().value()[dim];
        }
    } else {
        return itVar->getVal(dim, itVar->tvp->getCurrentTime(), 0);
    }
}


void
Vect_i::operator=(double val)
{
    if (itVar->history.size() == 0) {
        //update init value
        if (!(itVar->init_value && itVar->init_value->isTuple()
                && itVar->init_value->toTuple().size() == itVar->dim)) {
            itVar->init_value.reset();
            itVar->init_value = std::unique_ptr<vle::value::Value>(
                    new vle::value::Tuple(itVar->dim, 0));
        }
        itVar->init_value->toTuple().value()[dim] = val;
    } else {
        itVar->update(itVar->tvp->getCurrentTime(), dim, val);
    }
}


Vect::Vect() : name(), itVar(0)
{
}

Vect::~Vect()
{
}

void
Vect::init(TemporalValuesProvider* tvpin, const std::string& varName,
        const devs::InitEventList& initMap, const devs::Time& currentTime)
{
    init(tvpin, varName, initMap);
    itVar->initHistoryVar(varName, currentTime);
}

void
Vect::init(TemporalValuesProvider* tvpin, const std::string& varName,
        const devs::InitEventList& initMap)
{
    devs::InitEventList::const_iterator ite = initMap.end();
    devs::InitEventList::const_iterator itf = ite;
    std::string port;

    name.assign(varName);

    port.assign("dim_");
    port += varName;
    itf = initMap.find(port);
    unsigned int dim = 2;//Default
    if (itf != ite) {
        dim = itf->second->toInteger().value();
    }

    std::pair<Variables::iterator, bool> resInsert =
            tvpin->getVariables().insert(std::make_pair(name,
                    new VarMulti(tvpin, dim)));
    if (not resInsert.second) {
        throw vle::utils::ModellingError(vu::format("[%s] Vect '%s' "
                "already declared \n", tvpin->get_model_name().c_str(),
                name.c_str()));
    }
    itVar = dynamic_cast<VarMulti*>(resInsert.first->second);
    itVar->dim = dim;

    port.assign("error_no_sync_");
    port += varName;
    itf = initMap.find(port);
    itVar->error_no_sync = false;
    if (itf != ite) {
        itVar->error_no_sync = itf->second->toBoolean().value();
    }

    port.assign("allow_update_");
    port += varName;
    itf = initMap.find(port);
    itVar->allow_update = false; //Default
    if (itf != ite) {
        itVar->allow_update = itf->second->toBoolean().value();
    }

    port.assign("init_value_");
    port += varName;
    itf = initMap.find(port);
    itVar->init_value  = 0;
    if (itf != ite) {
        itVar->init_value = itf->second->clone();
    }
}

void
Vect::history_size(unsigned int s)
{
    itVar->history_size = s;
}

void
Vect::dim(unsigned int s)
{
    dynamic_cast<VarMulti*>(itVar)->dim = s;
}

unsigned int
Vect::dim() const
{
    return dynamic_cast<const VarMulti*>(itVar)->dim;
}

Vect_i
Vect::operator[](unsigned int i)
{
    if (i >= dim()) {
        throw vle::utils::ModellingError(vu::format("[%s] error access to index"
                " `%u` of Vect `%s` which has size `%u`.",
                itVar->tvp->get_model_name().c_str(), i, name.c_str(), dim()));
    }
    return Vect_i(itVar,i);
}

ValueVle::ValueVle(): name(""), itVar(0)
{
}

ValueVle::~ValueVle()
{
}

void
ValueVle::init(TemporalValuesProvider* tvpin, const std::string& varName,
        const devs::InitEventList& initMap, const vle::devs::Time& currentTime)
{
    init(tvpin, varName, initMap, 0);
    itVar->initHistoryVar(varName, currentTime);
}

void
ValueVle::init(TemporalValuesProvider* tvpin, const std::string& varName,
        const devs::InitEventList& initMap)
{
    devs::InitEventList::const_iterator ite = initMap.end();
    devs::InitEventList::const_iterator itf = ite;
    std::string port;

    name.assign(varName);

    std::pair<Variables::iterator, bool> resInsert =
            tvpin->getVariables().insert(std::make_pair(name,
                    new VarValue(tvpin)));
    if (not resInsert.second) {
        throw vle::utils::ModellingError(vu::format("[%s] ValueVle '%s' "
                "already declared \n", tvpin->get_model_name().c_str(),
                name.c_str()));
    }
    itVar = dynamic_cast<VarValue*>(resInsert.first->second);

    port.assign("history_size_");
    port += name;
    itf = initMap.find(port);
    itVar->history_size = 3;//Default
    itVar->history_size_given = false;
    if (itf != ite) {
        itVar->history_size = itf->second->toInteger().value();
        itVar->history_size_given = true;
    }

    port.assign("init_value_");
    port += varName;
    itf = initMap.find(port);
    itVar->init_value  = 0;
    if (itf != ite) {
        itVar->init_value = itf->second->clone();
    }

    port.assign("error_no_sync_");
    port += varName;
    itf = initMap.find(port);
    itVar->error_no_sync = false;
    if (itf != ite) {
        itVar->error_no_sync = itf->second->toBoolean().value();
    }

    port.assign("allow_update_");
    port += varName;
    itf = initMap.find(port);
    itVar->allow_update = false; //Default
    if (itf != ite) {
        itVar->allow_update = itf->second->toBoolean().value();
    }

}

void
ValueVle::history_size(unsigned int s)
{
    dynamic_cast<VarValue*>(itVar)->history_size = s;
}

const vle::value::Value&
ValueVle::operator()(double delay) const
{
    return itVar->getVal(itVar->tvp->getCurrentTime(),
                         delay * itVar->tvp->getDelta());
}

vle::value::Value&
ValueVle::operator()()
{
    return *(itVar->history.back()->value);
}

const vle::value::Value&
ValueVle::lastVal(double delayBeg, double delayEnd) const
{
    return itVar->lastVal(
            itVar->tvp->getCurrentTime() + delayBeg * itVar->tvp->getDelta(),
            itVar->tvp->getCurrentTime() + delayEnd * itVar->tvp->getDelta());
}

void
ValueVle::operator=(const vle::value::Value& val)
{
    itVar->update(itVar->tvp->getCurrentTime(), val);
}


TemporalValuesProvider::TemporalValuesProvider():
        model_name("unknown"), variables(), current_time(0), delta(1)
{
}

TemporalValuesProvider::TemporalValuesProvider(const std::string& modelname,
        const vle::devs::InitEventList& initMap):
        model_name(modelname), variables(), current_time(0), delta(1.0)
{
    auto itf = initMap.find("time_step");
    auto ite = initMap.end();
    if (itf != ite) {
        delta = itf->second->toDouble().value();
    }
}

TemporalValuesProvider::~TemporalValuesProvider()
{
    Variables::iterator itb = variables.begin();
    Variables::iterator ite = variables.end();
    for (; itb!=ite; itb++) {
        delete itb->second;
    }
}

void
TemporalValuesProvider::initHistory(const vle::devs::Time& t)
{
    Variables::iterator itb = variables.begin();
    Variables::iterator ite = variables.end();
    for (; itb != ite; itb++) {
        itb->second->initHistoryVar(itb->first, t);
    }
}

void
TemporalValuesProvider::snapshot(SNAPSHOT_ID idSnap)
{
    Variables::iterator itb = variables.begin();
    Variables::iterator ite = variables.end();
    for (; itb != ite; itb++) {
        switch(itb->second->getType()) {
        case MONO: {
            VarMono* itv = dynamic_cast<VarMono*>(itb->second);
            if (itv->history.size() == 0){
                throw vle::utils::ModellingError(
                        vu::format("[%s] Error snapshot of variable '%s'"
                                " (maybe you forgot to call initHistory)\n",
                                get_model_name().c_str(), itb->first.c_str()));
            }
            itv->addSnapshot(idSnap, itv->history.back()->value);
            break;
        } case MULTI: {
            VarMulti* itv = dynamic_cast<VarMulti*>(itb->second);
            if (itv->history.size() == 0){
                throw vle::utils::ModellingError(
                        vu::format("[%s] Error snapshot of variable '%s'"
                                " (maybe you forgot to call initHistory)\n",
                                get_model_name().c_str(), itb->first.c_str()));
            }
            itv->addSnapshot(idSnap, itv->history.back()->value);
            break;
        } case VALUE_VLE: {
            VarValue* itv = dynamic_cast<VarValue*>(itb->second);
            itv->addSnapshot(idSnap, *(itv->history.back()->value));
            break;
        }}
    }
}

unsigned int
TemporalValuesProvider::dim(const Vect& v) const
{
    return v.itVar->dim;
}

const std::string&
TemporalValuesProvider::get_model_name() const
{
    return model_name;
}

Variables&
TemporalValuesProvider::getVariables()
{
    return variables;
}

const Variables&
TemporalValuesProvider::getVariables() const
{
    return variables;
}

const vle::devs::Time&
TemporalValuesProvider::getCurrentTime() const
{
    return current_time;
}

double
TemporalValuesProvider::getDelta() const
{
    return delta;
}

void
TemporalValuesProvider::setCurrentTime(const vle::devs::Time& t)
{
    current_time = t;
}

std::string
TemporalValuesProvider::getVarName(VarInterface* v)
{
    Variables::iterator itb = variables.begin();
    Variables::iterator ite = variables.end();
    for (; itb != ite; itb++) {
        if (itb->second == v) {
            return itb->first;
        }
    }
    return "";
}

}} // namespace
