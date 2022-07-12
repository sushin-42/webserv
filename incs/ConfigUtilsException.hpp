#ifndef CONFIGUTILSEXCEPTION_HPP
#define CONFIGUTILSEXCEPTION_HPP

#include <exception>
#include <string>
#include "color.hpp"
#include "LocationConfig.hpp"
#include "ServerConfig.hpp"
using namespace std;
//=====================================util exception========================================
class isPathFail : public exception
{
private:
    string msg;

public:
    explicit isPathFail() : msg(RED("isPathFail")) {}
    explicit isPathFail(const string &m) : msg(m) {}
    virtual ~isPathFail() throw(){};
    virtual const char *what() const throw() { return msg.c_str(); }
};

class convertStringToSsize_TFail : public exception
{
private:
    string msg;

public:
    explicit convertStringToSsize_TFail() : msg(RED("convertStringToSsize_TFail")) {}
    explicit convertStringToSsize_TFail(const string &m) : msg(m) {}
    virtual ~convertStringToSsize_TFail() throw(){};
    virtual const char *what() const throw() { return msg.c_str(); }
};

class convertStringToStateCodeFail : public exception
{
private:
    string msg;

public:
    explicit convertStringToStateCodeFail() : msg(RED("convertStringToStateCodeFail")) {}
    explicit convertStringToStateCodeFail(const string &m) : msg(m) {}
    virtual ~convertStringToStateCodeFail() throw(){};
    virtual const char *what() const throw() { return msg.c_str(); }
};

class convertStringToIPFail : public exception
{
private:
    string msg;

public:
    explicit convertStringToIPFail() : msg(RED("convertStringToIPFail")) {}
    explicit convertStringToIPFail(const string &m) : msg(m) {}
    virtual ~convertStringToIPFail() throw(){};
    virtual const char *what() const throw() { return msg.c_str(); }
};

class convertStringToPortFail : public exception
{
private:
    string msg;

public:
    explicit convertStringToPortFail() : msg(RED("convertStringToPortFail")) {}
    explicit convertStringToPortFail(const string &m) : msg(m) {}
    virtual ~convertStringToPortFail() throw(){};
    virtual const char *what() const throw() { return msg.c_str(); }
};

class splitStringToTypeFail : public exception
{
private:
    string msg;

public:
    explicit splitStringToTypeFail() : msg(RED("splitStringToTypeFail")) {}
    explicit splitStringToTypeFail(const string &m) : msg(m) {}
    virtual ~splitStringToTypeFail() throw(){};
    virtual const char *what() const throw() { return msg.c_str(); }
};

class convertStringToTimeFail : public exception
{
private:
    string msg;

public:
    explicit convertStringToTimeFail() : msg(RED("convertStringToTimeFail")) {}
    explicit convertStringToTimeFail(const string &m) : msg(m) {}
    virtual ~convertStringToTimeFail() throw(){};
    virtual const char *what() const throw() { return msg.c_str(); }
};

class convertStringToByteFail : public exception
{
private:
    string msg;

public:
    explicit convertStringToByteFail() : msg(RED("convertStringToByteFail")) {}
    explicit convertStringToByteFail(const string &m) : msg(m) {}
    virtual ~convertStringToByteFail() throw(){};
    virtual const char *what() const throw() { return msg.c_str(); }
};

class parseLimitExceptMethodFail : public exception
{
private:
    string msg;

public:
    explicit parseLimitExceptMethodFail() : msg(RED("parseLimitExceptMethodFail")) {}
    explicit parseLimitExceptMethodFail(const string &m) : msg(m) {}
    virtual ~parseLimitExceptMethodFail() throw(){};
    virtual const char *what() const throw() { return msg.c_str(); }
};

class parseReadOpenFileFail : public exception
{
private:
    string msg;

public:
    explicit parseReadOpenFileFail() : msg(RED("parseReadOpenFileFail")) {}
    explicit parseReadOpenFileFail(const string &m) : msg(m) {}
    virtual ~parseReadOpenFileFail() throw(){};
    virtual const char *what() const throw() { return msg.c_str(); }
};
//=====================================parse_util exception========================================
#endif