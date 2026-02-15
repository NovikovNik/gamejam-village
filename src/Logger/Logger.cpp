#include "Logger.h"
#include <iostream>
#include <ctime>
#include <ostream>
#include <termcolor/termcolor.hpp>
#include <sstream>

void Logger::Log(const std::string& message) {
    std::stringstream ss;
    ss << termcolor::green << "Log | " << GetTimestamp() << " - " << message << std::endl;
    LogEntry logEntry;
    logEntry.type = LOG_INFO;
    logEntry.message = ss.str();
    messages.push_back(logEntry);
    std::cout << termcolor::green << "Log | " << GetTimestamp() << " - " << message << std::endl;
}

void Logger::Err(const std::string& message) {
    std::stringstream ss;
    ss << termcolor::red << "Err | " << GetTimestamp() << " - " << message << std::endl;
    LogEntry logEntry;
    logEntry.type = LOG_ERROR;
    logEntry.message = ss.str();
    messages.push_back(logEntry);
    std::cout << termcolor::red << "Err | " << GetTimestamp() << " - " << message << std::endl;
}

void Logger::Warn(const std::string& message) {
    std::stringstream ss;
    ss << termcolor::yellow << "Warn | " << GetTimestamp() << " - " << message << std::endl;
    LogEntry logEntry;
    logEntry.type = LOG_WARNING;
    logEntry.message = ss.str();
    messages.push_back(logEntry);
    std::cout << termcolor::yellow << "Warn | " << GetTimestamp() << " - " << message << std::endl;
}

void Logger::Debug(const std::string& message) {
    std::stringstream ss;
    ss << termcolor::red << "Err | " << GetTimestamp() << " - " << message << std::endl;
    LogEntry logEntry;
    logEntry.type = LOG_ERROR;
    logEntry.message = ss.str();
    messages.push_back(logEntry);
    std::cout << termcolor::yellow << "Debug | " << GetTimestamp() << " - " << message << std::endl;
}

std::string Logger::GetTimestamp() {
    time_t timestamp = time(NULL);
    struct tm datetime = *localtime(&timestamp);
    char output[60];
    strftime(output, sizeof(output), "%Y-%m-%d %H:%M:%S", &datetime);
    return std::string(output);
}
