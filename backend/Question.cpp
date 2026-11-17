#include "Question.h"
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>

// ============================================================================
// STATIC MEMBER DEFINITIONS
// ============================================================================
int Question::totalQuestionsCreated = 0;
int Student::totalQuizzesAttempted = 0;

// Helper: Escape string for JSON
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

// Helper: Trim whitespace
static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// ============================================================================
// QUESTION BASE CLASS IMPLEMENTATION
// ============================================================================

// Default Constructor
Question::Question() : id(0), text(""), type("UNKNOWN"), marks(0) {
    Question::totalQuestionsCreated++;
}

// Parameterized Constructor using 'this' pointer
Question::Question(int id, const std::string& text, const std::string& type, int marks) {
    this->id = id;
    this->text = text;
    this->type = type;
    this->marks = marks;
    Question::totalQuestionsCreated++;
}

// Copy Constructor
Question::Question(const Question& other) {
    this->id = other.id;
    this->text = other.text;
    this->type = other.type;
    this->marks = other.marks;
    Question::totalQuestionsCreated++;
}

// Virtual Destructor
Question::~Question() {
    // Virtual destructor guarantees derived class destructors are invoked
}

// Friend Function: Stream insertion operator for Question
std::ostream& operator<<(std::ostream& os, const Question& q) {
    os << q.serialize();
    return os;
}

// Friend Function: Meta summary
void displayQuestionMeta(const Question& q) {
    std::cout << "[Question #" << q.id << " (" << q.type << ") Marks: " << q.marks << "]\n";
}

// ============================================================================
// MCQQuestion IMPLEMENTATION
// ============================================================================

// Default Constructor
MCQQuestion::MCQQuestion() : Question(0, "", "MCQ", 0), correctOption("A") {}

// Parameterized Constructor
MCQQuestion::MCQQuestion(int id, const std::string& text,
                         const std::vector<std::string>& opts,
                         const std::string& correctOpt, int marks)
    : Question(id, text, "MCQ", marks), options(opts), correctOption(correctOpt) {}

// Copy Constructor
MCQQuestion::MCQQuestion(const MCQQuestion& other)
    : Question(other), options(other.options), correctOption(other.correctOption) {}

// Destructor
MCQQuestion::~MCQQuestion() {}

// Evaluate Answer
bool MCQQuestion::evaluate(const std::string& answer) const {
    std::string clean = trim(answer);
    if (clean.empty() || this->correctOption.empty()) return false;
    return std::toupper(static_cast<unsigned char>(clean[0])) ==
           std::toupper(static_cast<unsigned char>(this->correctOption[0]));
}

// JSON representation (excludes correct answer to prevent student cheating on client)
std::string MCQQuestion::toJSON() const {
    std::ostringstream ss;
    ss << "{"
       << "\"id\":" << this->id << ","
       << "\"text\":\"" << escapeJSON(this->text) << "\","
       << "\"type\":\"MCQ\","
       << "\"marks\":" << this->marks << ","
       << "\"options\":[";
    for (size_t i = 0; i < this->options.size(); ++i) {
        ss << "\"" << escapeJSON(this->options[i]) << "\"";
        if (i + 1 < this->options.size()) ss << ",";
    }
    ss << "]}";
    return ss.str();
}

// Pipe-delimited serialization for file storage
std::string MCQQuestion::serialize() const {
    std::ostringstream ss;
    ss << "MCQ|" << this->id << "|" << this->text;
    for (size_t i = 0; i < 4; ++i) {
        if (i < this->options.size()) {
            ss << "|" << this->options[i];
        } else {
            ss << "|Option";
        }
    }
    ss << "|" << this->correctOption << "|" << this->marks;
    return ss.str();
}

std::string MCQQuestion::getCorrectAnswer() const {
    return this->correctOption;
}

std::unique_ptr<Question> MCQQuestion::clone() const {
    return std::make_unique<MCQQuestion>(*this);
}

// ============================================================================
// TrueFalseQuestion IMPLEMENTATION
// ============================================================================

// Default Constructor
TrueFalseQuestion::TrueFalseQuestion() : Question(0, "", "TF", 0), correctAnswer("True") {}

// Parameterized Constructor
TrueFalseQuestion::TrueFalseQuestion(int id, const std::string& text,
                                     const std::string& correctAns, int marks)
    : Question(id, text, "TF", marks), correctAnswer(correctAns) {}

// Copy Constructor
TrueFalseQuestion::TrueFalseQuestion(const TrueFalseQuestion& other)
    : Question(other), correctAnswer(other.correctAnswer) {}

// Destructor
TrueFalseQuestion::~TrueFalseQuestion() {}

// Evaluate Answer
bool TrueFalseQuestion::evaluate(const std::string& answer) const {
    std::string cleanAns = trim(answer);
    std::string cleanExpected = trim(this->correctAnswer);
    std::transform(cleanAns.begin(), cleanAns.end(), cleanAns.begin(), ::tolower);
    std::transform(cleanExpected.begin(), cleanExpected.end(), cleanExpected.begin(), ::tolower);
    return cleanAns == cleanExpected;
}

// JSON representation
std::string TrueFalseQuestion::toJSON() const {
    std::ostringstream ss;
    ss << "{"
       << "\"id\":" << this->id << ","
       << "\"text\":\"" << escapeJSON(this->text) << "\","
       << "\"type\":\"TF\","
       << "\"marks\":" << this->marks << ","
       << "\"options\":[\"True\",\"False\"]"
       << "}";
    return ss.str();
}

// Pipe-delimited serialization
std::string TrueFalseQuestion::serialize() const {
    std::ostringstream ss;
    ss << "TF|" << this->id << "|" << this->text << "|"
       << this->correctAnswer << "|" << this->marks;
    return ss.str();
}

std::string TrueFalseQuestion::getCorrectAnswer() const {
    return this->correctAnswer;
}

std::unique_ptr<Question> TrueFalseQuestion::clone() const {
    return std::make_unique<TrueFalseQuestion>(*this);
}

// ============================================================================
// STUDENT CLASS IMPLEMENTATION
// ============================================================================

// Default Constructor
Student::Student()
    : rollNo(""), name(""), score(0), totalMarks(0), percentage(0.0), timestamp("") {}

// Parameterized Constructor
Student::Student(const std::string& rollNo, const std::string& name,
                 int score, int totalMarks, double percentage,
                 const std::string& timestamp) {
    this->rollNo = rollNo;
    this->name = name;
    this->score = score;
    this->totalMarks = totalMarks;
    this->percentage = percentage;

    if (timestamp.empty()) {
        std::time_t now = std::time(nullptr);
        char buf[64];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        this->timestamp = std::string(buf);
    } else {
        this->timestamp = timestamp;
    }
}

// Copy Constructor
Student::Student(const Student& other) {
    this->rollNo = other.rollNo;
    this->name = other.name;
    this->score = other.score;
    this->totalMarks = other.totalMarks;
    this->percentage = other.percentage;
    this->timestamp = other.timestamp;
}

// Destructor
Student::~Student() {}

// Operator + Overload (Bonus Marks)
Student Student::operator+(int bonusMarks) const {
    int newScore = this->score + bonusMarks;
    if (newScore > this->totalMarks) newScore = this->totalMarks;
    double newPct = (this->totalMarks > 0) ? (static_cast<double>(newScore) / this->totalMarks) * 100.0 : 0.0;
    return Student(this->rollNo, this->name, newScore, this->totalMarks, newPct, this->timestamp);
}

// Operator + Overload (Merge two test modules for the same student)
Student Student::operator+(const Student& other) const {
    int combinedScore = this->score + other.score;
    int combinedTotal = this->totalMarks + other.totalMarks;
    double combinedPct = (combinedTotal > 0) ? (static_cast<double>(combinedScore) / combinedTotal) * 100.0 : 0.0;
    return Student(this->rollNo, this->name, combinedScore, combinedTotal, combinedPct, this->timestamp);
}

// Stream Output Operator Overloading (File writing / display)
std::ostream& operator<<(std::ostream& os, const Student& s) {
    os << s.rollNo << "|"
       << s.name << "|"
       << s.score << "|"
       << s.totalMarks << "|"
       << std::fixed << std::setprecision(2) << s.percentage << "|"
       << s.timestamp;
    return os;
}

// Stream Input Operator Overloading (File reading)
std::istream& operator>>(std::istream& is, Student& s) {
    std::string line;
    if (std::getline(is, line)) {
        if (line.empty()) return is;
        std::stringstream ss(line);
        std::string roll, name, scoreStr, totalStr, pctStr, timeStr;

        if (std::getline(ss, roll, '|') &&
            std::getline(ss, name, '|') &&
            std::getline(ss, scoreStr, '|') &&
            std::getline(ss, totalStr, '|') &&
            std::getline(ss, pctStr, '|') &&
            std::getline(ss, timeStr)) {
            try {
                s.rollNo = trim(roll);
                s.name = trim(name);
                s.score = std::stoi(trim(scoreStr));
                s.totalMarks = std::stoi(trim(totalStr));
                s.percentage = std::stod(trim(pctStr));
                s.timestamp = trim(timeStr);
            } catch (...) {
                // If corrupted, leave stream in fail state
                is.setstate(std::ios::failbit);
            }
        } else {
            is.setstate(std::ios::failbit);
        }
    }
    return is;
}

// Friend Function
void printStudentSummary(const Student& s) {
    std::cout << "Student [Roll: " << s.rollNo << ", Name: " << s.name
              << ", Score: " << s.score << "/" << s.totalMarks
              << " (" << s.percentage << "%)]\n";
}

// JSON serialization for frontend API
std::string Student::toJSON() const {
    std::ostringstream ss;
    ss << "{"
       << "\"rollNo\":\"" << escapeJSON(this->rollNo) << "\","
       << "\"name\":\"" << escapeJSON(this->name) << "\","
       << "\"score\":" << this->score << ","
       << "\"totalMarks\":" << this->totalMarks << ","
       << "\"percentage\":" << std::fixed << std::setprecision(2) << this->percentage << ","
       << "\"timestamp\":\"" << escapeJSON(this->timestamp) << "\""
       << "}";
    return ss.str();
}

