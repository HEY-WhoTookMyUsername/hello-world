#pragma once

/*
	Camera shake effect 
*/
struct CameraShake
{
	CameraShake(float duration);
	CameraShake(float duration, float amplitude, float freq);;
	Vector3 amplitude;
	Vector3 frequency;
	Vector3 offsets;
	float duration;
	float time = 0.0f;
};

/*
	Camera that hovers above the playfield track and can process camera shake and tilt effects
*/
class Camera
{
public:
	Camera();
	~Camera();

	// Updates the camera's shake effects, movement, etc.
	void Tick(float deltaTime, class BeatmapPlayback& playback);

	void AddCameraShake(CameraShake camerShake);
	void AddRollImpulse(float dir, float strength);

	// Changes the amount of roll applied when lasers are controlled, default = 1
	void SetRollIntensity(float val);
	void SetLasersActive(bool lasersActive);
	void SetTargetRoll(float target);
	float GetRoll() const;

	Vector2 Project(const Vector3& pos);

	// Generates a new render state for drawing from this cameras Point of View
	// the clipped boolean indicates whenether to clip the cameras clipping planes to the track range
	RenderState CreateRenderState(bool clipped);

	bool rollKeep = false;

	// The track being watched
	class Track* track;

	// Zoom values, both can range from -1 to 1 to control the track zoom
	float zoomBottom = 0.0f;
	float zoomTop = 0.0f;

	float cameraHeightBase = 0.35f;
	float cameraHeightMult = 1.0f;
	float cameraNearBase = 0.53f;
	float cameraNearMult = 1.0f;

private:
	float m_baseRollBlend = 0.0f;
	float m_ClampRoll(float in) const;
	// -1 to 1 roll value
	float m_roll = 0.0f;
	// Target to roll towards
	float m_targetRoll = 0.0f;
	bool m_targetRollSet = false;
	bool m_lasersActive = false;
	// Roll force
	float m_rollVelocity = 0.0f;
	float m_rollIntensity;

	RenderState m_rsLast;

	Vector<CameraShake> m_shakeEffects;
	// Base position with shake effects applied after a frame
	Vector3 m_shakeOffset;
};