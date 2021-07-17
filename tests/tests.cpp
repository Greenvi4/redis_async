//
// Created by niko on 23.05.2021.
//
#include <gtest/gtest.h>
#include <log4cxx/helpers/properties.h>
#include <log4cxx/propertyconfigurator.h>

#include <redis_async/redis_async.hpp>

int main(int argc, char **argv) {
    log4cxx::helpers::Properties props;
    props.setProperty("log4j.rootLogger", "WARN, Global");
    props.setProperty("log4j.appender.Global", "org.apache.log4j.ConsoleAppender");
    props.setProperty("log4j.appender.Global.layout", "org.apache.log4j.PatternLayout");
    props.setProperty("log4j.appender.Global.layout.ConversionPattern",
                      "<%d{dd-MM-yy HH:mm::ss.SSS}> [%c %p]: %m%n");
    log4cxx::PropertyConfigurator::configure(props);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
