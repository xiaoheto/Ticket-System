//
// Created by 43741 on 2025/5/11.
//

#ifndef MYERROR_H
#define MYERROR_H
#include <string>
#include <exception>
#include "MyError.h";

class Error : public std::exception {
private:
    std::string err;

public:
    Error() noexcept : err("Unknown Error") {}
    explicit Error(const std::string& message) noexcept : err(message) {}
    explicit Error(const char* message) noexcept : err(message ? message : "Unknown Error") {}
    ~Error() noexcept override = default;

    const char* what() const noexcept override {
        return err.c_str();
    }
};

#endif //MYERROR_H
