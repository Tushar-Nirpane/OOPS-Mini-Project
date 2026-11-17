#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>
#include <string>

/**
 * @file Exceptions.h
 * @brief Custom Exception Hierarchy for the Online Quiz System
 * Demonstrates: Inheritance from std::exception, polymorphism via virtual what(),
 * encapsulation of error descriptions, and custom exception handling.
 */

// Base Exception for all Quiz-related runtime errors
class QuizException : public std::exception {
protected:
    std::string message;

public:
    explicit QuizException(const std::string& msg) : message(msg) {}
    virtual ~QuizException() noexcept = default;

    const char* what() const noexcept override {
        return message.c_str();
    }
};

// Thrown when questions.txt or results.txt cannot be opened
class FileNotFoundException : public QuizException {
public:
    explicit FileNotFoundException(const std::string& filepath)
        : QuizException("File not found or cannot be opened: " + filepath) {}
};

// Thrown when question file format/syntax is corrupted or invalid
class InvalidQuestionFormatException : public QuizException {
public:
    explicit InvalidQuestionFormatException(const std::string& details)
        : QuizException("Invalid question file format: " + details) {}
};

// Thrown when quiz questions pool is empty
class EmptyQuizException : public QuizException {
public:
    EmptyQuizException()
        : QuizException("Quiz question bank is empty. No questions available to attempt.") {}
};

#endif // EXCEPTIONS_H

