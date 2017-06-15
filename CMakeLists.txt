cmake_minimum_required(VERSION 3.2)
project(qtwilist)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)

set(qtwilist_SRC
  src/main.cpp
  src/qtwilist.cpp
  src/qtwilist.ui
  src/streamlist.cpp
  src/adddialog.ui
)

add_executable(qtwilist ${qtwilist_SRC} res/icon.qrc)

target_link_libraries(qtwilist Qt5::Widgets Qt5::Network)

install(TARGETS qtwilist DESTINATION bin)
