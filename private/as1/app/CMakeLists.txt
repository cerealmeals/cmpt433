# app/CMakeLists.txt

# Create an executable for the application
add_executable(Reaction_Game
    main.c
    JoystickInterp.c
    JoystickInterp.h
)

# Include directories for the app
target_include_directories(Reaction_Game PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link the HAL library
target_link_libraries(Reaction_Game PRIVATE hal)