#ifndef LARUTILMANAGER_CXX
#define LARUTILMANAGER_CXX

#include "LArUtilManager.h"

namespace larutil {

bool LArUtilManager::Reconfigure(galleryfmwk::geo::DetId_t type)
{

  if (type == LArUtilConfig::Detector()) return true;

  bool status = LArUtilConfig::SetDetector(type);

  if (!status) return status;

  return ReconfigureUtilities();

}

bool LArUtilManager::ReconfigureUtilities()
{
  bool status = true;

  // Geometry
  galleryfmwk::Message::send(galleryfmwk::msg::kNORMAL, __FUNCTION__, "Reconfiguring Geometry");
  Geometry* geom = (Geometry*)(Geometry::GetME(false));
  geom->SetFileName(Form("%s/LArUtil/dat/%s",
                         getenv("GALLERY_FMWK_COREDIR"),
                         kUTIL_DATA_FILENAME[LArUtilConfig::Detector()].c_str()));
  geom->SetTreeName(kTREENAME_GEOMETRY);
  status = status && geom->LoadData(true);
  // geom->SetFileName(Form("%s/LArUtil/dat/%s",
  //                        getenv("GALLERY_FMWK_COREDIR"),
  //                        kUTIL_FCL_FILENAME[LArUtilConfig::Detector()].c_str()));
  // status = status && geom->LoadDataFromServices(true);

  // LArProperties
  galleryfmwk::Message::send(galleryfmwk::msg::kNORMAL, __FUNCTION__, "Reconfiguring LArProperties");
  LArProperties* larp = (LArProperties*)(LArProperties::GetME(false));
  larp->SetFileName(Form("%s/LArUtil/dat/%s",
                         getenv("GALLERY_FMWK_COREDIR"),
                         kUTIL_DATA_FILENAME[LArUtilConfig::Detector()].c_str()));
  larp->SetTreeName(kTREENAME_LARPROPERTIES);
  status = status && larp->LoadData(true);
  // larp->SetFileName(Form("%s/LArUtil/dat/%s",
  //                        getenv("GALLERY_FMWK_COREDIR"),
  //                        kUTIL_FCL_FILENAME[LArUtilConfig::Detector()].c_str()));
  // status = status && larp->LoadDataFromServices(true);

  // DetectorProperties
  galleryfmwk::Message::send(galleryfmwk::msg::kNORMAL, __FUNCTION__, "Reconfiguring DetectorProperties");
  DetectorProperties* detp = (DetectorProperties*)(DetectorProperties::GetME(false));
  detp->SetFileName(Form("%s/LArUtil/dat/%s",
                         getenv("GALLERY_FMWK_COREDIR"),
                         kUTIL_DATA_FILENAME[LArUtilConfig::Detector()].c_str()));
  detp->SetTreeName(kTREENAME_DETECTORPROPERTIES);
  status = status && detp->LoadData(true);
  // detp->SetFileName(Form("%s/LArUtil/dat/%s",
  //                        getenv("GALLERY_FMWK_COREDIR"),
  //                        kUTIL_FCL_FILENAME[LArUtilConfig::Detector()].c_str()));
  // status = status && detp->LoadDataFromServices(true);

  if (status) {

    // GeometryHelper
    galleryfmwk::Message::send(galleryfmwk::msg::kNORMAL, __FUNCTION__, "Reconfiguring GeometryHelper...");
    GeometryHelper* gser = (GeometryHelper*)(GeometryHelper::GetME());
    gser->Reconfigure();

    // GeometryHelper
    galleryfmwk::Message::send(galleryfmwk::msg::kNORMAL, __FUNCTION__, "Reconfiguring GeometryHelper...");
    GeometryHelper* ghelp = (GeometryHelper*)(GeometryHelper::GetME());
    ghelp->Reconfigure();

  }

  return status;
}

}

#endif
