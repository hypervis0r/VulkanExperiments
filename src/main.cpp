#include "main.h"

int main(void)
{
#ifdef NDEBUG
	const auto EnableValidationLayers = false;
#else
	const auto EnableValidationLayers = true;
#endif

	Engine::Renderer renderer(EnableValidationLayers);

	renderer.Run();
}