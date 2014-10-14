
/*
 * Copyright (C) 2009-2014 INRA
 *
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

#ifndef VLE_RECURSIVE_VLEAPIFACILITIES_HPP_
#define VLE_RECURSIVE_VLEAPIFACILITIES_HPP_

#include <boost/algorithm/string/predicate.hpp>

#include <vle/vpz/Vpz.hpp>
#include <vle/utils/Exception.hpp>
#include <vle/manager/Manager.hpp>
#include <vle/value/Double.hpp>

#include <vle/oov/Plugin.hpp>
#include <vle/devs/Dynamics.hpp>

namespace vle {
namespace recursive {

namespace vu = vle::utils;
namespace vm = vle::manager;
namespace vz = vle::vpz;
namespace vv = vle::value;
namespace vo = vle::oov;
namespace vd = vle::devs;

/**
 * @brief Class that provides static methods for
 * handling vle API.
 */
class VleAPIfacilities
{
public:

    /**
     * @brief Build a map object gathering a list of conditions
     * @param vpz, the vpz containing the conditions
     * @param conditions, the list of conditions to use for building
     * the map
     * @return the map built object
     */
    static vv::Map* buildInitEventList(const vz::Vpz& vpz,
        const std::vector<std::string>& conditions)
    {
        vv::Map* initValues = new vv::Map();
        if (not conditions.empty()) {
            for (std::vector<std::string>::const_iterator it =
                    conditions.begin(); it != conditions.end(); ++it) {
                const vz::Condition& cnd(
                    vpz.project().experiment().conditions().get(*it));
                vv::MapValue vl;
                cnd.fillWithFirstValues(vl);
                for (vv::MapValue::const_iterator itv = vl.begin();
                        itv != vl.end(); ++itv) {
                    if (initValues->exist(itv->first)) {
                        throw vu::InternalError(vle::fmt(_(
                                "Multiples condition with the same init port "
                                "name '%1%'")) % itv->first);
                    }
                    initValues->add(itv->first,itv->second->clone());
                }
            }
        }
        return initValues;
    }

    /**
     * @brief Attach a set of conditions to an atomic model
     * @param vpz, the model to modify
     * @param atom_model_path, a path to find the atomic model
     * (e.g "coupled1:coupled2:atomic")
     * @param condNames, names of conditions to set for the atomic model
     */
    static void attachConditions(vz::Vpz& vpz,
        const std::string& atom_model_path,
        const std::vector<std::string>& condNames)
    {
        vz::Model& vpz_mod = vpz.project().model();
        vz::BaseModel* mdl =
                vpz_mod.model()->findModelFromPath(atom_model_path);
        if (mdl == 0) {
            throw vu::InternalError(" model not found ");
        }
        vz::AtomicModel* atom = vz::BaseModel::toAtomic(mdl);
        atom->setConditions(condNames);
    }
    /**
      * @brief Attach a set of conditions to an atomic model
      * @param vpz, the model to modify
      * @param atom_model_path, a path to find the atomic model
      * (e.g "coupled1:coupled2:atomic")
      * @param dynamic, the name of the dynamic to attach
      */
     static void setDynamics(vz::Vpz& vpz,
         const std::string& atom_model_path,
         const std::string& dynamic)
     {
         vz::Model& vpz_mod = vpz.project().model();
         vz::BaseModel* mdl =
                 vpz_mod.model()->findModelFromPath(atom_model_path);
         if (mdl == 0) {
             throw vu::InternalError(" model not found ");
         }
         vz::AtomicModel* atom = vz::BaseModel::toAtomic(mdl);
         atom->setDynamics(dynamic);
     }

    /**
     * @brief Change the output plugin for a view
     * @param vpz, the model to modify
     * @param view, the name of the view
     * @param plugin, the name of the new plugin
     */
    static void changePlugin(vz::Vpz& vpz, const std::string& view,
        const std::string& plugin)
    {
        vz::Views& vle_views = vpz.project().experiment().views();
        vz::Output& out = vle_views.outputs().get(view);
        out.setLocalStream("",plugin,"vle.output");

        //configure output plugins for column names
        if(plugin == "storage"){
            vv::Map* configOutput = new vv::Map();
            configOutput->addString("header","top");
            out.setData(configOutput);

        }

    }

    /**
     * @brief change the view type (between FINISH and TIMED)
     * @param vpz, the model to modify
     * @param view, the name of the view
     * @param viewType, the type of the view
     */
    static void changeViewType(vz::Vpz& vpz, const std::string& view,
            const std::string& viewType)
    {
        vz::Views& vle_views = vpz.project().experiment().views();
        if (viewType == "FINISH") {
            vle_views.get(view).setType(vz::View::FINISH);
        } else if (viewType == "TIMED") {
            vle_views.get(view).setType(vz::View::TIMED);
        }
    }


    /**
     * @brief Change the output plugin of all views
     * @param vpz, the model to modify
     * @param plugin, the name of the new plugin
     */
    static void changeAllPlugin(vz::Vpz& vpz, const std::string& plugin)
    {
        vz::Views& vle_views = vpz.project().experiment().views();
        vz::ViewList& views = vle_views.viewlist();
        vz::ViewList::iterator itb = views.begin();
        vz::ViewList::iterator ite = views.end();
        for (; itb != ite; itb++) {
            const std::string& v = itb->first;
            vz::Output& out = vle_views.outputs().get(v);
            out.setLocalStream("",plugin,"vle.output");
            //configure output plugins for column names
            if(plugin == "storage"){
                vv::Map* configOutput = new vv::Map();
                configOutput->addString("header","top");
                out.setData(configOutput);
            }
        }
    }

    /**
     * @brief Get the output plugin of a view
     * @param vpz, the model to modify
     * @param view, the name of the view
     * @return the name of the plugin
     */
    static const std::string& getPlugin(const vz::Vpz& vpz, const std::string& view)
    {
        const vz::Views& vle_views = vpz.project().experiment().views();
        const vz::Output& out = vle_views.outputs().get(view);
        return out.plugin();
    }


    /**
     * @brief Comparison operator between vle values
     * @param a the first value
     * @param b the second value
     * @return true if a == b
     */
    static bool equal(const vv::Value& a, const vv::Value& b)
    {
        if(a.getType() != b.getType()) return false;
        switch(a.getType()){
        case vv::Value::BOOLEAN :{
            return a.toBoolean().value() == b.toBoolean().value();
            break;
        } case vv::Value::DOUBLE :{
            return a.toDouble().value() == b.toDouble().value();
            break;
        } case vv::Value::STRING :{
            return a.toString().value() == b.toString().value();
            break;
        } case vv::Value::INTEGER :{
            return a.toInteger().value() == b.toInteger().value();
            break;
        } case vv::Value::SET :{
            const vv::Set& as = a.toSet();
            const vv::Set& bs = b.toSet();
            if(as.size() != bs.size()) return false;
            vv::Set::const_iterator itab = as.begin();
            vv::Set::const_iterator itae = as.end();
            vv::Set::const_iterator itbb = bs.begin();
            for(;itab!=itae; ){
                if(!VleAPIfacilities::equal((*(*itab)),(*(*itbb))))
                    return false;
                itab++;
                itbb++;
            }
            return true;
            break;
        } case vv::Value::MAP :{
            const vv::Map& as = a.toMap();
            const vv::Map& bs = b.toMap();
            if(as.value().size() != bs.value().size()) return false;
            vv::Map::const_iterator itab = as.begin();
            vv::Map::const_iterator itae = as.end();
            vv::Map::const_iterator itbb = bs.begin();
            for(;itab!=itae;){
                if(!(itab->first == itbb->first)) return false;
                if(!VleAPIfacilities::equal(*(itab->second), *(itbb->second)))
                    return false;
                itab++;
                itbb++;
            }
            return true;
            break;
        } case vv::Value::TUPLE :{
            const vv::Tuple& as = a.toTuple();
            const vv::Tuple& bs = b.toTuple();
            if(as.value().size() != bs.value().size()) return false;
            for(unsigned int i=0; i < as.size(); i++){
                if(as.at(i) != bs.at(i)) return false;
            }
            return true;
            break;
        } default :{
            throw vu::InternalError();
        }
        }
    }

    /**
     * @brief Get the last value of an output vector
     * @param result, the output matrix list
     * @param view, the name of the view containing the value
     * @param model_path, the path of the atomic model observed
     * (e.g "coupled1:coupled2:atomic")
     * @param port, the name of the port observed
     * @return the simulated value
     */
    static const vv::Value* findLastOutputValue(
            const vv::Map& result, const std::string& view,
            const std::string& model_path, const std::string& port)
    {
        vv::Map::const_iterator it = result.find(view);
        if (it == result.end()) {
            throw vu::ArgError(vle::fmt("view '%1%' not found)") % view);
        }
        const vv::Matrix& outMat = vv::toMatrixValue(*it->second);
        std::string absoluteModPath(model_path);
        absoluteModPath.append(".");
        absoluteModPath.append(port);
        unsigned int colIndex = 999;
        for (unsigned int i=0; i < outMat.columns(); i++) {
            if (outMat.getString(i,0) == absoluteModPath) {
                colIndex = i;
            }
        }
        if (colIndex == 999) {
            throw vu::ArgError(vle::fmt("view.port '%1%' not found)")
                 % absoluteModPath);
        }
        const vv::ConstVectorView& outVec = outMat.column(colIndex);
        return (outVec[outVec.size() - 1]);
    }


    /**
     * @brief Get the last value of an output vector
     * @param result, the OutputMatrixViewList to examine
     * @param view, the name of the view containing the value
     * @param port, the name of the port observed
     * @return the simulated value, note that only one port
     * with name 'port' should be present in the view
     */
    static const vv::Value* findLastOutputValue(
        const vv::Map& result, const std::string& view,
        const std::string& port)
    {
        vv::Map::const_iterator it = result.find(view);
        if (it == result.end()) {
            throw vu::ArgError(vle::fmt(
                "view '%1%' not found)") % view);
        }
        const vv::Matrix& outMat = vv::toMatrixValue(*it->second);
        bool found = false;
        vv::Value* res = 0;
        for (unsigned int i=0; i < outMat.columns(); i++) {
            if (boost::algorithm::ends_with(outMat.getString(i,0), port)) {
                if(found){
                    throw vu::ArgError(vle::fmt(
                            "[VleAPIFacilities]  Ambiguous port '%1%' "
                            " in view '%2%' (found more than once)")
                            % port % view);
                }
                found = true;
                const vv::ConstVectorView& outVec = outMat.column(i);
                res = outVec[outVec.size() - 1];
            }
        }
        if(not found){
            throw vu::ArgError(vle::fmt(
               "[VleAPIFacilities] port '%1%' not found in view '%2%' ")
            % port % view);
        }
        return res;
    }

};

}
}//namespaces

#endif
