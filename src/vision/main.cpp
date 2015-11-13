#include <arrayfire.h>
#include <stdio.h>
#include "camera.h"

int main(){
	af::setDevice(0);
	af::info();
	std::cout << "*** ArrayFire Test ***" << std::endl << std::endl;

	af::Window wnd(800, 600, "Webcam");
	wnd.setColorMap(af::ColorMap::AF_COLORMAP_DEFAULT);
	CameraParams params = CameraParams(640, 480);
	Camera cam;

	if (cam.open(0, &params))
	{
		af::array frame;

		while (cam.requestFrame(frame) && !wnd.close()){
			if (!wnd.close()){
				wnd.image(frame);
			}
		}

		cam.close();
	}
	else{
		std::cout << "Problem starting the camera." << std::endl;
	}
}