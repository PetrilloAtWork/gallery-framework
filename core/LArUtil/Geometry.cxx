#ifndef GALLERY_FMWK_GEOMETRY_CXX
#define GALLERY_FMWK_GEOMETRY_CXX

#include "Geometry.h"
#include "InvalidWireError.h"
namespace larutil {

Geometry* Geometry::_me = 0;

void Geometry::CryostatBoundaries(Double_t* boundaries) const
{
  if ( fCryostatBoundaries.size() != 6 )
    throw LArUtilException("CryostatBoundaries not loaded (length != 6)... ");

  //if( fCryostatBoundaries.size() != sizeof(boundaries)/8 )
  // throw LArUtilException("Input argument for CryostatBoundaries must be length 6 double array!");

  for (size_t i = 0; i < fCryostatBoundaries.size(); ++i)
    boundaries[i] = fCryostatBoundaries[i];
}

Geometry::Geometry(bool default_load) : LArUtilBase()
{
  if (default_load) {
    _file_name = Form("%s/LArUtil/dat/%s",
                      getenv("GALLERY_FMWK_COREDIR"),
                      kUTIL_DATA_FILENAME[LArUtilConfig::Detector()].c_str());
    _tree_name = kTREENAME_GEOMETRY;
    LoadData();
    DumpInfo();

  }

  // if(default_load) {
  //     std::cout << "**************************" << std::endl;
  //     _file_name = Form("%s/LArUtil/dat/%s",
  //     getenv("GALLERY_FMWK_COREDIR"),
  //     kUTIL_FCL_FILENAME[LArUtilConfig::Detector()].c_str());
  //     _fcl_name = kTREENAME_DETECTORPROPERTIES;
  //     LoadDataFromServices();
  //   }
}

bool Geometry::LoadData(bool force_reload)
{
  bool status = LArUtilBase::LoadData(force_reload);

  if (!status) return status;

  fOrthVectorsY.resize(this->Nplanes());
  fOrthVectorsZ.resize(this->Nplanes());
  fFirstWireProj.resize(this->Nplanes());
  for (size_t plane = 0; plane < this->Nplanes(); ++plane) {

    galleryfmwk::geo::View_t view = this->PlaneToView(plane);

    Double_t ThisWirePitch = this->WirePitch(view);

    Double_t WireCentre1[3] = {0.};
    Double_t WireCentre2[3] = {0.};

    Double_t  th = this->WireAngleToVertical(view);
    Double_t sth = TMath::Sin(th);
    Double_t cth = TMath::Cos(th);

    for (size_t coord = 0; coord < 3; ++coord) {
      WireCentre1[coord] = (fWireEndVtx.at(plane).at(0).at(coord) + fWireStartVtx.at(plane).at(0).at(coord)) / 2.;
      WireCentre2[coord] = (fWireEndVtx.at(plane).at(1).at(coord) + fWireStartVtx.at(plane).at(1).at(coord)) / 2.;
    }

    Double_t OrthY =  cth;
    Double_t OrthZ = -sth;
    if (((WireCentre2[1] - WireCentre1[1])*OrthY
         + (WireCentre2[2] - WireCentre1[2])*OrthZ) < 0) {
      OrthZ *= -1;
      OrthY *= -1;
    }

    fOrthVectorsY[plane] = OrthY / ThisWirePitch;
    fOrthVectorsZ[plane] = OrthZ / ThisWirePitch;

    fFirstWireProj[plane]  = WireCentre1[1] * OrthY + WireCentre1[2] * OrthZ;
    fFirstWireProj[plane] /= ThisWirePitch;
    fFirstWireProj[plane] -= 0.5;

  }
  return status;
}

void Geometry::ClearData()
{
  fDetLength = galleryfmwk::data::kINVALID_DOUBLE;
  fDetHalfWidth = galleryfmwk::data::kINVALID_DOUBLE;
  fDetHalfHeight = galleryfmwk::data::kINVALID_DOUBLE;

  fCryoLength = galleryfmwk::data::kINVALID_DOUBLE;
  fCryoHalfWidth = galleryfmwk::data::kINVALID_DOUBLE;
  fCryoHalfHeight = galleryfmwk::data::kINVALID_DOUBLE;

  fCryostatBoundaries.clear();

  fNTPC = 1;
  fChannelToPlaneMap.clear();
  fChannelToWireMap.clear();
  fChannelToTPCMap.clear(); 
  fPlaneWireToChannelMap.clear();
  fTPCPlaneWireToChannelMap.clear();
  fSignalType.clear();
  fViewType.clear();
  fPlanePitch.clear();
  //fFirstWireStartVtx.clear();
  //fFirstWireEndVtx.clear();
  fWireStartVtx.clear();
  fWireEndVtx.clear();
  fWirePitch.clear();
  fWireAngle.clear();
  fOpChannelVtx.clear();
  fOpChannel2OpDet.clear();
  fOpDetVtx.clear();
  fPlaneOriginVtx.clear();
}

// bool Geometry::ReadFromServices()
// {
//     ClearData();
//     auto geo = LArUtilServicesHandler::GetGeometry(_file_name);

//     fNTPC = geo->NTPC();

//     fDetLength = geo->DetLength();
//     fDetHalfWidth = geo->DetHalfWidth();
//     fDetHalfHeight = geo->DetHalfHeight();

//     fCryoLength = -9999;
//     fCryoHalfWidth = -9999;
//     fCryoHalfHeight = -9999;



//     fPlaneWireToChannelMap.resize(geo->Nplanes(0));
//     for (size_t i = 0; i < fPlaneWireToChannelMap.size(); i++) {
//       fPlaneWireToChannelMap.at(i).resize(geo->Nwires(i));
//     }



//     fTPCPlaneWireToChannelMap.resize(geo->NTPC());
//     for (size_t i = 0; i < fTPCPlaneWireToChannelMap.size(); i++) {
//       fTPCPlaneWireToChannelMap.at(i).resize(geo->Nplanes(i));
//       for (size_t j = 0; j < fTPCPlaneWireToChannelMap.at(i).size(); j++) {
//         fTPCPlaneWireToChannelMap.at(i).at(j).resize(geo->Nwires(j, i));
//       }
//     }

//     fWireStartVtx.resize(geo->Nplanes(0) * geo->NTPC());
//     fWireEndVtx.resize(geo->Nplanes(0) * geo->NTPC());

//     for (size_t i = 0; i < fWireStartVtx.size(); i++) {
//       fWireStartVtx.at(i).resize(geo->Nwires(i % (geo->NTPC()+1), int(i/geo->Nplanes(0))));
//       fWireEndVtx.at(i).resize(geo->Nwires(i % (geo->NTPC()+1), int(i/geo->Nplanes(0))));
//     }

//     for (size_t i = 1; i < 11277; i++) {
//       // std::cout << "Channel " << i << " -> n wires " << geo->ChannelToWire(i)[0] << std::endl;
//       auto wire = geo->ChannelToWire(i)[0].Wire;
//       auto plane = geo->ChannelToWire(i)[0].Plane;
//       auto tpc = geo->ChannelToWire(i)[0].TPC;

//       fChannelToWireMap.push_back(wire);
//       fChannelToPlaneMap.push_back(plane);
//       fChannelToTPCMap.push_back(tpc);

//       fPlaneWireToChannelMap.at(plane).at(wire) = i;
//       fTPCPlaneWireToChannelMap.at(tpc).at(plane).at(wire) = i;

//       double xyz_start[3] = {0., 0., 0.};
//       geo->WireIDToWireGeo(geo->ChannelToWire(i)[0]).GetStart(xyz_start);
//       double xyz_end[3] = {0., 0., 0.};
//       geo->WireIDToWireGeo(geo->ChannelToWire(i)[0]).GetEnd(xyz_end);

//       fWireStartVtx.at(plane + tpc * geo->Nplanes(tpc)).at(wire).push_back(xyz_start[0]);
//       fWireStartVtx.at(plane + tpc * geo->Nplanes(tpc)).at(wire).push_back(xyz_start[1]);
//       fWireStartVtx.at(plane + tpc * geo->Nplanes(tpc)).at(wire).push_back(xyz_start[2]);
//       fWireEndVtx.at(plane + tpc * geo->Nplanes(tpc)).at(wire).push_back(xyz_end[0]);
//       fWireEndVtx.at(plane + tpc * geo->Nplanes(tpc)).at(wire).push_back(xyz_end[1]);
//       fWireEndVtx.at(plane + tpc * geo->Nplanes(tpc)).at(wire).push_back(xyz_end[2]);
//     }




//     for (size_t tpc = 0; tpc < 1/*tpc < geo->NTPC()*/; tpc++) {
//       for (size_t plane = 0; plane < geo->Nplanes(tpc); plane++) {

//         // auto plane_id = geo::PlaneID(tpc, plane);
//         fSignalType.push_back((galleryfmwk::geo::SigType_t) geo->SignalType(plane));
//         fViewType.push_back((galleryfmwk::geo::View_t) geo->View(plane));
//         fPlanePitch.push_back(geo->PlanePitch(plane, plane+1));

//         fWirePitch.push_back(geo->WirePitch(plane, tpc));
//         fWireAngle.push_back(geo->WireAngleToVertical(geo->View(plane), tpc));
//       }
//   }


//   //*** taken from above
//   fOrthVectorsY.resize(this->Nplanes());
//   fOrthVectorsZ.resize(this->Nplanes());
//   fFirstWireProj.resize(this->Nplanes());
//   for (size_t plane = 0; plane < this->Nplanes(); ++plane) {

//     galleryfmwk::geo::View_t view = this->PlaneToView(plane);

//     Double_t ThisWirePitch = this->WirePitch(view);

//     Double_t WireCentre1[3] = {0.};
//     Double_t WireCentre2[3] = {0.};

//     Double_t  th = this->WireAngleToVertical(view);
//     Double_t sth = TMath::Sin(th);
//     Double_t cth = TMath::Cos(th);

//     for (size_t coord = 0; coord < 3; ++coord) {
//       WireCentre1[coord] = (fWireEndVtx.at(plane).at(0).at(coord) + fWireStartVtx.at(plane).at(0).at(coord)) / 2.;
//       WireCentre2[coord] = (fWireEndVtx.at(plane).at(1).at(coord) + fWireStartVtx.at(plane).at(1).at(coord)) / 2.;
//     }

//     Double_t OrthY =  cth;
//     Double_t OrthZ = -sth;
//     if (((WireCentre2[1] - WireCentre1[1])*OrthY
//          + (WireCentre2[2] - WireCentre1[2])*OrthZ) < 0) {
//       OrthZ *= -1;
//       OrthY *= -1;
//     }

//     fOrthVectorsY[plane] = OrthY / ThisWirePitch;
//     fOrthVectorsZ[plane] = OrthZ / ThisWirePitch;

//     fFirstWireProj[plane]  = WireCentre1[1] * OrthY + WireCentre1[2] * OrthZ;
//     fFirstWireProj[plane] /= ThisWirePitch;
//     fFirstWireProj[plane] -= 0.5;

//   }
//   //*** ends

//   return true;
// }



bool Geometry::ReadTree()
{
  ClearData();

  TChain *ch = new TChain(_tree_name.c_str());
  ch->AddFile(_file_name.c_str());

  std::string error_msg("");
  if (!(ch->GetBranch("fDetLength")))      error_msg += "      fDetLength\n";
  if (!(ch->GetBranch("fDetHalfWidth")))   error_msg += "      fDetHalfWidth\n";
  if (!(ch->GetBranch("fDetHalfHeight")))  error_msg += "      fDetHalfHeight\n";

  if (!(ch->GetBranch("fCryoLength")))     error_msg += "      fCryoLength\n";
  if (!(ch->GetBranch("fCryoHalfWidth")))  error_msg += "      fCryoHalfWidth\n";
  if (!(ch->GetBranch("fCryoHalfHeight"))) error_msg += "      fCryoHalfHeight\n";

  if (LArUtilConfig::Detector() != galleryfmwk::geo::kArgoNeuT) {
    if (!(ch->GetBranch("fCryostatBoundaries"))) error_msg += "       fCryostatBoundaries\n";
  }
  if (!(ch->GetBranch("fChannelToPlaneMap")))           error_msg += "      fChannelToPlaneMap\n";
  if (!(ch->GetBranch("fChannelToWireMap")))            error_msg += "      fChannelToWireMap\n";
  if (!(ch->GetBranch("fPlaneWireToChannelMap")))       error_msg += "      fPlaneWireToChannelMap\n";
  if (LArUtilConfig::Detector() == galleryfmwk::geo::kSBND) {
    if (!(ch->GetBranch("fChannelToTPCMap")))           error_msg += "      fChannelToTPCMap\n";
    if (!(ch->GetBranch("fTPCPlaneWireToChannelMap")))  error_msg += "      fTPCPlaneWireToChannelMap\n";
  }

  if (!(ch->GetBranch("fSignalType"))) error_msg += "      fSignalType\n";
  if (!(ch->GetBranch("fViewType")))   error_msg += "      fViewType\n";
  if (!(ch->GetBranch("fPlanePitch"))) error_msg += "      fPlanePitch\n";

  //if(!(ch->GetBranch("fFirstWireStartVtx"))) error_msg += "      fFirstWireStartVtx\n";
  //if(!(ch->GetBranch("fFirstWireEndVtx")))   error_msg += "      fFirstWireEndVtx\n";

  if (!(ch->GetBranch("fWireStartVtx"))) error_msg += "      fWireStartVtx\n";
  if (!(ch->GetBranch("fWireEndVtx")))   error_msg += "      fWireEndVtx\n";

  if (!(ch->GetBranch("fWirePitch")))       error_msg += "      fWirePitch\n";
  if (!(ch->GetBranch("fWireAngle")))       error_msg += "      fWireAngle\n";
  if (LArUtilConfig::Detector() != galleryfmwk::geo::kArgoNeuT) {
    if (!(ch->GetBranch("fOpChannelVtx")))    error_msg += "      fOpChannelVtx\n";
    if (!(ch->GetBranch("fOpChannel2OpDet"))) error_msg += "      fOpChannel2OpDet\n";
    if (!(ch->GetBranch("fOpDetVtx")))        error_msg += "      fOpDetVtx\n";
  }

  if (!(ch->GetBranch("fPlaneOriginVtx"))) error_msg += "      fPlaneOriginVtx\n";

  if (!error_msg.empty()) {

    throw LArUtilException(Form("Missing following TBranches...\n%s", error_msg.c_str()));

    return false;
  }

  // Cryo boundaries
  std::vector<Double_t> *pCryostatBoundaries = nullptr;

  // Vectors with length = # channels
  std::vector<UChar_t>                *pChannelToPlaneMap = nullptr;
  std::vector<UShort_t>               *pChannelToWireMap = nullptr;
  std::vector<UShort_t>               *pChannelToTPCMap = nullptr; // sbnd only

  // Vectors with length = # planes
  std::vector<std::vector<UShort_t> >   *pPlaneWireToChannelMap = nullptr;
  std::vector<std::vector<std::vector<UShort_t>> >   *pTPCPlaneWireToChannelMap = nullptr; // sbnd only
  std::vector<galleryfmwk::geo::SigType_t> *pSignalType = nullptr;
  std::vector<galleryfmwk::geo::View_t>    *pViewType = nullptr;
  std::vector<Double_t>                 *pPlanePitch = nullptr;
  //std::vector<std::vector<Double_t> >   *pFirstWireStartVtx = nullptr;
  //std::vector<std::vector<Double_t> >   *pFirstWireEndVtx = nullptr;
  std::vector<std::vector<std::vector<Double_t> > >  *pWireStartVtx = nullptr;
  std::vector<std::vector<std::vector<Double_t> > >  *pWireEndVtx = nullptr;
  std::vector<std::vector<Double_t> >  *pPlaneOriginVtx = nullptr;

  // Vectors with length = view
  std::vector<Double_t> *pWirePitch = nullptr;
  std::vector<Double_t> *pWireAngle = nullptr;

  std::vector<std::vector<Float_t> > *pOpChannelVtx = nullptr;
  std::vector<std::vector<Float_t> > *pOpDetVtx = nullptr;
  std::vector<unsigned int> *pOpChannel2OpDet = nullptr;
  ch->SetBranchAddress("fDetLength", &fDetLength);
  ch->SetBranchAddress("fDetHalfWidth", &fDetHalfWidth);
  ch->SetBranchAddress("fDetHalfHeight", &fDetHalfHeight);

  ch->SetBranchAddress("fCryoLength", &fCryoLength);
  ch->SetBranchAddress("fCryoHalfWidth", &fCryoHalfWidth);
  ch->SetBranchAddress("fCryoHalfHeight", &fCryoHalfHeight);

  if (LArUtilConfig::Detector() != galleryfmwk::geo::kArgoNeuT) {
    ch->SetBranchAddress("fCryostatBoundaries", &pCryostatBoundaries);
  }

  ch->SetBranchAddress("fChannelToWireMap",  &pChannelToWireMap);
  ch->SetBranchAddress("fChannelToPlaneMap", &pChannelToPlaneMap);
  ch->SetBranchAddress("fPlaneWireToChannelMap", &pPlaneWireToChannelMap);

  if (LArUtilConfig::Detector() == galleryfmwk::geo::kSBND) {
    ch->SetBranchAddress("fChannelToTPCMap",          &pChannelToTPCMap);
    ch->SetBranchAddress("fTPCPlaneWireToChannelMap", &pTPCPlaneWireToChannelMap);
  }

  ch->SetBranchAddress("fSignalType", &pSignalType);
  ch->SetBranchAddress("fViewType", &pViewType);
  ch->SetBranchAddress("fPlanePitch", &pPlanePitch);

  //ch->SetBranchAddress("fFirstWireStartVtx",&pFirstWireStartVtx);
  //ch->SetBranchAddress("fFirstWireEndVtx",&pFirstWireEndVtx);
  ch->SetBranchAddress("fWireStartVtx", &pWireStartVtx);
  ch->SetBranchAddress("fWireEndVtx", &pWireEndVtx);
  ch->SetBranchAddress("fPlaneOriginVtx", &pPlaneOriginVtx);

  ch->SetBranchAddress("fWirePitch", &pWirePitch);
  ch->SetBranchAddress("fWireAngle", &pWireAngle);

  if (LArUtilConfig::Detector() != galleryfmwk::geo::kArgoNeuT) {
    ch->SetBranchAddress("fOpChannelVtx", &pOpChannelVtx);
    ch->SetBranchAddress("fOpDetVtx", &pOpDetVtx);
    ch->SetBranchAddress("fOpChannel2OpDet", &pOpChannel2OpDet);
  }
  ch->GetEntry(0);


  if (LArUtilConfig::Detector() != galleryfmwk::geo::kArgoNeuT) {
    fCryostatBoundaries.resize(pCryostatBoundaries->size());

    for (size_t i = 0; i < pCryostatBoundaries->size(); ++i)
      fCryostatBoundaries[i] = (*pCryostatBoundaries)[i];
  }

  // Copy TPC-wise variabels
  if (LArUtilConfig::Detector() == galleryfmwk::geo::kSBND) {
    size_t n_tpcs = pTPCPlaneWireToChannelMap->size();
    std::cout <<"n_tpcs " << n_tpcs << std::endl;
    fTPCPlaneWireToChannelMap.reserve(n_tpcs);
    for (size_t i = 0; i < n_tpcs; ++i) {
      fTPCPlaneWireToChannelMap.push_back(pTPCPlaneWireToChannelMap->at(i));
    }
    fNTPC = n_tpcs;
  }

  // Copy channel-wise variables
  size_t n_channels = pChannelToPlaneMap->size();
  std::cout <<"n_channels " << n_channels << std::endl;
  fChannelToPlaneMap.reserve(n_channels);
  fChannelToWireMap.reserve(n_channels);
  fChannelToTPCMap.reserve(n_channels);
  for (size_t i = 0; i < n_channels; ++i) {
    fChannelToPlaneMap.push_back(pChannelToPlaneMap->at(i));
    fChannelToWireMap.push_back(pChannelToWireMap->at(i));
    if (LArUtilConfig::Detector() == galleryfmwk::geo::kSBND){
      fChannelToTPCMap.push_back(pChannelToTPCMap->at(i));
    }
  }

  // Copy plane-wise variables
  size_t n_planes = pPlaneWireToChannelMap->size();
  std::cout <<"n_planes " << n_planes << std::endl;

  fPlaneWireToChannelMap.reserve(n_planes);
  fSignalType.reserve(n_planes);
  fViewType.reserve(n_planes);
  fPlanePitch.reserve(n_planes);
  //fFirstWireStartVtx.reserve(n_planes);
  //fFirstWireEndVtx.reserve(n_planes);
  fWireStartVtx.reserve(n_planes);
  fWireEndVtx.reserve(n_planes);
  fPlaneOriginVtx.reserve(n_planes);
  for (size_t i = 0; i < n_planes; ++i) {
    fPlaneWireToChannelMap.push_back(pPlaneWireToChannelMap->at(i));
    fSignalType.push_back(pSignalType->at(i));
    fViewType.push_back(pViewType->at(i));
    fPlanePitch.push_back(pPlanePitch->at(i));
    //fFirstWireStartVtx.push_back(pFirstWireStartVtx->at(i));
    //fFirstWireEndVtx.push_back(pFirstWireEndVtx->at(i));
    fWireStartVtx.push_back(pWireStartVtx->at(i));
    fWireEndVtx.push_back(pWireEndVtx->at(i));

    fPlaneOriginVtx.push_back(pPlaneOriginVtx->at(i));
  }


  // Copy view-wise variables
  size_t n_views = pWirePitch->size();
  fWirePitch.reserve(n_views);
  fWireAngle.reserve(n_views);
  for (size_t i = 0; i < n_views; ++i) {
    fWirePitch.push_back(pWirePitch->at(i));
    fWireAngle.push_back(pWireAngle->at(i));
  }

  if (LArUtilConfig::Detector() != galleryfmwk::geo::kArgoNeuT) {
    // Copy op-channel-wise variables
    size_t n_opchannel = pOpChannelVtx->size();
    fOpChannelVtx.reserve(n_opchannel);
    for (size_t i = 0; i < n_opchannel; ++i)
      fOpChannelVtx.push_back(pOpChannelVtx->at(i));

    size_t n_opdet = pOpDetVtx->size();
    fOpDetVtx.reserve(n_opdet);
    for (size_t i = 0; i < n_opdet; ++i)
      fOpDetVtx.push_back(pOpDetVtx->at(i));

    size_t n_opmap = pOpChannel2OpDet->size();
    fOpChannel2OpDet.reserve(n_opmap);
    for (size_t i = 0; i < n_opmap; ++i)
      fOpChannel2OpDet.push_back(pOpChannel2OpDet->at(i));

  }
  delete ch;
  return true;
}


void Geometry::DumpInfo()
{
  std::cout << std::endl;
  std::cout << "Dumping Geometry info:" << std::endl;
  std::cout << "\tfDetLength: " << fDetLength << std::endl;
  std::cout << "\tfDetHalfWidth: " << fDetHalfWidth << std::endl;
  std::cout << "\tfDetHalfHeight: " << fDetHalfHeight << std::endl;
  std::cout << std::endl;

  std::cout << "\tfCryoLength: " << fCryoLength << std::endl;
  std::cout << "\tfCryoHalfWidth: " << fCryoHalfWidth << std::endl;
  std::cout << "\tfCryoHalfHeight: " << fCryoHalfHeight << std::endl;
  std::cout << std::endl;

  std::cout << "\tfChannelToPlaneMap[0]: " << fChannelToPlaneMap[0] << std::endl;
  std::cout << "\tfChannelToWireMap[0]: " << fChannelToWireMap[0] << std::endl;
  std::cout << "\tfPlaneWireToChannelMap[0][0]: " << fPlaneWireToChannelMap[0][0] << std::endl;
  std::cout << std::endl;

  std::cout << "\tfSignalType[0]: " << fSignalType[0] << std::endl;
  std::cout << "\tfViewType[0]: " << fViewType[0] << std::endl;
  std::cout << "\tfPlanePitch[0]: " << fPlanePitch[0] << std::endl;
  std::cout << std::endl;

  std::cout << "\tfWireStartVtx[0] xyx: " << fWireStartVtx[0][0][0] << ", " << fWireStartVtx[0][0][1] << ", " << fWireStartVtx[0][0][2] << std::endl;
  std::cout << "\tfWireEndVtx[0] xyx: " << fWireEndVtx[0][0][0] << ", " << fWireEndVtx[0][0][1] << ", " << fWireEndVtx[0][0][2] << std::endl;
  std::cout << std::endl;

  std::cout << "\tfWirePitch[0]: " << fWirePitch[0] << std::endl;
  std::cout << "\tfWireAngle[0]: " << fWireAngle[0] << std::endl;
  std::cout << std::endl;

  std::cout << "\tfOpChannelVtx[0] xyx: " << fOpChannelVtx[0][0] << ", " << fOpChannelVtx[0][1] << ", " << fOpChannelVtx[0][2] << std::endl;
  std::cout << "\tfOpDetVtx[0] xyx: " << fOpDetVtx[0][0] << ", " << fOpDetVtx[0][1] << ", " << fOpDetVtx[0][2] << std::endl;
  std::cout << "\fOpChannel2OpDet[0]: " << fOpChannel2OpDet[0] << std::endl;
  std::cout << std::endl;

  std::cout << "\tfPlaneOriginVtx[0] xyx: " << fPlaneOriginVtx[0][0] << ", " << fPlaneOriginVtx[0][1] << ", " << fPlaneOriginVtx[0][2] << std::endl;
  std::cout << std::endl;

}




UInt_t Geometry::Nwires(UInt_t p) const
{
  if (Nplanes() <= p) {
    throw LArUtilException(Form("Invalid plane ID :%d", p));
    return galleryfmwk::data::kINVALID_UINT;
  }

  return fPlaneWireToChannelMap.at(p).size();
}

UChar_t  Geometry::ChannelToTPC(const UInt_t ch) const
{
  if (fChannelToTPCMap.size() == 0) {
    return 0;
  }
  if (ch >= fChannelToTPCMap.size()) {
    throw LArUtilException(Form("Invalid channel number: %d", ch));
    return galleryfmwk::data::kINVALID_CHAR;
  }
  return fChannelToTPCMap.at(ch);
}

UChar_t  Geometry::ChannelToPlane(const UInt_t ch) const
{
  if (ch >= fChannelToPlaneMap.size()) {
    throw LArUtilException(Form("Invalid channel number: %d", ch));
    return galleryfmwk::data::kINVALID_CHAR;
  }
  return fChannelToPlaneMap.at(ch);
}

UInt_t   Geometry::ChannelToWire(const UInt_t ch)const
{
  if (ch >= fChannelToWireMap.size()) {
    throw LArUtilException(Form("Invalid channel number: %d", ch));
    return galleryfmwk::data::kINVALID_CHAR;
  }
  return fChannelToWireMap.at(ch);
}


galleryfmwk::geo::WireID Geometry::ChannelToWireID(const UInt_t ch)const
{
  if (ch >= fChannelToWireMap.size()) {
    throw LArUtilException(Form("Invalid channel number: %d", ch));
    return galleryfmwk::geo::WireID();
  }

  UInt_t wire  = fChannelToWireMap.at(ch);
  UInt_t plane = ChannelToPlane(ch);

  galleryfmwk::geo::WireID wireID(0, 0, plane, wire);

  return wireID;
}

galleryfmwk::geo::SigType_t Geometry::SignalType(const UInt_t ch) const
{
  if (ch >= fChannelToPlaneMap.size()) {
    throw LArUtilException(Form("Invalid Channel number :%d", ch));
    return galleryfmwk::geo::kMysteryType;
  }

  return fSignalType.at(fChannelToPlaneMap.at(ch));
}

galleryfmwk::geo::SigType_t Geometry::PlaneToSignalType(const UChar_t plane) const
{
  if (plane >= fSignalType.size()) {
    throw LArUtilException(Form("Invalid Plane number: %d", plane));
    return galleryfmwk::geo::kMysteryType;
  }

  return fSignalType.at(plane);
}

galleryfmwk::geo::View_t Geometry::View(const UInt_t ch) const
{
  if (ch >= fChannelToPlaneMap.size()) {
    throw LArUtilException(Form("Invalid Channel number :%d", ch));
    return galleryfmwk::geo::kUnknown;
  }

  return fViewType.at(fChannelToPlaneMap.at(ch));
}

galleryfmwk::geo::View_t Geometry::PlaneToView(const UChar_t plane) const
{
  if (plane >= fViewType.size()) {
    throw LArUtilException(Form("Invalid Plane number: %d", plane));
    return galleryfmwk::geo::kUnknown;
  }

  return fViewType.at(plane);
}

std::set<galleryfmwk::geo::View_t> const Geometry::Views() const
{
  std::set<galleryfmwk::geo::View_t> views;
  for (auto const v : fViewType) views.insert(v);
  return views;
}

UInt_t Geometry::PlaneWireToChannel(const UInt_t plane,
                                    const UInt_t wire) const
{

  if (plane >= Nplanes() || fPlaneWireToChannelMap.at(plane).size() <= wire) {
    throw LArUtilException(Form("Invalid (plane, wire) = (%d, %d)", plane, wire));
    return galleryfmwk::data::kINVALID_UINT;
  }
  return fPlaneWireToChannelMap.at(plane).at(wire);
}

UInt_t Geometry::NearestChannel(const Double_t worldLoc[3],
                                const UInt_t PlaneNo) const
{
  return PlaneWireToChannel(PlaneNo, NearestWire(worldLoc, PlaneNo));
}

UInt_t Geometry::NearestChannel(const std::vector<Double_t> &worldLoc,
                                const UInt_t PlaneNo) const
{
  return PlaneWireToChannel(PlaneNo, NearestWire(worldLoc, PlaneNo));
}

UInt_t Geometry::NearestChannel(const TVector3 &worldLoc,
                                const UInt_t PlaneNo) const
{
  return PlaneWireToChannel(PlaneNo, NearestWire(worldLoc, PlaneNo));
}

UInt_t Geometry::NearestWire(const Double_t worldLoc[3],
                             const UInt_t PlaneNo) const
{
  TVector3 loc(worldLoc);
  return NearestWire(loc, PlaneNo);
}

UInt_t Geometry::NearestWire(const std::vector<Double_t> &worldLoc,
                             const UInt_t PlaneNo) const
{
  TVector3 loc(&worldLoc[0]);
  return NearestWire(loc, PlaneNo);
}

UInt_t Geometry::NearestWire(const TVector3 &worldLoc,
                             const UInt_t PlaneNo) const
{
  int NearestWireNumber = int(nearbyint(worldLoc[1] * fOrthVectorsY.at(PlaneNo)
                                        + worldLoc[2] * fOrthVectorsZ.at(PlaneNo)
                                        - fFirstWireProj.at(PlaneNo)));

  unsigned int wireNumber = (unsigned int) NearestWireNumber;

  if (NearestWireNumber < 0 ||
      NearestWireNumber >= (int)(this->Nwires(PlaneNo)) ) {

    if (NearestWireNumber < 0) wireNumber = 0;
    else wireNumber = this->Nwires(PlaneNo) - 1;

    larutil::InvalidWireError err(Form("Can't find nearest wire for (%g,%g,%g)",
                                       worldLoc[0], worldLoc[1], worldLoc[2]));
    err.better_wire_number = wireNumber;

    throw err;
  }
  /*
  std::cout<<"NearestWireID"<<std::endl;
  std::cout<<Form("(%g,%g,%g) position ... using (%g,%g,%g) ... Wire %d Plane %d",
                  worldLoc[0],worldLoc[1],worldLoc[2],
                  fOrthVectorsY[PlaneNo],
                  fOrthVectorsZ[PlaneNo],
                  fFirstWireProj[PlaneNo],
                  wireNumber,PlaneNo)
           << std::endl;
  */
  return wireNumber;
}

/// exact wire coordinate (fractional wire) to input world coordinates
Double_t Geometry::WireCoordinate(const Double_t worldLoc[3],
                                  const UInt_t   PlaneNo) const
{
  TVector3 loc(worldLoc);
  return WireCoordinate(loc, PlaneNo);
}

/// exact wire coordinate (fractional wire) to input world coordinate
Double_t Geometry::WireCoordinate(const std::vector<Double_t> &worldLoc,
                                  const UInt_t  PlaneNo) const
{
  TVector3 loc(&worldLoc[0]);
  return WireCoordinate(loc, PlaneNo);
}

/// exact wire coordinate (fractional wire) to input world coordinates
Double_t Geometry::WireCoordinate(const TVector3& worldLoc,
                                  const UInt_t PlaneNo) const
{

  Double_t NearestWireNumber = worldLoc[1] * fOrthVectorsY.at(PlaneNo)
                               + worldLoc[2] * fOrthVectorsZ.at(PlaneNo)
                               - fFirstWireProj.at(PlaneNo);


  /*
  std::cout<<"NearestWireID"<<std::endl;
  std::cout<<Form("(%g,%g,%g) position ... using (%g,%g,%g) ... Wire %d Plane %d",
                  worldLoc[0],worldLoc[1],worldLoc[2],
                  fOrthVectorsY[PlaneNo],
                  fOrthVectorsZ[PlaneNo],
                  fFirstWireProj[PlaneNo],
                  wireNumber,PlaneNo)
           << std::endl;
  */
  return NearestWireNumber;
}


// distance between planes p1 < p2
Double_t Geometry::PlanePitch(const UChar_t p1, const UChar_t p2) const
{
  if ( p1 == p2 ) return 0;
  else if ( (p1 == 0 && p2 == 1) || (p1 == 1 && p2 == 0) ) return fPlanePitch.at(0);
  else if ( (p1 == 1 && p2 == 2) || (p1 == 2 && p2 == 1) ) return fPlanePitch.at(1);
  else if ( (p1 == 0 && p2 == 2) || (p1 == 2 && p2 == 0) ) return fPlanePitch.at(2);
  else {
    throw LArUtilException("Plane number > 2 not supported!");
    return galleryfmwk::data::kINVALID_DOUBLE;
  }
}

Double_t Geometry::WirePitch(const UInt_t  w1,
                             const UInt_t  w2,
                             const UChar_t plane) const
{
  if ( w1 > w2 && w1 >= fPlaneWireToChannelMap.at(plane).size() ) {
    throw LArUtilException(Form("Invalid wire number: %d", w1));
    return galleryfmwk::data::kINVALID_DOUBLE;
  }
  if ( w2 > w1 && w2 >= fPlaneWireToChannelMap.at(plane).size() ) {
    throw LArUtilException(Form("Invalid wire number: %d", w2));
    return galleryfmwk::data::kINVALID_DOUBLE;
  }

  return ( w1 < w2 ? (w2 - w1) * (fWirePitch.at(fViewType.at(plane))) : (w1 - w2) * (fWirePitch.at(fViewType.at(plane))));
}

/// assumes all planes in a view have the same pitch
Double_t   Geometry::WirePitch(const galleryfmwk::geo::View_t view) const
{
  if ((size_t)view > Nviews()) {
    throw LArUtilException(Form("Invalid view: %d", view));
    return galleryfmwk::data::kINVALID_DOUBLE;
  }

  return fWirePitch.at((size_t)view);
}

Double_t   Geometry::WireAngleToVertical(galleryfmwk::geo::View_t view) const
{
  if ((size_t)view > Nviews()) {
    throw LArUtilException(Form("Invalid view: %d", view));
    return galleryfmwk::data::kINVALID_DOUBLE;
  }

  return fWireAngle.at((size_t)view);
}


void Geometry::WireEndPoints(const UChar_t plane,
                             const UInt_t wire,
                             Double_t *xyzStart, Double_t *xyzEnd) const
{

  if (plane >= fWireStartVtx.size())  {
    throw LArUtilException(Form("Plane %d invalid!", plane));
    return;
  }
  if (wire >= fWireStartVtx.at(plane).size()) {
    // throw LArUtilException(Form("Wire %d invalid for plane %d! Max is %i.", wire, 
    //                                                                         plane, 
    //                                                                         fWireStartVtx.at(plane).size()-1));
    std::cout << "Ahhhhhhhhhhhhhhh"<< Form("Wire %d invalid for plane %d! Max is %i.", wire, 
                                                                            plane, 
                                                                            fWireStartVtx.at(plane).size()-1) << std::endl;
    return;
  }

  xyzStart[0] = fWireStartVtx.at(plane).at(wire).at(0);
  xyzStart[1] = fWireStartVtx.at(plane).at(wire).at(1);
  xyzStart[2] = fWireStartVtx.at(plane).at(wire).at(2);
  xyzEnd[0]   = fWireEndVtx.at(plane).at(wire).at(0);
  xyzEnd[1]   = fWireEndVtx.at(plane).at(wire).at(1);
  xyzEnd[2]   = fWireEndVtx.at(plane).at(wire).at(2);

}

bool Geometry::ChannelsIntersect(const UInt_t c1,
                                 const UInt_t c2,
                                 Double_t &y, Double_t &z) const
{
  if (c1 == c2) {
    throw LArUtilException("Same channel does not intersect!");
    return false;
  }

  if ( c1 >= fChannelToPlaneMap.size() || c2 >= fChannelToPlaneMap.size() ) {
    throw LArUtilException(Form("Invalid channels : %d and %d", c1, c2));
    return false;
  }
  if ( fViewType.at(fChannelToPlaneMap.at(c1)) == fViewType.at(fChannelToPlaneMap.at(c2)) ) {
    return false;
  }

  UInt_t w1 = fChannelToWireMap.at(c1);
  UInt_t w2 = fChannelToWireMap.at(c2);

  UChar_t p1 = fChannelToPlaneMap.at(c1);
  UChar_t p2 = fChannelToPlaneMap.at(c2);

  galleryfmwk::geo::View_t v1 = fViewType.at(p1);
  galleryfmwk::geo::View_t v2 = fViewType.at(p2);

  Double_t start1[3] = {0.};
  Double_t start2[3] = {0.};
  Double_t end1[3] = {0.};
  Double_t end2[3] = {0.};

  std::cout << "Geometry::ChannelsIntersect about to call WireEndPoints" << std::endl;
  WireEndPoints(p1, w1, start1, end1);
  WireEndPoints(p2, w2, start2, end2);

  // if endpoint of one input wire is within range of other input wire in
  // BOTH y AND z, wires overlap
  bool overlapY = (ValueInRange(start1[1], start2[1], end2[1]) || ValueInRange(end1[1], start2[1], end2[1]));
  bool overlapZ = (ValueInRange(start1[2], start2[2], end2[2]) || ValueInRange(end1[2], start2[2], end2[2]));

  bool overlapY_rev = (ValueInRange(start2[1], start1[1], end1[1]) || ValueInRange(end2[1], start1[1], end1[1]));
  bool overlapZ_rev = (ValueInRange(start2[2], start1[2], end1[2]) || ValueInRange(end2[2], start1[2], end1[2]));

  // override y overlap checks if a vertical plane exists:
  if ( fWireAngle.at(v1) == TMath::Pi() / 2 || fWireAngle.at(v2) == TMath::Pi() / 2 ) {
    overlapY     = true;
    overlapY_rev = true;
  }

  //catch to get vertical wires, where the standard overlap might not work, Andrzej
  if (std::abs(start2[2] - end2[2]) < 0.01) overlapZ = overlapZ_rev;


  if (overlapY && overlapZ) {
    IntersectionPoint(w1, w2, p1, p2,
                      start1, end1,
                      start2, end2,
                      y, z);
    return true;
  }

  else if (overlapY_rev && overlapZ_rev) {
    this->IntersectionPoint(w2, w1, p2, p1,
                            start2, end2,
                            start1, end1,
                            y, z);
    return true;
  }

  return false;

}

void Geometry::IntersectionPoint(const UInt_t  wire1,  const UInt_t  wire2,
                                 const UChar_t plane1, const UChar_t plane2,
                                 Double_t start_w1[3], Double_t end_w1[3],
                                 Double_t start_w2[3], Double_t end_w2[3],
                                 Double_t &y, Double_t &z) const
{

  galleryfmwk::geo::View_t v1 = fViewType.at(plane1);
  galleryfmwk::geo::View_t v2 = fViewType.at(plane2);
  //angle of wire1 wrt z-axis in Y-Z plane...in radians
  Double_t angle1 = fWireAngle.at(v1);
  //angle of wire2 wrt z-axis in Y-Z plane...in radians
  Double_t angle2 = fWireAngle.at(v2);

  if (angle1 == angle2) return; //comparing two wires in the same plane...pointless.

  //coordinates of "upper" endpoints...(z1,y1) = (a,b) and (z2,y2) = (c,d)
  double a = 0.;
  double b = 0.;
  double c = 0.;
  double d = 0.;
  double angle = 0.;
  double anglex = 0.;

  // below is a special case of calculation when one of the planes is vertical.
  angle1 < angle2 ? angle = angle1 : angle = angle2;//get angle closest to the z-axis

  // special case, one plane is vertical
  if (angle1 == TMath::Pi() / 2 || angle2 == TMath::Pi() / 2) {
    if (angle1 == TMath::Pi() / 2) {

      anglex = (angle2 - TMath::Pi() / 2);
      a = end_w1[2];
      b = end_w1[1];
      c = end_w2[2];
      d = end_w2[1];
      // the if below can in principle be replaced by the sign of anglex (inverted)
      // in the formula for y below. But until the geometry is fully symmetric in y I'm
      // leaving it like this. Andrzej
      if ((anglex) > 0 ) b = start_w1[1];

    }
    else if (angle2 == TMath::Pi() / 2) {
      anglex = (angle1 - TMath::Pi() / 2);
      a = end_w2[2];
      b = end_w2[1];
      c = end_w1[2];
      d = end_w1[1];
      // the if below can in principle be replaced by the sign of anglex (inverted)
      // in the formula for y below. But until the geometry is fully symmetric in y I'm
      // leaving it like this. Andrzej
      if ((anglex) > 0 ) b = start_w2[1];
    }

    y = b + ((c - a) - (b - d) * tan(anglex)) / tan(anglex);
    z = a;   // z is defined by the wire in the vertical plane

    return;
  }

  // end of vertical case
  z = 0; y = 0;

  if (angle1 < (TMath::Pi() / 2.0)) {
    c = end_w1[2];
    d = end_w1[1];
    a = start_w2[2];
    b = start_w2[1];
  }
  else {
    c = end_w2[2];
    d = end_w2[1];
    a = start_w1[2];
    b = start_w1[1];
  }

  //Intersection point of two wires in the yz plane is completely
  //determined by wire endpoints and angle of inclination.
  z = 0.5 * ( c + a + (b - d) / TMath::Tan(angle) );
  y = 0.5 * ( b + d + (a - c) * TMath::Tan(angle) );

  return;

}

// Added shorthand function where start and endpoints are looked up automatically
//  - whether to use this or the full function depends on optimization of your
//    particular algorithm.  Ben J, Oct 2011
//--------------------------------------------------------------------
void Geometry::IntersectionPoint(const UInt_t  wire1,  const UInt_t  wire2,
                                 const UChar_t plane1, const UChar_t plane2,
                                 Double_t &y, Double_t &z) const

{
  double WireStart1[3] = {0.};
  double WireStart2[3] = {0.};
  double WireEnd1[3]   = {0.};
  double WireEnd2[3]   = {0.};

  std::cout << "Geometry::IntersectionPoint about to call WireEndPoints" << std::endl;
  this->WireEndPoints(plane1, wire1, WireStart1, WireEnd1);
  this->WireEndPoints(plane2, wire2, WireStart2, WireEnd2);
  this->IntersectionPoint(wire1, wire2, plane1, plane2,
                          WireStart1, WireEnd1,
                          WireStart2, WireEnd2, y, z);
}

UInt_t Geometry::GetClosestOpDet(const Double_t *xyz) const
{
  Double_t dist2      = 0;
  Double_t min_dist2  = galleryfmwk::data::kINVALID_DOUBLE;
  UInt_t   closest_ch = galleryfmwk::data::kINVALID_UINT;
  for (size_t ch = 0; ch < fOpDetVtx.size(); ++ch) {

    dist2 =
      pow(xyz[0] - fOpDetVtx.at(ch).at(0), 2) +
      pow(xyz[1] - fOpDetVtx.at(ch).at(1), 2) +
      pow(xyz[2] - fOpDetVtx.at(ch).at(2), 2);

    if ( dist2 < min_dist2 ) {

      min_dist2 = dist2;
      closest_ch = ch;

    }
  }

  return closest_ch;
}

UInt_t Geometry::GetClosestOpDet(const Double_t *xyz, Double_t &dist) const
{
  Double_t min_dist2  = galleryfmwk::data::kINVALID_DOUBLE;
  UInt_t   closest_ch = galleryfmwk::data::kINVALID_UINT;
  for (size_t ch = 0; ch < fOpDetVtx.size(); ++ch) {

    dist =
      pow(xyz[0] - fOpDetVtx.at(ch).at(0), 2) +
      pow(xyz[1] - fOpDetVtx.at(ch).at(1), 2) +
      pow(xyz[2] - fOpDetVtx.at(ch).at(2), 2);

    if ( dist < min_dist2 ) {

      min_dist2 = dist;
      closest_ch = ch;

    }
  }
  dist = sqrt(dist);
  return closest_ch;
}

UInt_t Geometry::OpDetFromOpChannel(UInt_t ch) const
{
  if (ch >= fOpChannel2OpDet.size())
    throw LArUtilException(Form("Invalid OpChannel: %d", ch));
  return fOpChannel2OpDet[ch];
}

void Geometry::GetOpChannelPosition(const UInt_t i, Double_t *xyz) const
{
  if ( i >= fOpChannelVtx.size() ) {
    throw LArUtilException(Form("Invalid PMT channel number: %d", i));
    xyz[0] = galleryfmwk::data::kINVALID_DOUBLE;
    xyz[0] = galleryfmwk::data::kINVALID_DOUBLE;
    xyz[0] = galleryfmwk::data::kINVALID_DOUBLE;
    return;
  }

  xyz[0] = fOpChannelVtx.at(i).at(0);
  xyz[1] = fOpChannelVtx.at(i).at(1);
  xyz[2] = fOpChannelVtx.at(i).at(2);
  return;
}

void Geometry::GetOpDetPosition(const UInt_t i, Double_t *xyz) const
{
  if ( i >= fOpDetVtx.size() ) {
    throw LArUtilException(Form("Invalid PMT channel number: %d", i));
    xyz[0] = galleryfmwk::data::kINVALID_DOUBLE;
    xyz[0] = galleryfmwk::data::kINVALID_DOUBLE;
    xyz[0] = galleryfmwk::data::kINVALID_DOUBLE;
    return;
  }

  xyz[0] = fOpDetVtx.at(i).at(0);
  xyz[1] = fOpDetVtx.at(i).at(1);
  xyz[2] = fOpDetVtx.at(i).at(2);
  return;
}

const std::vector<Double_t>& Geometry::PlaneOriginVtx(UChar_t plane)
{
  if (plane >= fPlaneOriginVtx.size()) {
    throw LArUtilException(Form("Invalid plane number: %d", plane));
    fPlaneOriginVtx.push_back(std::vector<Double_t>(3, galleryfmwk::data::kINVALID_DOUBLE));
    return fPlaneOriginVtx.at(this->Nplanes());
  }

  return fPlaneOriginVtx.at(plane);
}

void Geometry::PlaneOriginVtx(UChar_t plane, Double_t *vtx) const
{
  vtx[0] = fPlaneOriginVtx.at(plane)[0];
  vtx[1] = fPlaneOriginVtx.at(plane)[1];
  vtx[2] = fPlaneOriginVtx.at(plane)[2];
}

}


#endif
