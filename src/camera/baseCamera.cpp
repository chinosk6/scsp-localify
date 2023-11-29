#include <stdinclude.hpp>
#include "camera/baseCamera.hpp"

namespace BaseCamera {
	namespace CameraCalc {


		Vector3::Vector3(float _x, float _y, float _z)
		{
			x = _x;
			y = _y;
			z = _z;
		}

		Vector3::Vector3(Vector3_t& vec) {
			x = vec.x;
			y = vec.y;
			z = vec.z;
		}

		Vector3 Vector3::operator+(const Vector3& v) const
		{
			return Vector3(x + v.x, y + v.y, z + v.z);
		}

		Vector3 Vector3::operator-(const Vector3& other) const {
			return Vector3(x - other.x, y - other.y, z - other.z);
		}

		Vector3 Vector3::operator*(float s) const
		{
			return Vector3(x * s, y * s, z * s);
		}

		Vector3 Vector3::normalized() const {
			float n = norm();
			return Vector3(x / n, y / n, z / n);
		}

		float Vector3::norm() const {
			return std::sqrt(x * x + y * y + z * z);
		}

		Vector3 Vector3::cross(const Vector3& a, const Vector3& b) {
			return Vector3(a.y * b.z - a.z * b.y,
				a.z * b.x - a.x * b.z,
				a.x * b.y - a.y * b.x);
		}


		Quaternion::Quaternion(float _w, float _x, float _y, float _z)
		{
			w = _w;
			x = _x;
			y = _y;
			z = _z;
		}

		Quaternion::Quaternion(Quaternion_t& quat) {
			w = quat.w;
			x = quat.x;
			y = quat.y;
			z = quat.z;
		}

		Quaternion::operator Quaternion_t() const {
			return Quaternion_t{ w,x,y,z };
		}

		Quaternion Quaternion::operator*(const Quaternion& q) const
		{
			float nw = w * q.w - x * q.x - y * q.y - z * q.z;
			float nx = w * q.x + x * q.w + y * q.z - z * q.y;
			float ny = w * q.y - x * q.z + y * q.w + z * q.x;
			float nz = w * q.z + x * q.y - y * q.x + z * q.w;
			return Quaternion(nw, nx, ny, nz);
		}

		Quaternion Quaternion::operator *(const float& rhs) const {
			return Quaternion(w * rhs, x * rhs, y * rhs, z * rhs);
		}

		Quaternion Quaternion::operator +(const Quaternion& rhs) const {
			return Quaternion(w + rhs.w, x + rhs.x, y + rhs.y, z + rhs.z);
		}

		Quaternion Quaternion::operator -(const Quaternion& rhs) const {
			return Quaternion(w - rhs.w, x - rhs.x, y - rhs.y, z - rhs.z);
		}

		Quaternion Quaternion::operator -() const {
			return Quaternion(-w, -x, -y, -z);
		}

		float Quaternion::norm() const {
			return std::sqrt(w * w + x * x + y * y + z * z);
		}

		Quaternion Quaternion::normalized() const {
			float n = norm();
			return Quaternion(w / n, x / n, y / n, z / n);
		}

		Quaternion Quaternion::Conjugate() const
		{
			return Quaternion(w, -x, -y, -z);
		}

		//  roll, pitch, yaw
		Vector3 Quaternion::ToEuler() {
			Vector3 euler(0, 0, 0);
			// 计算 roll (x-axis rotation)
			double sinr = 2.0 * (w * x + y * z);
			double cosr = 1.0 - 2.0 * (x * x + y * y);
			euler.x = atan2(sinr, cosr);

			// 计算 pitch (y-axis rotation)
			double sinp = 2.0 * (w * y - z * x);
			if (fabs(sinp) >= 1) {
				euler.y = copysign(M_PI / 2, sinp); // use 90 degrees if out of range
			}
			else {
				euler.y = asin(sinp);
			}

			// 计算 yaw (z-axis rotation)
			double siny = 2.0 * (w * z + x * y);
			double cosy = 1.0 - 2.0 * (y * y + z * z);
			euler.z = atan2(siny, cosy);

			return euler;
		}

		Quaternion Quaternion::FromEuler(Vector3 euler) {
			double cy = cos(euler.z * 0.5);
			double sy = sin(euler.z * 0.5);
			double cp = cos(euler.y * 0.5);
			double sp = sin(euler.y * 0.5);
			double cr = cos(euler.x * 0.5);
			double sr = sin(euler.x * 0.5);

			Quaternion q(0, 0, 0, 0);
			q.w = cr * cp * cy + sr * sp * sy;
			q.x = sr * cp * cy - cr * sp * sy;
			q.y = cr * sp * cy + sr * cp * sy;
			q.z = cr * cp * sy - sr * sp * cy;
			return q;
		}

		float Quaternion::Dot(const Quaternion& q1, const Quaternion& q2) {
			return q1.w * q2.w + q1.x * q2.x + q1.y * q2.y + q1.z * q2.z;
		}

		// 计算夹角
		float Quaternion::Acos(const float x) {
			if (x < -1.0f) {
				return M_PI;
			}
			else if (x > 1.0f) {
				return 0.0f;
			}
			else {
				return std::acos(x);
			}
		}

		// Slerp方法
		Quaternion Quaternion::Slerp(const Quaternion& q1, const Quaternion& q2, const float t) {
			Quaternion q3(0, 0, 0, 0);
			float dot = Quaternion::Dot(q1, q2);
			if (dot < 0.0f) {
				dot = -dot;
				q3.w = -q2.w;
				q3.x = -q2.x;
				q3.y = -q2.y;
				q3.z = -q2.z;
			}
			else {
				q3 = q2;
			}
			if (dot < 0.95f) {
				const float angle = 2.0f * Quaternion::Acos(dot);
				const float sinAngle = std::sinf(angle);
				const float sin1 = std::sinf((1.0f - t) * angle) / sinAngle;
				const float sin2 = std::sinf(t * angle) / sinAngle;
				q3.w = (q1.w * sin1 + q3.w * sin2);
				q3.x = (q1.x * sin1 + q3.x * sin2);
				q3.y = (q1.y * sin1 + q3.y * sin2);
				q3.z = (q1.z * sin1 + q3.z * sin2);
			}
			else {
				q3.w = q1.w + t * (q3.w - q1.w);
				q3.x = q1.x + t * (q3.x - q1.x);
				q3.y = q1.y + t * (q3.y - q1.y);
				q3.z = q1.z + t * (q3.z - q1.z);
			}
			return q3;
		}

		void SmoothQuaternion(Quaternion& q0, Quaternion& q1, const float threshold) {
			float angle = 2.0f * Quaternion::Acos(Quaternion::Dot(q0, q1));
			int forCount = 0;
			while (angle > threshold) {
				float t = threshold / angle;
				Quaternion q2 = Quaternion::Slerp(q0, q1, t);
				q0 = q2;
				angle = 2.0f * Quaternion::Acos(Quaternion::Dot(q0, q1));
				forCount++;
				if (forCount > 50) break;
			}
		}

		Quaternion LookRotation(const Vector3& forward, const Vector3& upwards = Vector3(0, 1, 0)) {
			Vector3 right = Vector3::cross(upwards, forward).normalized();
			Vector3 up = Vector3::cross(forward, right).normalized();
			float m[4][4] = {
				{right.x, up.x, forward.x, 0},
				{right.y, up.y, forward.y, 0},
				{right.z, up.z, forward.z, 0},
				{0, 0, 0, 1}
			};
			float tr = m[0][0] + m[1][1] + m[2][2];
			float qw, qx, qy, qz;
			if (tr > 0) {
				float S = sqrt(tr + 1.0f) * 2;
				qw = 0.25f * S;
				qx = (m[2][1] - m[1][2]) / S;
				qy = (m[0][2] - m[2][0]) / S;
				qz = (m[1][0] - m[0][1]) / S;
			}
			else if ((m[0][0] > m[1][1]) && (m[0][0] > m[2][2])) {
				float S = sqrt(1.0f + m[0][0] - m[1][1] - m[2][2]) * 2;
				qw = (m[2][1] - m[1][2]) / S;
				qx = 0.25f * S;
				qy = (m[0][1] + m[1][0]) / S;
				qz = (m[0][2] + m[2][0]) / S;
			}
			else if (m[1][1] > m[2][2]) {
				float S = sqrt(1.0f + m[1][1] - m[0][0] - m[2][2]) * 2;
				qw = (m[0][2] - m[2][0]) / S;
				qx = (m[0][1] + m[1][0]) / S;
				qy = 0.25f * S;
				qz = (m[1][2] + m[2][1]) / S;
			}
			else {
				float S = sqrt(1.0f + m[2][2] - m[0][0] - m[1][1]) * 2;
				qw = (m[1][0] - m[0][1]) / S;
				qx = (m[0][2] + m[2][0]) / S;
				qy = (m[1][2] + m[2][1]) / S;
				qz = 0.25f * S;
			}
			return Quaternion(qx, qy, qz, qw);
		}

		Quaternion RotateQuaternion(const Quaternion& q, float angle_degrees, const Vector3& axis) {
			float angle_radians = angle_degrees * M_PI / 180.0f;
			float half_angle = angle_radians * 0.5f;
			float s = sin(half_angle);
			Vector3 normalized_axis = axis.normalized();
			Quaternion q1(cos(half_angle), normalized_axis.x * s, normalized_axis.y * s, normalized_axis.z * s);
			Quaternion q2 = q * q1;
			return q2;
		}

		Vector3 RotateVector(const Quaternion& q, const Vector3& v)
		{
			Quaternion p(0, v.x, v.y, v.z);
			Quaternion q_inv = q.Conjugate();
			Quaternion rotated = q * p * q_inv;
			return Vector3(rotated.x, rotated.y, rotated.z);
		}

		Vector3 GetFrontPos(const Vector3& pos, const Quaternion& rot, float distance)
		{
			Vector3 v(0, 0, distance);
			Vector3 v_rotated = RotateVector(rot, v);
			Vector3 pos_front = pos + v_rotated;
			return pos_front;
		}

		Vector3_t GetFrontPos(const Vector3_t& pos, const Quaternion_t& rot, float distance) {
			Vector3 vPos(pos.x, pos.y, pos.z);
			Quaternion vQos(rot.w, rot.x, rot.y, rot.z);
			const auto ret = GetFrontPos(vPos, vQos, distance);
			return Vector3_t{ ret.x, ret.y, ret.z };
		}

	}


	float moveStep = 0.05;
	float look_radius = 5;  // 转向半径
	float moveAngel = 1.5;  // 转向角度

	int smoothLevel = 1;
	unsigned long sleepTime = 0;
	

	Camera::Camera() {
		Camera(0, 0, 0, 0, 0, 0);
	}

	Camera::Camera(Vector3_t* vec, Vector3_t* lookAt) {
		Camera(vec->x, vec->y, vec->z, lookAt->x, lookAt->y, lookAt->z);
	}

	Camera::Camera(Vector3_t& vec, Vector3_t& lookAt) {
		Camera(vec.x, vec.y, vec.z, lookAt.x, lookAt.y, lookAt.z);
	}

	Camera::Camera(float x, float y, float z, float lx, float ly, float lz) {
		pos.x = x;
		pos.y = y;
		pos.z = z;
		lookAt.x = lx;
		lookAt.y = ly;
		lookAt.z = lz;
	}

	void Camera::setPos(float x, float y, float z) {
		pos.x = x;
		pos.y = y;
		pos.z = z;
	}

	void Camera::setLookAt(float x, float y, float z) {
		lookAt.x = x;
		lookAt.y = y;
		lookAt.z = z;
	}

	void Camera::reset() {
		setPos(0.5, 1.1, 1.3);
		setLookAt(0.5, 1.1, -3.7);
		fov = 60;
		verticalAngle = 0;
		horizontalAngle = 0;
	}

	CameraCalc::Vector3 Camera::getPos() {
		return pos;
	}

	CameraCalc::Vector3 Camera::getLookAt() {
		return lookAt;
	}

	void Camera::set_lon_move(float vertanglePlus, LonMoveHState moveState) {  // 前后移动
		auto radian = (verticalAngle + vertanglePlus) * M_PI / 180;
		auto radianH = (double)horizontalAngle * M_PI / 180;

		auto f_step = cos(radian) * moveStep * cos(radianH) / smoothLevel;  // ↑↓
		auto l_step = sin(radian) * moveStep * cos(radianH) / smoothLevel;  // ←→
		// auto h_step = tan(radianH) * sqrt(pow(f_step, 2) + pow(l_step, 2));
		auto h_step = sin(radianH) * moveStep / smoothLevel;

		switch (moveState)
		{
		case LonMoveForward: break;
		case LonMoveBack: h_step = -h_step; break;
		default: h_step = 0; break;
		}

		for (int i = 0; i < smoothLevel; i++) {
			pos.z -= f_step;
			lookAt.z -= f_step;
			pos.x += l_step;
			lookAt.x += l_step;
			pos.y += h_step;
			lookAt.y += h_step;
			Sleep(sleepTime);
		}
	}

	void Camera::updateVertLook() {  // 上+
		auto radian = verticalAngle * M_PI / 180;
		auto radian2 = ((double)horizontalAngle - 90) * M_PI / 180;  // 日

		auto stepX1 = look_radius * sin(radian2) * cos(radian) / smoothLevel;
		auto stepX2 = look_radius * sin(radian2) * sin(radian) / smoothLevel;
		auto stepX3 = look_radius * cos(radian2) / smoothLevel;

		for (int i = 0; i < smoothLevel; i++) {
			lookAt.z = pos.z + stepX1;
			lookAt.y = pos.y + stepX3;
			lookAt.x = pos.x - stepX2;
			Sleep(sleepTime);
		}
	}

	void Camera::setHoriLook(float vertangle) {  // 左+
		auto radian = vertangle * M_PI / 180;
		auto radian2 = horizontalAngle * M_PI / 180;

		auto stepBt = cos(radian) * look_radius * cos(radian2) / smoothLevel;
		auto stepHi = sin(radian) * look_radius * cos(radian2) / smoothLevel;
		auto stepY = sin(radian2) * look_radius / smoothLevel;

		for (int i = 0; i < smoothLevel; i++) {
			lookAt.x = pos.x + stepHi;
			lookAt.z = pos.z - stepBt;
			lookAt.y = pos.y + stepY;
			Sleep(sleepTime);
		}
	}

	void Camera::updateOtherPos(Vector3_t* pos) {
		pos->x = this->pos.x;
		pos->y = this->pos.y;
		pos->z = this->pos.z;
	}

	void CameraPosRotToLookAt(const Vector3_t& cameraPosition, const Quaternion_t& cameraRotation, Vector3_t* lookatPosition) {
		const auto retPos = CameraCalc::GetFrontPos(cameraPosition, cameraRotation, look_radius);
		lookatPosition->x = retPos.x;
		lookatPosition->y = retPos.y;
		lookatPosition->z = retPos.z;
	}

}
