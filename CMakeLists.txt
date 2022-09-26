cmake_minimum_required( VERSION 3.0 )

project( xvp LANGUAGES C )

set( PROJECT_SOURCES
    xvp.c
)

add_executable( xvp ${PROJECT_SOURCES} )
