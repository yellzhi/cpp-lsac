add_executable(brcd-key KeyAux.h main.cpp)
target_link_libraries(brcd-key PRIVATE brccore devcore Boost::program_options jsoncpp)
install(TARGETS brcd-key EXPORT brcdTargets DESTINATION bin)
