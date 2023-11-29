#pragma once
#include <stdinclude.hpp>
#include <corecrt_math_defines.h>

enum LonMoveHState {
	LonMoveLeftAndRight,
	LonMoveForward,
	LonMoveBack
};

namespace BaseCamera {
	namespace CameraCalc {
		struct Vector3
		{
			float x, y, z;

			Vector3(float _x, float _y, float _z);
			Vector3(Vector3_t& vec);
			Vector3 operator+(const Vector3& v) const;
			Vector3 operator-(const Vector3& other) const;
			Vector3 operator*(float s) const;
			Vector3 normalized() const;
			float norm() const;
			static Vector3 cross(const Vector3& a, const Vector3& b);
		};

		struct Quaternion
		{
			float w, x, y, z;

			Quaternion(float _w, float _x, float _y, float _z);
			Quaternion(Quaternion_t& quat);
			operator Quaternion_t() const;
			Quaternion operator*(const Quaternion& q) const;
			Quaternion operator *(const float& rhs) const;
			Quaternion operator +(const Quaternion& rhs) const;
			Quaternion operator -(const Quaternion& rhs) const;
			Quaternion operator -() const;
			float norm() const;
			Quaternion normalized() const;
			Quaternion Conjugate() const;

			// roll, pitch, yaw
			Vector3 ToEuler();
			static Quaternion FromEuler(Vector3 euler);
			static float Dot(const Quaternion& q1, const Quaternion& q2);

			// 计算夹角
			static float Acos(const float x);

			// Slerp方法
			static Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, const float t);
		};

	}

	extern float moveStep;
	extern float look_radius;  // 转向半径
	extern float moveAngel;  // 转向角度

	extern int smoothLevel;
	extern unsigned long sleepTime;
	

	class Camera {
	public:
		Camera();
		Camera(Vector3_t& vec, Vector3_t& lookAt);
		Camera(Vector3_t* vec, Vector3_t* lookAt);
		Camera(float x, float y, float z, float lx, float ly, float lz);

		void reset();
		void setPos(float x, float y, float z);
		void setLookAt(float x, float y, float z);

		void set_lon_move(float vertanglePlus, LonMoveHState moveState = LonMoveHState::LonMoveLeftAndRight);
		void updateVertLook();
		void setHoriLook(float vertangle);

		void updateOtherPos(Vector3_t* pos);

		CameraCalc::Vector3 getPos();
		CameraCalc::Vector3 getLookAt();

		CameraCalc::Vector3 pos{0.5, 1.1, 1.3};
		CameraCalc::Vector3 lookAt{0.5, 1.1, -3.7};
		float fov = 60;

		float horizontalAngle = 0;  // 水平方向角度
		float verticalAngle = 0;  // 垂直方向角度

	};

	void CameraPosRotToLookAt(const Vector3_t& cameraPosition, const Quaternion_t& cameraRotation, Vector3_t* lookatPosition);

}
