
# Source files
set(sources	
	"WaterRenderer.hpp"
	"WaterRenderer.cpp"

	"WaterSurface.hpp"
	"WaterSurface.cpp"

	"WaterSurface.cpp"

    "TemporaryTerrain.hpp"
)

# Add these sources to the project target
target_relative_sources(${CGRA_PROJECT} ${sources})
