//
// Created by James Anderson on 4/27/16.
//

#include <glog/logging.h>
#include "kinect.h"

kinect::kinect():_max_depth(0), _buffer_size(0) {
    xn::EnumerationErrors errors;
    XnStatus status = XN_STATUS_OK;
    status = _ctx.Init();

    if (status != XN_STATUS_OK) {
        for(xn::EnumerationErrors::Iterator it = errors.Begin(); it != errors.End(); ++it) {
            XnChar desc[512];
            xnProductionNodeDescriptionToString(&it.Description(), desc,512);
            LOG(FATAL) << desc << " failed to to enumerate: " << xnGetStatusString(it.Error());
        }
    }
    xnPrintRegisteredLicenses();

    status = _ctx.FindExistingNode(XN_NODE_TYPE_DEPTH, _depth_generator);
    if (status != XN_STATUS_OK) {
        status = _depth_generator.Create(_ctx);
    }
    if (status != XN_STATUS_OK) {
        LOG(FATAL) << "couldnt get a depth generator";
    }
    LOG(INFO) << "kinect::kinect()";

    XnMapOutputMode map_mode;
    map_mode.nXRes = XN_VGA_X_RES;
    map_mode.nYRes = XN_VGA_Y_RES;
    map_mode.nFPS = 30;

    _buffer_size = map_mode.nXRes * map_mode.nYRes;

    status = _depth_generator.SetMapOutputMode(map_mode);
    _max_depth = _depth_generator.GetDeviceMaxDepth();

//    depth_texture.allocate(map_mode.nXRes, map_mode.nYRes, GL_RGB);
//    depth_pixels = new unsigned char[map_mode.nXRes * map_mode.nYRes * 3];
//    gray_pixels = new unsigned char[map_mode.nXRes * map_mode.nYRes];
//    memset(depth_pixels, 0, map_mode.nXRes * map_mode.nYRes * 4 * sizeof(unsigned char));

    _depth_generator.StartGenerating();
}

kinect::~kinect() {
    _depth_generator.Release();
    _ctx.Release();

    LOG(INFO) << "kinect::~kinect()";
}

void kinect::copy_depths(void *dest) {
    xn::DepthMetaData dmd;
    _depth_generator.GetMetaData(dmd);
    const XnDepthPixel* depth = _depth_generator.GetDepthMap();
    if(!dest) return;

    int xr = dmd.XRes(), yr = dmd.YRes();

    memccpy(dest, depth, xr * yr, sizeof(uint16_t));
}