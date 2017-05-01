#pragma once

class Component {
public:
	virtual void Initialize() {};
	virtual void PostInitialize() {}

	virtual void Update() {}

	virtual void Sync() {}

	virtual void Draw() {}
};