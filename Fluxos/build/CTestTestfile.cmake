# CMake generated Testfile for 
# Source directory: /home/juliana/GitHub/Algoritmos-Avancados/Fluxos
# Build directory: /home/juliana/GitHub/Algoritmos-Avancados/Fluxos/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(FlowTestSuite "/home/juliana/GitHub/Algoritmos-Avancados/Fluxos/build/FlowTests")
set_tests_properties(FlowTestSuite PROPERTIES  _BACKTRACE_TRIPLES "/home/juliana/GitHub/Algoritmos-Avancados/Fluxos/CMakeLists.txt;53;add_test;/home/juliana/GitHub/Algoritmos-Avancados/Fluxos/CMakeLists.txt;0;")
subdirs("_deps/googletest-build")
