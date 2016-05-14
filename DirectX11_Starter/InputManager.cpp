#include "InputManager.h"
#include <iostream>


InputManager::~InputManager()
{
}

InputManager* InputManager::_inputManager = 0;
InputManager* InputManager::getInstance() {

	if (!_inputManager) {
		_inputManager = new InputManager();
		std::cout << "Created an Instance of InputManager";
		return _inputManager;
	}
	else
	{
		return _inputManager;
	}
}