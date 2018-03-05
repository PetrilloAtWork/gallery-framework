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
bool LArUtilBase::LoadData(bool force_reload, int i_tpc, int i_cryostat)
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
    status = ReadTree(i_tpc, i_cryostat);
    if (!status)
      throw LArUtilException("Failed to load !");
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

}
#endif
