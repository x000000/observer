project(observer)

include(LocalDefines.before.cmake OPTIONAL)

include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/UI/obs-frontend-api")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
#set(CMAKE_CXX_EXTENSIONS NO)
add_compile_options(/Zc:__cplusplus)

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
set(CMAKE_AUTOMOC TRUE)
set(CMAKE_AUTORCC TRUE)
set(CMAKE_AUTOUIC TRUE)

find_package(Qt5 COMPONENTS
        WebSockets
        Widgets
        REQUIRED
        )

set(observer_SOURCES
        observer.cpp
        action-config.cpp
        delegates/button-delegate.cpp
        delegates/multiline-delegate.cpp
        delegates/action-delegate.cpp
        delegates/spinbox-delegate.cpp
        settings-model.cpp
        action-delegate-editor.cpp
        observer-settings.cpp
        ws-chat-client.cpp
        )
set(observer_HEADERS
        observer.hpp
        action-config.hpp
        delegates/button-delegate.hpp
        delegates/multiline-delegate.hpp
        delegates/action-delegate.hpp
        delegates/spinbox-delegate.hpp
        settings-model.hpp
        action-delegate-editor.hpp
        observer-settings.hpp
        ws-chat-client.hpp
        )
set(observer_UI
        action-delegate-editor.ui
        observer-settings.ui
        )
qt5_wrap_ui(observer_UI_HEADERS
        ${observer_UI}
        )

add_library(observer MODULE
        ${observer_SOURCES}
        ${observer_HEADERS}
        ${observer_UI_HEADERS}
        )

target_link_libraries(observer
        libobs
        obs-frontend-api
        Qt5::WebSockets
        Qt5::Widgets
        )

set_target_properties(observer PROPERTIES FOLDER "plugins")

include(LocalDefines.after.cmake OPTIONAL)

install_obs_plugin_with_data(observer data)