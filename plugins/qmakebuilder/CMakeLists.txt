add_definitions(-DTRANSLATION_DOMAIN=\"kdevqmakebuilder\")

set(qmakebuilder_SRCS
    qmakebuilder.cpp
    qmakebuilderpreferences.cpp
    qmakejob.cpp
)
ecm_qt_declare_logging_category(qmakebuilder_SRCS
    HEADER debug.h
    IDENTIFIER KDEV_QMAKEBUILDER
    CATEGORY_NAME "kdevelop.plugins.qmakebuilder"
)
ki18n_wrap_ui(qmakebuilder_SRCS qmakeconfig.ui)
kconfig_add_kcfg_files(qmakebuilder_SRCS qmakebuilderconfig.kcfgc)
kdevplatform_add_plugin(kdevqmakebuilder JSON kdevqmakebuilder.json SOURCES ${qmakebuilder_SRCS})
target_link_libraries(kdevqmakebuilder
    KDev::Interfaces
    KDev::OutputView
    KDev::Util
    KDev::Project
    KDev::IMakeBuilder
    kdevqmakecommon
)

add_library(kdevqmakebuilder_interface INTERFACE)
add_library(KDev::QMakeBuilder ALIAS kdevqmakebuilder_interface)
target_include_directories(kdevqmakebuilder_interface INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>"
    "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/kdevelop>"
)
set_target_properties(kdevqmakebuilder_interface PROPERTIES
    EXPORT_NAME QMakeBuilder
)
install(TARGETS kdevqmakebuilder_interface EXPORT KDevelopTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES iqmakebuilder.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevelop/qmake COMPONENT Devel)
install( FILES data.kdev4 DESTINATION ${KDE_INSTALL_DATADIR}/kdevqmakebuilder )