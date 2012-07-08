#ifndef ENGINE_DEMORECORDER_H
#define ENGINE_DEMORECORDER_H

#include "kernel.h"

class IDemoVideoRecorder : public IInterface
{
	MACRO_INTERFACE("demovideorecorder", 0)
public:
    IDemoVideoRecorder() {}
    ~IDemoVideoRecorder() {}
};

#endif
