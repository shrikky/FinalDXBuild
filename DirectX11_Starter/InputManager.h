#pragma once
class InputManager
{
private:

	InputManager() {

	}
	static InputManager* _inputManager;
public:

	static InputManager* getInstance();
	~InputManager();
};

