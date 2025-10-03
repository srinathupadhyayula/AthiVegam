# AthiVegam Engine Modules

# Include compiler warnings helper
include(CompilerWarnings)

# Core module (Foundation - low-level utilities)
add_subdirectory(Core)

# Job System (depends on Core)
add_subdirectory(Jobs)

# Communication Layer (depends on Core and Jobs)
add_subdirectory(Comm)
# add_subdirectory(ECS)
# add_subdirectory(Rendering)
# add_subdirectory(Physics)
# add_subdirectory(Scripting)
# add_subdirectory(Networking)
# add_subdirectory(Assets)
# add_subdirectory(EditorIPC)

