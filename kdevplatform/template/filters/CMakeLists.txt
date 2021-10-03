add_library(kdev_filters MODULE kdevfilters.cpp)
grantlee_adjust_plugin_name(kdev_filters)
target_link_libraries(kdev_filters
    KDev::Language
    KDev::Serialization
)
if(USE_KFGRANTLEE)
    target_link_libraries(kdev_filters
        KF5::Grantlee
    )
else()
    target_link_libraries(kdev_filters
        Grantlee5::Templates
    )
endif()

if(USE_KFGRANTLEE)
    install(TARGETS kdev_filters DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/grantlee/${KF5Grantlee_VERSION_MAJOR}.${KF5Grantlee_VERSION_MINOR})
else()
    install(TARGETS kdev_filters DESTINATION ${KDE_INSTALL_PLUGINDIR}/grantlee/${Grantlee5_VERSION_MAJOR}.${Grantlee5_VERSION_MINOR})
endif()
