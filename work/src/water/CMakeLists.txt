
# Source files
set(sources	
	"WaterRenderer.hpp"
	"WaterRenderer.cpp"

	"WaterSurface.hpp"
	"WaterSurface.cpp"

	"SkyBox.hpp"
	"SkyBox.cpp"

	"Timer.hpp"
	"Timer.cpp"
)

# Add these sources to the project target
target_relative_sources(${CGRA_PROJECT} ${sources})
