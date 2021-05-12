/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       dotest.cpp
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           2/23/21
  *Description:
**********************************************************************************/
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestListener.h>
#include <cppunit/TestResult.h>
#include <cppunit/CompilerOutputter.h>
#include <tulip/TLogDefine.h>
#include <string>
#include <cstring>
#include "fst/test/test.h"
#include "common/util/hash_util.h"

using namespace std;
using namespace CppUnit;
COMMON_USE_NAMESPACE;


class LogFlusherListener : public CppUnit::TestListener
{
    virtual void addFailure (const TestFailure &failure)
    {
        tulip::TLogger::s_GetRootLogger()->s_FlushAll();
        usleep(200000);
    }
};

void recursivePrintTestName(Test *test, size_t indentLevel = 0)
{
    if (test == NULL)
        return;
    for (size_t i = 0; i < indentLevel; i++) {
        std::cout << "  ";
    }
    std::cout << test->getName() << std::endl;
    int testCount = test->getChildTestCount();
    for (int i = 0; i < testCount; ++i) {
        Test *childTest = test->getChildTestAt(i);
        recursivePrintTestName(childTest, indentLevel+1);
    }
}

static bool matchTestNameFilter(const std::string &testName, int argc, char **argv)
{
    for (int iArg = 1; iArg < argc; ++iArg) {
        const char *testNamePattern = argv[iArg];
        if (testName.find(testNamePattern) != std::string::npos) {
            return true;
        }
    }
    return false;
}

void recursiveGetTests(Test *test, std::vector<Test *> &tests, int argc, char **argv)
{
    if (test == NULL)
        return;
    bool matched = matchTestNameFilter(test->getName(), argc, argv);
    if (matched) {
        tests.push_back(test);
        return;
    }
    int testCount = test->getChildTestCount();
    for (int i = 0; i < testCount; ++i) {
        Test *childTest = test->getChildTestAt(i);
        recursiveGetTests(childTest, tests, argc, argv);
    }
}

int main( int argc, char **argv)
{
    Random<uint32_t>::seedDefault();
    Random<uint64_t>::seedDefault();

    TLOG_CONFIG(DOTEST_LOGGER_CONF);

    LogFlusherListener logFlusherListener;
    TextUi::TestRunner runner;
    runner.setOutputter(new CompilerOutputter(&runner.result(), std::cerr));
    runner.eventManager().addListener(&logFlusherListener);
    TestFactoryRegistry &registry = TestFactoryRegistry::getRegistry();

    if (argc == 1) { // run all tests by default
        runner.addTest( registry.makeTest() );
    }
    else if (argc == 2 && 0 == strcmp(argv[1], "-l")) { // list all tests and exit
        Test *test = registry.makeTest();
        recursivePrintTestName(test);
        delete test;
        TLOG_LOG_FLUSH();
        TLOG_LOG_SHUTDOWN();
        return 0;
    }
    else { // filter tests by names
        Test *test = registry.makeTest();
        runner.addTest(test);
        std::vector<Test*> tests;
        recursiveGetTests(test, tests, argc, argv);
        bool ok = true;
        for (std::vector<Test*>::const_iterator it = tests.begin(); it != tests.end(); ++it) {
            Test *childTest = *it;;
            std::cout << "Will run test " << childTest->getName() << std::endl;
            ok = runner.run(childTest->getName(), false);
            if (!ok) {
                break;
            }
        }
        TLOG_LOG_FLUSH();
        TLOG_LOG_SHUTDOWN();
        return ok ? 0 : 1;
    }

    bool ok = runner.run("", false);
    TLOG_LOG_FLUSH();
    TLOG_LOG_SHUTDOWN();
    return ok ? 0 : 1;
}
