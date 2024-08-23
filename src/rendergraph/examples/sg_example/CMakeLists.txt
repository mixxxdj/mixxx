qt_add_executable(sg_example WIN32 MACOSX_BUNDLE
    customitem.cpp
    customitem.h
    main.cpp
)

target_link_libraries(sg_example PRIVATE
    rendergraph_sg
    rendergraph_examples
)

qt_add_qml_module(sg_example
    URI RenderGraph
    QML_FILES
        qml/main.qml
    RESOURCES
        images/test.png
    RESOURCE_PREFIX /example
    NO_RESOURCE_TARGET_PATH
)

install(TARGETS sg_example
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET sg_example
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
