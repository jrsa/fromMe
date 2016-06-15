#include <glog/logging.h>
#include "kinect.h"

kinect::kinect() : _max_depth(0), _buffer_size(0) {}

kinect::~kinect() {
  if (_depth_generator)
    _depth_generator.Release();
  _ctx.Release();

  LOG(INFO) << "kinect::~kinect()";
}

void kinect::setup() {
  xn::EnumerationErrors errors;
  XnStatus status = XN_STATUS_OK;
  status = _ctx.Init();

  if (status != XN_STATUS_OK) {
    for (xn::EnumerationErrors::Iterator it = errors.Begin();
         it != errors.End(); ++it) {
      XnChar desc[512];
      xnProductionNodeDescriptionToString(&it.Description(), desc, 512);
      LOG(ERROR) << desc << " failed to to enumerate: "
                 << xnGetStatusString(it.Error());
    }
  }
  xnPrintRegisteredLicenses();

  status = _ctx.FindExistingNode(XN_NODE_TYPE_DEPTH, _depth_generator);
  if (status != XN_STATUS_OK) {
    status = _depth_generator.Create(_ctx);
  }
  if (status != XN_STATUS_OK) {
    LOG(ERROR) << "couldnt get a depth generator";
    throw 20;
  } else {
    XnMapOutputMode map_mode;
    map_mode.nXRes = XN_VGA_X_RES;
    map_mode.nYRes = XN_VGA_Y_RES;
    map_mode.nFPS = 30;

    _buffer_size = map_mode.nXRes * map_mode.nYRes;

    status = _depth_generator.SetMapOutputMode(map_mode);
    _max_depth = _depth_generator.GetDeviceMaxDepth();
    _depth_generator.StartGenerating();
  }
}

const XnDepthPixel *kinect::get_depthmap_pointer() {

  XnStatus status = _depth_generator.WaitAndUpdateData();

  if (status != XN_STATUS_OK) {
    throw 69;
  }

  xn::DepthMetaData dmd;
  _depth_generator.GetMetaData(dmd);
  const XnDepthPixel *_map = dmd.Data();
  return _map;
}
