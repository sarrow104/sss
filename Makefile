.PHONY: all release debug clean clean-debug clean-release doxygen ctest
all: release

release:
	@mkdir -p ../../Release
	cd ../../Release && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DCMAKE_BUILD_TYPE=Release ../include/sss && make

debug:
	@mkdir -p ../../Debug
	cd ../../Debug && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DCMAKE_BUILD_TYPE=Debug ../include/sss && make

doxygen:
	@doxygen sss.conf

.PHONY: meld

meld:
	@meld . /home/sarrow/T470p_home/sarrow/extra/sss/include/sss

clean: clean-debug clean-release

clean-debug:
	if [ -d ../../Debug ]; then cd ../../Debug && make clean && rm -rf ./* ; fi

clean-release:
	if [ -d ../../Release ]; then cd ../../Release && make clean && rm -rf ./* ; fi
	#cd ../../Release && make clean && rm -rf ./*

ctest:
	(cd ../../Release/tests ; ctest --output-on-failure --timeout 10 ; cd -)
#--test-timeout 10

