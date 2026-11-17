#ifndef QUIZ_MANAGER_H
#define QUIZ_MANAGER_H

#include "Question.h"
#include "Exceptions.h"
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <algorithm>
#include <random>
#include <fstream>
#include <sstream>

/**
 * @file QuizManager.h
 * @brief Coordinates Quiz operations, File I/O, Ranking, and Evaluation.
 * Demonstrates:
 * 1. Template Functions & Generic Template Classes
 * 2. STL Containers (std::vector, std::map) & Algorithms (std::sort, std::shuffle)
 * 3. File Streams (std::ifstream, std::ofstream)
 * 4. Exception Handling (throw / catch)
 * 5. Polymorphic Base Class Pointer management (std::unique_ptr<Question>)
 */

// ============================================================================
// TEMPLATE 1: Generic Percentage Calculator Function
// ============================================================================
template <typename T>
T calculatePercentage(T obtained, T total) {
    if (total == static_cast<T>(0)) {
        return static_cast<T>(0);
    }
    return (obtained * static_cast<T>(100)) / total;
}

// ============================================================================
// TEMPLATE 2: Generic Repository Class (Template Class demonstration)
// ============================================================================
template <typename T>
class Repository {
private:
    std::vector<T> items;

public:
    void add(const T& item) {
        items.push_back(item);
    }

    const std::vector<T>& getAll() const {
        return items;
    }

    size_t count() const {
        return items.size();
    }

    void clear() {
        items.clear();
    }
};

// ============================================================================
// Review & Result Structures
// ============================================================================
struct QuestionReview {
    int id;
    std::string text;
    std::string type;
    std::string userAnswer;
    std::string correctAnswer;
    bool isCorrect;
    int marksAwarded;
    int maxMarks;
};

struct EvaluationResult {
    Student student;
    int totalQuestions;
    int correctCount;
    bool passed;
    std::vector<QuestionReview> reviews;

    std::string toJSON() const;
};

// ============================================================================
// QUIZ MANAGER CLASS
// ============================================================================
class QuizManager {
private:
    // Polymorphic collection using base class smart pointers
    std::vector<std::unique_ptr<Question>> questions;

    // Student results collection
    std::vector<Student> results;

    std::string questionsFilePath;
    std::string resultsFilePath;

public:
    QuizManager(const std::string& questionsPath = "data/questions.txt",
                const std::string& resultsPath = "data/results.txt");
    ~QuizManager();

    // Disable copy constructor and assignment operator to manage unique resources
    QuizManager(const QuizManager&) = delete;
    QuizManager& operator=(const QuizManager&) = delete;

    // File I/O
    void loadQuestions();
    void loadResults();
    void saveResult(const Student& student);
    void addNewQuestion(std::unique_ptr<Question> newQ);

    // Quiz Operations
    std::vector<std::unique_ptr<Question>> getShuffledQuestions() const;
    std::string getQuestionsJSON() const;

    // Evaluation
    EvaluationResult evaluate(const std::string& rollNo,
                             const std::string& name,
                             const std::map<int, std::string>& answers);

    // Leaderboard
    std::vector<Student> getLeaderboard() const;
    std::string getLeaderboardJSON() const;

    // Admin & Metrics
    inline size_t getQuestionCount() const { return questions.size(); }
    inline size_t getResultCount() const { return results.size(); }

    const std::vector<std::unique_ptr<Question>>& getAllQuestions() const {
        return questions;
    }
};

#endif // QUIZ_MANAGER_H
