//
// helper class for getting depth map from kinect using OpenNI
// Created by James Anderson on 4/27/16.
//

#ifndef PROJECT_KINECT_H
#define PROJECT_KINECT_H


#include <XnOpenNI.h>
#include <XnCppWrapper.h>


class kinect {
private:
    xn::DepthGenerator _depth_generator;
    xn::UserGenerator _ugen;
    xn::Context _ctx;
    int _max_depth, _buffer_size;

public:
    kinect();
    ~kinect();

    void setup();

    const XnDepthPixel* get_depthmap_pointer();

    int get_max_depth() const {
        return _max_depth;
    }
    int get_buffer_size() const {
        return _buffer_size;
    }
};


#endif //PROJECT_KINECT_H
