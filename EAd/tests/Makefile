EAD := ../../bin/ead

SETUP := /bin/bash EAdSetup.sh
CLEANUP := /bin/bash EAdCleanup.sh
TESTS := ead-get-noop.js ead-get-domain.js ead-get-casekeys.js ead-put-ctrl-time.js ead-set-1st-knob.js #ead-functest-gui.py

.PHONY:	all clean test

all:	test

clean:
	rm -rf *.h5 *.log node_modules venv

node_modules:	package.json
	npm install

venv:	requirements.txt
	@rm -rf venv
	python3 -m venv venv && venv/bin/pip3 install -r requirements.txt

pushtestdata:	ead-functest-combined.py venv
	venv/bin/python3 ead-functest-combined.py

test:	$(EAD) $(SETUP) $(CLEANUP) $(TESTS) node_modules venv
	$(SETUP) $(EAD)
	@-for f in $(TESTS) ; do echo "+++Testing $$f ..."; node $$f || exit $? ; echo "+++PASS"; echo; done
	$(CLEANUP)


# Original CMake test wiring:
#add_test(NAME EAdSetup COMMAND "/bin/bash" "${CMAKE_CURRENT_SOURCE_DIR}/tests/EAdSetup.sh" "${CMAKE_CURRENT_BINARY_DIR}/ead")
#add_test(NAME ead-get-noop COMMAND "${NODEJS_EXE}/node" "${CMAKE_CURRENT_SOURCE_DIR}/tests/ead-get-noop.js")
#add_test(NAME ead-get-domain COMMAND "${NODEJS_EXE}/node" "${CMAKE_CURRENT_SOURCE_DIR}/tests/ead-get-domain.js")
#add_test(NAME ead-get-casekeys COMMAND "${NODEJS_EXE}/node" "${CMAKE_CURRENT_SOURCE_DIR}/tests/ead-get-casekeys.js")
#add_test(NAME ead-put-ctrl-time COMMAND "${NODEJS_EXE}/node" "${CMAKE_CURRENT_SOURCE_DIR}/tests/ead-put-ctrl-time.js")
#add_test(NAME ead-set-1st-knob COMMAND "${NODEJS_EXE}/node" "${CMAKE_CURRENT_SOURCE_DIR}/tests/ead-set-1st-knob.js")
#add_test(NAME EAdCleanup COMMAND "/bin/bash" "${CMAKE_CURRENT_SOURCE_DIR}/tests/EAdCleanup.sh")
#
#set_tests_properties(ead-get-noop ead-get-domain ead-get-casekeys ead-put-ctrl-time ead-set-1st-knob PROPERTIES FIXTURES_REQUIRED EAd)
#set_tests_properties(EAdSetup        PROPERTIES FIXTURES_SETUP    EAd)
#set_tests_properties(EAdCleanup      PROPERTIES FIXTURES_CLEANUP  EAd)
