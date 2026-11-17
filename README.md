# 🎓 Online Quiz System — C++ OOP Mini Project

A complete, production-clean **Online Quiz System** built for Second Year B.Tech Computer Engineering OOP Lab. Features a **C++17 OOP backend** with REST API and a **modern web frontend** — plus a terminal CLI fallback.

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue?logo=cplusplus)
![License](https://img.shields.io/badge/License-MIT-green)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey)

> 🚀 **Want a public web URL?** See **[DEPLOY.md](DEPLOY.md)** for one-click Render / Railway deployment (Docker + `$PORT` ready).

---

## ✨ Features

| Feature | Description |
|---------|-------------|
| 🔐 **Student Login** | Roll Number & Name entry to start the quiz |
| 📝 **Interactive Quiz** | MCQ & True/False questions with countdown timer, progress bar, and question navigator |
| 📊 **Instant Results** | Score, percentage, pass/fail badge, and detailed answer-by-answer review |
| 🏆 **Leaderboard** | All student scores ranked high-to-low with medal icons |
| ⚙️ **Admin Panel** | Add new questions directly from the web UI (saved to file) |
| 🌙 **Dark / Light Mode** | Theme toggle with localStorage persistence |
| 💻 **CLI Fallback** | Full terminal-based quiz using the same OOP backend classes |
| 🐳 **Docker Ready** | Multi-stage Dockerfile included for containerized deployment |

---

## 📂 Project Structure

```
├── backend/
│   ├── httplib.h              # Single-header HTTP server (cpp-httplib)
│   ├── Exceptions.h           # Custom exception hierarchy
│   ├── Question.h             # Class declarations (Question, MCQ, TF, Student)
│   ├── Question.cpp           # Class implementations
│   ├── QuizManager.h          # QuizManager + Template declarations
│   ├── QuizManager.cpp        # File I/O, evaluation, leaderboard logic
│   ├── server.cpp             # HTTP REST API + static file server
│   └── main_cli.cpp           # Terminal fallback (same OOP classes)
├── frontend/
│   ├── index.html             # SPA: Login, Quiz, Results, Leaderboard, Admin
│   ├── style.css              # Dark/Light theme card UI (no frameworks)
│   └── app.js                 # Fetch API, timer, state management
├── data/
│   ├── questions.txt          # 10 pre-loaded CS questions (pipe-delimited)
│   └── results.txt            # Student results (auto-appended)
├── Dockerfile                 # Multi-stage Docker build
└── .gitignore
```

---

## 🚀 Quick Start

### Prerequisites

- **g++** (GCC 8+) or **clang++** with C++17 support
- Any modern web browser

### Build & Run (Web UI)

```bash
# Clone the repository
git clone https://github.com/Tushar-Nirpane/OOPS-Mini-Project.git
cd OOPS-Mini-Project

# Compile the HTTP server
g++ -std=c++17 -O2 -pthread backend/Question.cpp backend/QuizManager.cpp backend/server.cpp -o quiz_server

# Run the server
./quiz_server

# Open in browser → http://localhost:8080
```

### Build & Run (Terminal CLI)

```bash
# Compile the CLI version
g++ -std=c++17 -O2 -pthread backend/Question.cpp backend/QuizManager.cpp backend/main_cli.cpp -o quiz_cli

# Run in terminal
./quiz_cli
```

### Windows (MinGW)

```cmd
g++ -std=c++17 -O2 -pthread -lws2_32 backend\Question.cpp backend\QuizManager.cpp backend\server.cpp -o quiz_server.exe
quiz_server.exe
```

> **Note:** The `-lws2_32` flag is required on Windows for socket support. The CLI build does not need it.

### Docker

```bash
docker build -t quiz-system .
docker run -d -p 8080:8080 quiz-system
# Open → http://localhost:8080
```

---

## 🌐 REST API Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| `GET` | `/api/questions` | Fetch all questions (answers hidden) |
| `POST` | `/api/submit` | Submit quiz answers & get evaluated result |
| `GET` | `/api/leaderboard` | Get student scores sorted high-to-low |
| `POST` | `/api/questions` | Admin: Add a new question to the bank |
| `GET` | `/api/stats` | System stats (static member counters) |

### Example: Submit Quiz

```bash
curl -X POST http://localhost:8080/api/submit \
  -H "Content-Type: application/json" \
  -d '{"rollNo":"21BCSE101","name":"Aarav","answers":{"1":"B","2":"A","8":"True"}}'
```

---

## 🎯 OOP Concepts Demonstrated

This project covers **every major OOP concept** required for the lab syllabus:

| # | OOP Concept | Where Demonstrated |
|---|-------------|-------------------|
| 1 | **Classes, Encapsulation, Constructors** (Default, Parameterized, Copy, Destructor), `this`, `static`, `inline`, `friend` | `Question.h` / `Question.cpp` — Classes: `Question`, `MCQQuestion`, `TrueFalseQuestion`, `Student` |
| 2 | **Inheritance & Base-Class Pointers** | `MCQQuestion : public Question`, `TrueFalseQuestion : public Question`, `std::vector<std::unique_ptr<Question>>` for polymorphic storage, `virtual ~Question()` |
| 3 | **Polymorphism & Operator Overloading** | Pure virtual functions: `evaluate()`, `toJSON()`, `serialize()`, `clone()`. Operators: `<<`, `>>` (file I/O), `+` (bonus marks & score merging) |
| 4 | **File Handling & Custom Exceptions** | `std::ifstream` / `std::ofstream` for `questions.txt` and `results.txt`. Custom exceptions: `FileNotFoundException`, `InvalidQuestionFormatException`, `EmptyQuizException` |
| 5 | **Templates & STL** | Template function: `calculatePercentage<T>()`. Template class: `Repository<T>`. STL: `std::vector`, `std::map`, `std::sort`, `std::shuffle` |

### CLI Viva Demo

The CLI includes a built-in **Option 4: "Viva OOP Concept Verification"** that prints live proof of every OOP concept:

```
$ ./quiz_cli
> Choose option 4

1. STATIC MEMBERS: Question::totalQuestionsCreated = 10
2. OPERATOR+ OVERLOADING: Student score 35 + 5 bonus = 40
3. TEMPLATE FUNCTION: calculatePercentage<double>(42, 50) = 84%
4. TEMPLATE CLASS: Repository<string> holds 4 OOP topics
5. RUNTIME POLYMORPHISM: Base pointer → MCQQuestion, evaluate("A") = Correct
```

---

## 📸 Screenshots

### Login Screen
> Enter Roll Number and Name to start the quiz.

### Quiz Screen
> Countdown timer, progress bar, question navigator pills, MCQ/TF options.

### Results Screen
> Score circle, percentage, pass/fail badge, detailed answer review.

### Leaderboard
> Medal icons (🥇🥈🥉), percentage badges, sorted high-to-low.

---

## 🛠️ Tech Stack

| Layer | Technology |
|-------|-----------|
| Backend | C++17, [cpp-httplib](https://github.com/yhirose/cpp-httplib) |
| Frontend | HTML5, CSS3, Vanilla JavaScript |
| Data Storage | File-based (pipe-delimited `.txt`) |
| Containerization | Docker (multi-stage build) |

---

## 👨‍💻 Author

**Tushar Nirpane**
- B.Tech Computer Engineering
- GitHub: [@Tushar-Nirpane](https://github.com/Tushar-Nirpane)

---

## 📄 License

This project is open source and available under the [MIT License](LICENSE).

