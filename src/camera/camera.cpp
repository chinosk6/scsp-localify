#include <stdinclude.hpp>
#include "camera/baseCamera.hpp"
#include <mhotkey.hpp>

#define KEY_W  87
#define KEY_S  83
#define KEY_A  65
#define KEY_D  68
#define KEY_R  82
#define KEY_UP  38
#define KEY_DOWN  40
#define KEY_LEFT  37
#define KEY_RIGHT  39
#define KEY_CTRL  17
#define KEY_SHIFT  16
#define KEY_ALT  18
#define KEY_SPACE  32


namespace SCCamera {
	BaseCamera::Camera baseCamera{};

	bool rMousePressFlg = false;

	void reset_camera() {
		baseCamera.reset();
	}

	void camera_forward() {  // 向前
		baseCamera.set_lon_move(0, LonMoveHState::LonMoveForward);
	}
	void camera_back() {  // 后退
		baseCamera.set_lon_move(180, LonMoveHState::LonMoveBack);
	}
	void camera_left() {  // 向左
		baseCamera.set_lon_move(90);
	}
	void camera_right() {  // 向右
		baseCamera.set_lon_move(-90);
	}
	void camera_down() {  // 向下
		float preStep = BaseCamera::moveStep / BaseCamera::smoothLevel;

		for (int i = 0; i < BaseCamera::smoothLevel; i++) {
			baseCamera.pos.y -= preStep;
			baseCamera.lookAt.y -= preStep;
			Sleep(BaseCamera::sleepTime);
		}
	}
	void camera_up() {  // 向上
		float preStep = BaseCamera::moveStep / BaseCamera::smoothLevel;

		for (int i = 0; i < BaseCamera::smoothLevel; i++) {
			baseCamera.pos.y += preStep;
			baseCamera.lookAt.y += preStep;
			Sleep(BaseCamera::sleepTime);
		}
	}
	void cameraLookat_up(float mAngel, bool mouse = false) {
		baseCamera.horizontalAngle += mAngel;
		if (baseCamera.horizontalAngle >= 90) baseCamera.horizontalAngle = 89.99;
		baseCamera.updateVertLook();
	}
	void cameraLookat_down(float mAngel, bool mouse = false) {
		baseCamera.horizontalAngle -= mAngel;
		if (baseCamera.horizontalAngle <= -90) baseCamera.horizontalAngle = -89.99;
		baseCamera.updateVertLook();
	}
	void cameraLookat_left(float mAngel) {
		baseCamera.verticalAngle += mAngel;
		if (baseCamera.verticalAngle >= 360) baseCamera.verticalAngle = -360;
		baseCamera.setHoriLook(baseCamera.verticalAngle);
	}
	void cameraLookat_right(float mAngel) {
		baseCamera.verticalAngle -= mAngel;
		if (baseCamera.verticalAngle <= -360) baseCamera.verticalAngle = 360;
		baseCamera.setHoriLook(baseCamera.verticalAngle);
	}
	void changeCameraFOV(float value) {
		baseCamera.fov += value;
	}

	void onMouseScroll(LONG value) {
		changeCameraFOV(-value);
	}

	void mouseMove(LONG moveX, LONG moveY, int mouseEventType) {
		if (mouseEventType == 1) {  // down
			rMousePressFlg = true;
			int fCount = 0;
			while (ShowCursor(false) >= 0) {
				if (fCount >= 5) break;
				fCount++;
			}
		}
		else if (mouseEventType == 2) {  // up
			rMousePressFlg = false;
			int fCount = 0;
			while (ShowCursor(true) < 0) {
				if (fCount >= 5) break;
				fCount++;
			}
		}
		else if (mouseEventType == 3) {  // move
			std::thread([moveX, moveY]() {
				if (!rMousePressFlg) return;
				if (moveX > 0) {
					cameraLookat_right(moveX * g_free_camera_mouse_speed / 100.0);
				}
				else if (moveX < 0) {
					cameraLookat_left(-moveX * g_free_camera_mouse_speed / 100.0);
				}
				if (moveY > 0) {
					cameraLookat_down(moveY * g_free_camera_mouse_speed / 100.0, true);
				}
				else if (moveY < 0) {
					cameraLookat_up(-moveY * g_free_camera_mouse_speed / 100.0, true);
				}
				// printf("move x: %d, y: %d\n", moveX, moveY);
				}).detach();
		}
		else if (mouseEventType == 4) {  // scroll
			onMouseScroll(moveY);
		}
	}

	struct CameraMoveState {
		bool w = false;
		bool s = false;
		bool a = false;
		bool d = false;
		bool ctrl = false;
		bool space = false;
		bool up = false;
		bool down = false;
		bool left = false;
		bool right = false;
		bool q = false;
		bool e = false;
		bool i = false;
		bool k = false;
		bool j = false;
		bool l = false;
		bool threadRunning = false;

		void resetAll() {
			auto p = reinterpret_cast<bool*>(this);
			const auto numMembers = sizeof(*this) / sizeof(bool);
			for (size_t i = 0; i < numMembers; ++i) {
				p[i] = false;
			}
		}
	} cameraMoveState;

	void onKillFocus() {
		mouseMove(0, 0, 2);
		cameraMoveState.resetAll();

		std::thread([]() {
			Sleep(50);
			
			}).detach();
	}

	void cameraRawInputThread() {
		using namespace BaseCamera;

		std::thread([]() {
			if (cameraMoveState.threadRunning) return;
			cameraMoveState.threadRunning = true;
			while (true) {
				if (cameraMoveState.w) camera_forward();
				if (cameraMoveState.s) camera_back();
				if (cameraMoveState.a) camera_left();
				if (cameraMoveState.d) camera_right();
				if (cameraMoveState.ctrl) camera_down();
				if (cameraMoveState.space) camera_up();
				if (cameraMoveState.up) cameraLookat_up(moveAngel);
				if (cameraMoveState.down) cameraLookat_down(moveAngel);
				if (cameraMoveState.left) cameraLookat_left(moveAngel);
				if (cameraMoveState.right) cameraLookat_right(moveAngel);
				if (cameraMoveState.q) changeCameraFOV(0.5f);
				if (cameraMoveState.e) changeCameraFOV(-0.5f);
				// if (cameraMoveState.i) changeLiveFollowCameraOffsetY(moveStep / 3);
				// if (cameraMoveState.k) changeLiveFollowCameraOffsetY(-moveStep / 3);
				// if (cameraMoveState.j) changeLiveFollowCameraOffsetX(moveStep * 10);
				// if (cameraMoveState.l) changeLiveFollowCameraOffsetX(-moveStep * 10);
				Sleep(10);
			}
			}).detach();
	}

	void on_cam_rawinput_keyboard(int message, int key) {
		// printf("key %d - %d\n", message, key);
		if (message == WM_KEYDOWN || message == WM_KEYUP) {
			switch (key) {
			case KEY_W:
				cameraMoveState.w = message == WM_KEYDOWN ? true : false; break;
			case KEY_S:
				cameraMoveState.s = message == WM_KEYDOWN ? true : false; break;
			case KEY_A:
				cameraMoveState.a = message == WM_KEYDOWN ? true : false; break;
			case KEY_D:
				cameraMoveState.d = message == WM_KEYDOWN ? true : false; break;
			case KEY_CTRL:
				cameraMoveState.ctrl = message == WM_KEYDOWN ? true : false; break;
			case KEY_SPACE:
				cameraMoveState.space = message == WM_KEYDOWN ? true : false; break;
			case KEY_UP:
				cameraMoveState.up = message == WM_KEYDOWN ? true : false; break;
			case KEY_DOWN:
				cameraMoveState.down = message == WM_KEYDOWN ? true : false; break;
			case KEY_LEFT:
				cameraMoveState.left = message == WM_KEYDOWN ? true : false; break;
			case KEY_RIGHT:
				cameraMoveState.right = message == WM_KEYDOWN ? true : false; break;
			case 'Q':
				cameraMoveState.q = message == WM_KEYDOWN ? true : false; break;
			case 'E':
				cameraMoveState.e = message == WM_KEYDOWN ? true : false; break;
			case 'I':
				cameraMoveState.i = message == WM_KEYDOWN ? true : false; break;
			case 'K':
				cameraMoveState.k = message == WM_KEYDOWN ? true : false; break;
			case 'J':
				cameraMoveState.j = message == WM_KEYDOWN ? true : false; break;
			case 'L':
				cameraMoveState.l = message == WM_KEYDOWN ? true : false; break;
			case 'R': {
				if (message == WM_KEYDOWN) reset_camera();
			}; break;
			case 192: {
				if (message == WM_KEYDOWN) mouseMove(0, 0, rMousePressFlg ? 2 : 1);
			}; break;
			default: break;
			}
		}
	}

	void initCameraSettings() {
		reset_camera();
		cameraRawInputThread();
		MHotkey::setMKeyBoardRawCallBack(on_cam_rawinput_keyboard);
		// MHotkey::SetKeyCallBack(on_keyboard_down);
	}

}
