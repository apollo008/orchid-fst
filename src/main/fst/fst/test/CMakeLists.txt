set(TOP_SRCDIR ${PROJECT_SOURCE_DIR})
set(TOP_BUILDDIR ${PROJECT_BINARY_DIR})
set(DOTEST_LOGGER_CONF ${TOP_BUILDDIR}/fst/test/logger.conf)
configure_file(test.h.in test.h)  # generate ${TOP_BUILDDIR}/xx/test/test.h
configure_file(logger.conf logger.conf)  # generate ${TOP_BUILDDIR}/xx/test/logger.conf
