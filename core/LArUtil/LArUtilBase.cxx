#ifndef GALLERY_FMWK_LARUTILBASE_CXX
#define GALLERY_FMWK_LARUTILBASE_CXX

#include "LArUtilBase.h"

namespace larutil {

//-----------------------------------------------------
LArUtilBase::LArUtilBase()
  : _file_name(""),
    _tree_name("")
    //-----------------------------------------------------
{
  _name   = "LArUtilBase";
  _loaded = false;
}

//-----------------------------------------------------
bool LArUtilBase::LoadData(bool force_reload)
//-----------------------------------------------------
{
  if (!force_reload && _loaded) return true;

  galleryfmwk::Message::send(galleryfmwk::msg::kINFO, __FUNCTION__,
        Form("Reading-in data for %s", _name.c_str()));

  if (_file_name.empty() || _tree_name.empty()) {

    throw LArUtilException(Form("Either file or tree name not specified to load data for %s", _name.c_str()));

    return false;
  }

  bool status = false;
  try {

    galleryfmwk::Message::send(galleryfmwk::msg::kNORMAL, __FUNCTION__,
          Form("Loading data for %s...\n     file=%s ", _name.c_str(), _file_name.c_str()));
    ClearData();
    status = ReadTree();
    if (!status)
      throw LArUtilException("Failed to load from ROOT input TTree!");
  }
  catch (LArUtilException &e) {

    galleryfmwk::Message::send(galleryfmwk::msg::kERROR, __FUNCTION__,
          Form("Failed loading data for %s...", _name.c_str()));

    galleryfmwk::Message::send(galleryfmwk::msg::kERROR, __FUNCTION__,
          e.what());

    throw e;
    status = false;
  }

  return status;
}

// //-----------------------------------------------------
// bool LArUtilBase::LoadDataFromServices(bool force_reload)
// //-----------------------------------------------------
// {
//   if (!force_reload && _loaded) return true;

//   galleryfmwk::Message::send(galleryfmwk::msg::kINFO, __FUNCTION__,
//         Form("Reading-in data for %s", _name.c_str()));

//   if (_file_name.empty()) {

//     throw LArUtilException(Form("Fcl file name not specified to load data for %s", _name.c_str()));

//     return false;
//   }

//   bool status = false;
//   try {

//     galleryfmwk::Message::send(galleryfmwk::msg::kNORMAL, __FUNCTION__,
//           Form("Loading data for %s...\n     using fcl file=%s ", _name.c_str(), _file_name.c_str()));
//     ClearData();
//     status = ReadFromServices();
//     // status = true;
//     if (!status)
//       throw LArUtilException("Failed to load from services!");
//   }
//   catch (LArUtilException &e) {

//     galleryfmwk::Message::send(galleryfmwk::msg::kERROR, __FUNCTION__,
//           Form("Failed loading data for %s...", _name.c_str()));

//     galleryfmwk::Message::send(galleryfmwk::msg::kERROR, __FUNCTION__,
//           e.what());

//     throw e;
//     status = false;
//   }

//   return status;
// }

}
#endif
