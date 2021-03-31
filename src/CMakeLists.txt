message (STATUS "added src")
add_library (rockprop SHARED primaryGeneratorAction.cxx steppingAction.cxx propagator.cxx)
target_link_libraries(rockprop ${Geant4_LIBRARIES} ${GENIE_LIBS_DIR_WITH_LFLAGS})

add_executable(runRockPropagation runPropagation.cxx)
target_link_libraries(runRockPropagation ${GENIE_LIBS_DIR_WITH_LFLAGS} ${Geant4_LIBRARIES} ${ROOT_LIBRARIES} ${log4cpp} ${pythia6} ${libxml2} rockprop)

add_executable(gntpc_dune gNtpConvDUNE.cxx)
target_link_libraries(gntpc_dune ${GENIE_LIBS_DIR_WITH_LFLAGS} ${ROOT_LIBRARIES} ${log4cpp} ${pythia6} ${libxml2})

install( TARGETS rockprop LIBRARY DESTINATION lib )
install( TARGETS gntpc_dune RUNTIME DESTINATION bin )
install( TARGETS runRockPropagation RUNTIME DESTINATION bin )
