#pragma once

#include "../../deps/escapi/escapi.h"
#include <arrayfire.h>
#include <iostream>

class CameraParams{
public:
	CameraParams(int width, int height);
	virtual ~CameraParams();

	int width();
	int height();
	int* frameData();

	SimpleCapParams* getParams();
	void set(int width, int height);

private:
	SimpleCapParams params;
};

class Camera{
public:
	Camera();
	virtual ~Camera();

	bool open(unsigned int deviceId, CameraParams* params);
	int getDeviceCount();
	std::string getDeviceName(unsigned int deviceId);
	void close();
	bool requestFrame(af::array& frame);

private:
	static int deviceCount;
	static bool isSetup;
	int lastDevice;
	CameraParams* params;
};