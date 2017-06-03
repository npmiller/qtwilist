cmake_minimum_required(VERSION 2.8.11)
project(qtwilist)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)

set(qtwilist_SRC
  src/main.cpp
  src/qtwilist.cpp
  src/streamlist.cpp
)

# Create code from a list of Qt designer ui files.
#set(CMAKE_AUTOUIC ON) # use this if you have CMake 3.x instead of the following
qt5_wrap_ui(qtwilist_SRC src/qtwilist.ui src/adddialog.ui)

# Tell CMake to create the helloworld executable
add_executable(qtwilist ${qtwilist_SRC})

# Use the Widgets module from Qt 5.
target_link_libraries(qtwilist Qt5::Widgets Qt5::Network)

# Install the executable
install(TARGETS qtwilist DESTINATION bin)
