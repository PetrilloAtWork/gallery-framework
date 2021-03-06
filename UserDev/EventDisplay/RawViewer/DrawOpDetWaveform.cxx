
#ifndef EVD_DRAWOPDETWAVEFORM_CXX
#define EVD_DRAWOPDETWAVEFORM_CXX


#include "DrawOpDetWaveform.h"
#include "LArUtil/DetectorProperties.h"

namespace evd {

DrawOpDetWaveform::DrawOpDetWaveform(const geo::GeometryCore& geometry, 
                                     const detinfo::DetectorProperties& detectorProperties, 
                                     const detinfo::DetectorClocks& detectorClocks) :
  _geo_service(geometry),
  _det_prop(detectorProperties),
  _det_clocks(detectorClocks)
{
  _name = "DrawOpDetWaveform";
  _producer = "opdaq";
  
  _import_array();
}


bool DrawOpDetWaveform::initialize() {

  initDataHolder();

  _tick_period = _det_clocks.OpticalClock().TickPeriod();
  _n_op_channels = _geo_service.NOpDets();
  _n_time_ticks = _det_clocks.OpticalClock().FrameTicks() * _n_frames;

  return true;
}

bool DrawOpDetWaveform::analyze(gallery::Event * ev) {

  art::InputTag op_wvf_tag(_producer);
  auto const & op_wvfs
    = ev -> getValidHandle<std::vector <raw::OpDetWaveform> >(op_wvf_tag);


  _wvf_data.clear();
  initDataHolder();

  for (auto const& op_wvf : *op_wvfs) {
    unsigned int       ch   = op_wvf.ChannelNumber();
    double             time = op_wvf.TimeStamp();


    // time offset is at -1250 mu s
    // time tick period is 0.002
    size_t time_in_ticks = (time + _time_offset) / _tick_period;

    int offset = ch * _n_time_ticks;

    size_t i = 0;
    for (short adc : op_wvf) {
      _wvf_data.at(offset + time_in_ticks + i) = (float)adc;
      i++;
    }

  }

  return true;
}

bool DrawOpDetWaveform::finalize() {

  return true;
}

void DrawOpDetWaveform::initDataHolder() {
  // for now 3 windows of 1250 mu s
  // there are 625000 time ticks per window
  // so a tick period of 0.002 mu s
  // int n_op_channels = 600;
  float default_value = -9999.;
  _wvf_data.clear();
  _wvf_data.resize(_n_op_channels * _n_time_ticks, default_value);
  // for (size_t i = 0; i < n_op_channels; i++) {
  //   _wvf_data.at(i).resize(_n_time_ticks);
  // }
  return;
}


PyObject * DrawOpDetWaveform::getArray() {

  PyObject * returnNull = nullptr;

  // int n_op_channels = 600;
  // int n_time_ticks = 625000*3;

  try {
    // Convert the wire data to numpy arrays:
    int n_dim = 2;
    int * dims = new int[n_dim];
    dims[0] = _n_op_channels;
    dims[1] = _n_time_ticks;
    int data_type = NPY_FLOAT;

    return (PyObject *) PyArray_FromDimsAndData(n_dim, dims, data_type, (char*) & ((_wvf_data)[0]) );
  }
  catch ( ... ) {
    std::cerr << "WARNING: CANNOT GET OP DET WAVEFORM.\n";
    return returnNull;
  }

}

}

#endif