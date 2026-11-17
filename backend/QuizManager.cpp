#include "QuizManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>

// Helper to escape JSON
static std::string escapeJSON(const std::string& input) {
    std::ostringstream ss;
    for (char c : input) {
        switch (c) {
            case '"':  ss << "\\\""; break;
            case '\\': ss << "\\\\"; break;
            case '\b': ss << "\\b";  break;
            case '\f': ss << "\\f";  break;
            case '\n': ss << "\\n";  break;
            case '\r': ss << "\\r";  break;
            case '\t': ss << "\\t";  break;
            default:   ss << c;      break;
        }
    }
    return ss.str();
}

static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// ============================================================================
// EvaluationResult JSON serialization
// ============================================================================
std::string EvaluationResult::toJSON() const {
    std::ostringstream ss;
    ss << "{"
       << "\"status\":\"success\","
       << "\"student\":" << student.toJSON() << ","
       << "\"totalQuestions\":" << totalQuestions << ","
       << "\"correctCount\":" << correctCount << ","
       << "\"passed\":" << (passed ? "true" : "false") << ","
       << "\"reviews\":[";

    for (size_t i = 0; i < reviews.size(); ++i) {
        const auto& r = reviews[i];
        ss << "{"
           << "\"id\":" << r.id << ","
           << "\"text\":\"" << escapeJSON(r.text) << "\","
           << "\"type\":\"" << escapeJSON(r.type) << "\","
           << "\"userAnswer\":\"" << escapeJSON(r.userAnswer) << "\","
           << "\"correctAnswer\":\"" << escapeJSON(r.correctAnswer) << "\","
           << "\"isCorrect\":" << (r.isCorrect ? "true" : "false") << ","
           << "\"marksAwarded\":" << r.marksAwarded << ","
           << "\"maxMarks\":" << r.maxMarks
           << "}";
        if (i + 1 < reviews.size()) ss << ",";
    }
    ss << "]}";
    return ss.str();
}

// ============================================================================
// QUIZ MANAGER IMPLEMENTATION
// ============================================================================
QuizManager::QuizManager(const std::string& questionsPath, const std::string& resultsPath)
    : questionsFilePath(questionsPath), resultsFilePath(resultsPath) {
    try {
        loadQuestions();
    } catch (const std::exception& e) {
        std::cerr << "[QuizManager Warning] Could not load questions from "
                  << questionsFilePath << ": " << e.what() << "\n";
    }

    try {
        loadResults();
    } catch (const std::exception& e) {
        std::cerr << "[QuizManager Warning] Could not load results: " << e.what() << "\n";
    }
}

QuizManager::~QuizManager() {
    // Unique_ptrs in vector automatically destruct polymorphic Question objects
}

// Check and open file, trying fallback paths if run from different cwd
static bool openInputFile(std::ifstream& file, const std::string& primaryPath, std::string& resolvedPath) {
    file.open(primaryPath);
    if (file.is_open()) {
        resolvedPath = primaryPath;
        return true;
    }
    // Check fallback relative to project root or bin directory
    std::vector<std::string> fallbacks = {
        primaryPath,
        "../" + primaryPath,
        "../../" + primaryPath,
        "./data/" + primaryPath.substr(primaryPath.find_last_of("/\\") + 1)
    };
    for (const auto& path : fallbacks) {
        file.clear();
        file.open(path);
        if (file.is_open()) {
            resolvedPath = path;
            return true;
        }
    }
    return false;
}

// Load Questions from Pipe-Delimited File
void QuizManager::loadQuestions() {
    questions.clear();
    std::ifstream file;
    std::string resolvedPath;

    if (!openInputFile(file, questionsFilePath, resolvedPath)) {
        throw FileNotFoundException(questionsFilePath);
    }
    questionsFilePath = resolvedPath;

    std::string line;
    int lineNum = 0;
    while (std::getline(file, line)) {
        lineNum++;
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string type;
        if (!std::getline(ss, type, '|')) {
            throw InvalidQuestionFormatException("Line " + std::to_string(lineNum) + " missing type");
        }
        type = trim(type);

        if (type == "MCQ") {
            // MCQ|id|text|optA|optB|optC|optD|correctOpt|marks
            std::string idStr, text, optA, optB, optC, optD, correctOpt, marksStr;
            if (std::getline(ss, idStr, '|') &&
                std::getline(ss, text, '|') &&
                std::getline(ss, optA, '|') &&
                std::getline(ss, optB, '|') &&
                std::getline(ss, optC, '|') &&
                std::getline(ss, optD, '|') &&
                std::getline(ss, correctOpt, '|') &&
                std::getline(ss, marksStr)) {

                int id = std::stoi(trim(idStr));
                int marks = std::stoi(trim(marksStr));
                std::vector<std::string> opts = { trim(optA), trim(optB), trim(optC), trim(optD) };
                questions.push_back(std::make_unique<MCQQuestion>(id, trim(text), opts, trim(correctOpt), marks));
            } else {
                throw InvalidQuestionFormatException("Line " + std::to_string(lineNum) + " has invalid MCQ columns");
            }
        } else if (type == "TF") {
            // TF|id|text|correctAns|marks
            std::string idStr, text, correctAns, marksStr;
            if (std::getline(ss, idStr, '|') &&
                std::getline(ss, text, '|') &&
                std::getline(ss, correctAns, '|') &&
                std::getline(ss, marksStr)) {

                int id = std::stoi(trim(idStr));
                int marks = std::stoi(trim(marksStr));
                questions.push_back(std::make_unique<TrueFalseQuestion>(id, trim(text), trim(correctAns), marks));
            } else {
                throw InvalidQuestionFormatException("Line " + std::to_string(lineNum) + " has invalid TF columns");
            }
        }
    }
    file.close();
}

// Load Results from File using Operator>> Overload
void QuizManager::loadResults() {
    results.clear();
    std::ifstream file;
    std::string resolvedPath;

    if (!openInputFile(file, resultsFilePath, resolvedPath)) {
        // Results file might not exist yet; try creating an empty one
        std::ofstream create(resultsFilePath);
        create.close();
        return;
    }
    resultsFilePath = resolvedPath;

    Student s;
    while (file >> s) {
        results.push_back(s);
    }
    file.close();
}

// Save Student Result to File using Operator<< Overload
void QuizManager::saveResult(const Student& student) {
    results.push_back(student);
    Student::incrementQuizCount();

    std::ofstream file(resultsFilePath, std::ios::app);
    if (!file.is_open()) {
        // Try fallback directory
        file.open("../" + resultsFilePath, std::ios::app);
    }
    if (file.is_open()) {
        file << student << "\n";
        file.close();
    } else {
        std::cerr << "[QuizManager Error] Could not write to results file: " << resultsFilePath << "\n";
    }
}

// Append new Question (Admin functionality)
void QuizManager::addNewQuestion(std::unique_ptr<Question> newQ) {
    if (!newQ) return;

    // Append to file
    std::ofstream file(questionsFilePath, std::ios::app);
    if (!file.is_open()) {
        file.open("../" + questionsFilePath, std::ios::app);
    }
    if (file.is_open()) {
        file << *newQ << "\n";
        file.close();
    }

    // Add to memory
    questions.push_back(std::move(newQ));
}

// Get shuffled copy of questions for a randomized quiz experience
std::vector<std::unique_ptr<Question>> QuizManager::getShuffledQuestions() const {
    if (questions.empty()) {
        throw EmptyQuizException();
    }

    std::vector<std::unique_ptr<Question>> shuffled;
    shuffled.reserve(questions.size());
    for (const auto& q : questions) {
        shuffled.push_back(q->clone());
    }

    unsigned seed = static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count());
    std::shuffle(shuffled.begin(), shuffled.end(), std::default_random_engine(seed));
    return shuffled;
}

// Get JSON representation of questions for API
std::string QuizManager::getQuestionsJSON() const {
    if (questions.empty()) {
        return "[]";
    }

    std::ostringstream ss;
    ss << "[";
    for (size_t i = 0; i < questions.size(); ++i) {
        ss << questions[i]->toJSON();
        if (i + 1 < questions.size()) ss << ",";
    }
    ss << "]";
    return ss.str();
}

// Evaluate Student Submission
EvaluationResult QuizManager::evaluate(const std::string& rollNo,
                                      const std::string& name,
                                      const std::map<int, std::string>& answers) {
    if (questions.empty()) {
        throw EmptyQuizException();
    }

    int totalScore = 0;
    int totalPossibleMarks = 0;
    int correctCount = 0;
    std::vector<QuestionReview> reviews;

    for (const auto& q : questions) {
        totalPossibleMarks += q->getMarks();
        std::string userAns = "";
        auto it = answers.find(q->getId());
        if (it != answers.end()) {
            userAns = it->second;
        }

        bool isCorrect = q->evaluate(userAns);
        int marksAwarded = isCorrect ? q->getMarks() : 0;
        if (isCorrect) {
            correctCount++;
            totalScore += marksAwarded;
        }

        QuestionReview rev;
        rev.id = q->getId();
        rev.text = q->getText();
        rev.type = q->getType();
        rev.userAnswer = userAns.empty() ? "(Not Answered)" : userAns;
        rev.correctAnswer = q->getCorrectAnswer();
        rev.isCorrect = isCorrect;
        rev.marksAwarded = marksAwarded;
        rev.maxMarks = q->getMarks();
        reviews.push_back(rev);
    }

    // Use Template Function to calculate percentage!
    double pct = calculatePercentage<double>(static_cast<double>(totalScore),
                                             static_cast<double>(totalPossibleMarks));

    Student s(rollNo, name, totalScore, totalPossibleMarks, pct);
    saveResult(s);

    EvaluationResult result;
    result.student = s;
    result.totalQuestions = static_cast<int>(questions.size());
    result.correctCount = correctCount;
    result.passed = (pct >= 40.0); // 40% passing standard
    result.reviews = reviews;

    return result;
}

// Leaderboard sorted high-to-low using std::sort algorithm
std::vector<Student> QuizManager::getLeaderboard() const {
    std::vector<Student> sorted = results;
    std::sort(sorted.begin(), sorted.end(), [](const Student& a, const Student& b) {
        if (a.getScore() != b.getScore()) {
            return a.getScore() > b.getScore();
        }
        return a.getPercentage() > b.getPercentage();
    });
    return sorted;
}

// Get JSON Leaderboard for API
std::string QuizManager::getLeaderboardJSON() const {
    std::vector<Student> sorted = getLeaderboard();
    std::ostringstream ss;
    ss << "[";
    for (size_t i = 0; i < sorted.size(); ++i) {
        ss << sorted[i].toJSON();
        if (i + 1 < sorted.size()) ss << ",";
    }
    ss << "]";
    return ss.str();
}
