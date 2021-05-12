/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       CLI11_unittest_exe.cpp
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           4/27/21
  *Description:
**********************************************************************************/
#include "common/common.h"
#include <cassert>
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TextTestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <string>
#include <sstream>
#include "common/util/CLI11.hpp"

using namespace std;
using namespace CppUnit;


int main(int argc, char** argv) {
//    TextTestRunner testRunner;
//    testRunner.setOutputter(new CompilerOutputter(&testRunner.result(), std::cerr));
//    TestFactoryRegistry& registry = TestFactoryRegistry::getRegistry();
//    testRunner.addTest(registry.makeTest());
//    bool ok = testRunner.run("",false);
//    return ok ?0:1;

    CLI::App app{"xxx App description"}; // 软件描述出现在第一行打印
    app.footer("My footer"); // 最后一行打印
    app.get_formatter()->column_width(40); // 列的宽度
    app.require_subcommand(1); // 表示运行命令需要且仅需要一个子命令

    auto sub1 = app.add_subcommand("sub1", "subcommand1");
    auto sub2 = app.add_subcommand("sub2", "subcommand1");
    sub1->fallthrough(); // 当出现的参数子命令解析不了时,返回上一级尝试解析
    sub2->fallthrough();

    // 定义需要用到的参数
    string filename;
    int threads = 10;
    int mode = 0;
    vector<int> barcodes;
    bool reverse = false;
    string outPath;
    if (sub1)
    {
        // 第一个参数不加-, 表示位置参数,位置参数按出现的顺序来解析
        // 这里还检查了文件是否存在,已经是必须参数
        sub1->add_option("file", filename, "Position paramter")->check(CLI::ExistingFile)->required();

        // 检查参数必须大于0
        sub1->add_option("-n,-N", threads, "Set thread number")->check(CLI::PositiveNumber);
    }
    if (sub2)
    {
        // 设置范围
        sub2->add_option("-e,-E", mode, "Set mode")->check(CLI::Range(0,3));
        // 将数据放到vector中,并限制可接受的长度
        sub2->add_option("-b", barcodes, "Barcodes info:start,len,mismatch")->expected(3,6);
    }
    // 添加flag,有就是true
    app.add_flag("-r,-R", reverse, "Apply reverse");
    // 检查目录是否存在
    app.add_option("-o", outPath, "Output path")->check(CLI::ExistingDirectory);

    CLI11_PARSE(app, argc, argv);

    // 判断哪个子命令被使用
    if (sub1->parsed())
    {
        cout<<"Got sub1"<<endl;
        cout<<"filename:"<<filename<<endl;
        cout<<"threads:"<<threads<<endl;
    }
    else if (sub2->parsed())
    {
        cout<<"Got sub2"<<endl;
        cout<<"mode:"<<mode<<endl;
        cout<<"barcodes:";
        for (auto& b : barcodes)
            cout<<b<<" ";
        cout<<endl;
    }
    cout<<endl<<"Comman paras"<<endl;
    cout<<"reverse:"<<reverse<<endl;
    cout<<"outPath:"<<outPath<<endl;

    return 0;
}
