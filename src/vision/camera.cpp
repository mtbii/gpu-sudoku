#include "camera.h"

int Camera::deviceCount = 0;
bool Camera::isSetup = false;

CameraParams::CameraParams(int w, int h){
	params.mHeight = h;
	params.mWidth = w;
	params.mTargetBuf = new int[w*h];
}

CameraParams::~CameraParams(){
	delete[] params.mTargetBuf;
}

int CameraParams::height(){
	return params.mHeight;
}

int CameraParams::width(){
	return params.mWidth;
}

int* CameraParams::frameData(){
	return params.mTargetBuf;
}

SimpleCapParams* CameraParams::getParams(){
	return &params;
}

void CameraParams::set(int w, int h){
	delete[] params.mTargetBuf;
	params.mHeight = h;
	params.mWidth = w;
	params.mTargetBuf = new int[w*h];
}


Camera::Camera(){
	this->lastDevice = -1;

	if (!Camera::isSetup){
		Camera::deviceCount = setupESCAPI();
		Camera::isSetup = true;

		if (deviceCount == 0){
			throw std::exception("No capture devices detected!");
		}
	}
}

Camera::~Camera(){
	if (this->lastDevice >= 0){
		deinitCapture(this->lastDevice);
	}
	lastDevice = -1;
}

bool Camera::open(unsigned int deviceId, CameraParams* params){

	if (deviceId > deviceCount - 1){
		return false;
	}

	if (deviceId == lastDevice){
		return false;
	}
	else if (deviceId != lastDevice && lastDevice >= 0){
		close();
	}

	this->params = params;
	lastDevice = deviceId;
	SimpleCapParams *capParams = params->getParams();

	return initCapture(deviceId, capParams) != 0;
}

int Camera::getDeviceCount(){
	return (Camera::deviceCount = countCaptureDevices());
}

std::string Camera::getDeviceName(unsigned int deviceId){
	char buff[100];
	getCaptureDeviceName(deviceId, buff, 100);
	return std::string(buff);
}

void Camera::close(){
	if (this->lastDevice >= 0){
		deinitCapture(this->lastDevice);
	}
	lastDevice = -1;
}

bool Camera::requestFrame(af::array& frame){
	if (lastDevice < 0){
		throw std::exception("No device opened!");
	}

	doCapture(lastDevice);

	while (isCaptureDone(lastDevice) == 0);

	int h = params->height();
	int w = params->width();

	byte* rData = new byte[w*h];
	byte* gData = new byte[w*h];
	byte* bData = new byte[w*h];

	for (int i = 0; i < w; i++){
		for (int j = 0; j < h; j++){
			//Transpose and
			//Reverse columns for arrayfire graphics display
			rData[(i)*h + j] = (byte)(params->frameData()[i + j*w] >> 16) & 0xff;
			gData[(i)*h + j] = (byte)(params->frameData()[i + j*w] >> 8) & 0xff;
			bData[(i)*h + j] = (byte)(params->frameData()[i + j*w] >> 0) & 0xff;
		}
	}

	frame = af::join(2, af::array(h, w, rData), af::array(h, w, gData), af::array(h, w, bData)) / 255.0f;

	delete[] rData;
	delete[] gData;
	delete[] bData;

	return getCaptureErrorCodeProc(lastDevice) == 0;
}