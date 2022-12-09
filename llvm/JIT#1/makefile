all:
	clang++ -g toy.cpp `llvm-config --cxxflags --ldflags --system-libs --libs core orcjit native` -rdynamic -Wl,--export-dynamic -O3 -o toy

tmp:
	clang++ -g tmp.cpp `llvm-config --cxxflags --ldflags --system-libs --libs core orcjit native` -rdynamic -Wl,--export-dynamic -O3 -o tmp