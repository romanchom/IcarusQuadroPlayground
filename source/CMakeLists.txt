project(Main)


add_subdirectory("MPU-9255-driver")

add_library(${PROJECT_NAME} STATIC
    main.cpp
)

target_link_libraries(${PROJECT_NAME}
    "MPU-9255"
)
