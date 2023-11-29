#include <stdinclude.hpp>

namespace SCGUIData {
	bool needExtractText = false;

	int screenW = 1280;
	int screenH = 720;
	bool screenFull = false;

	float sysCamFov = 60;
	Vector3_t sysCamPos{};
	Vector3_t sysCamLookAt{};
	Quaternion_t sysCamRot{};

	void updateSysCamLookAt() {
		BaseCamera::CameraPosRotToLookAt(sysCamPos, sysCamRot, &sysCamLookAt);
	}

}
