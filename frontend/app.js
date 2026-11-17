/**
 * @file app.js
 * Online Quiz System — Frontend Application (Vanilla JS)
 * Communicates with the C++ OOP Backend via Fetch API.
 */

// ============================================================================
// CONFIGURATION
// ============================================================================
const API_BASE = window.location.origin; // Same-origin when served by C++ backend
const QUIZ_TIME_SECONDS = 10 * 60;      // 10-minute timer

// ============================================================================
// APPLICATION STATE
// ============================================================================
const state = {
    student: { rollNo: '', name: '' },
    questions: [],
    answers: {},        // { questionId: "selectedOption" }
    currentIndex: 0,
    timerSeconds: QUIZ_TIME_SECONDS,
    timerInterval: null,
    result: null,
};

// ============================================================================
// DOM ELEMENT REFERENCES
// ============================================================================
const $ = (sel) => document.querySelector(sel);
const $$ = (sel) => document.querySelectorAll(sel);

// Screens
const screens = {
    login: $('#screen-login'),
    quiz: $('#screen-quiz'),
    result: $('#screen-result'),
    leaderboard: $('#screen-leaderboard'),
    admin: $('#screen-admin'),
};

// ============================================================================
// NAVIGATION
// ============================================================================
function showScreen(name) {
    Object.values(screens).forEach(s => s.classList.remove('active'));
    if (screens[name]) screens[name].classList.add('active');

    // Update tab buttons
    $$('.tab-btn').forEach(btn => {
        btn.classList.toggle('active', btn.dataset.screen === name);
    });
}

// Tab navigation
$$('.tab-btn').forEach(btn => {
    btn.addEventListener('click', () => {
        const target = btn.dataset.screen;
        if (target === 'leaderboard') loadLeaderboard();
        showScreen(target);
    });
});

// ============================================================================
// THEME TOGGLE
// ============================================================================
const themeToggle = $('#themeToggle');
const savedTheme = localStorage.getItem('quizTheme') || 'light';
document.documentElement.setAttribute('data-theme', savedTheme);
themeToggle.textContent = savedTheme === 'dark' ? '☀️' : '🌙';

themeToggle.addEventListener('click', () => {
    const current = document.documentElement.getAttribute('data-theme');
    const next = current === 'dark' ? 'light' : 'dark';
    document.documentElement.setAttribute('data-theme', next);
    localStorage.setItem('quizTheme', next);
    themeToggle.textContent = next === 'dark' ? '☀️' : '🌙';
});

// ============================================================================
// TOAST NOTIFICATIONS
// ============================================================================
function showToast(message, duration = 3000) {
    const toast = $('#toast');
    toast.textContent = message;
    toast.classList.remove('hidden');
    toast.classList.add('show');
    setTimeout(() => {
        toast.classList.remove('show');
        setTimeout(() => toast.classList.add('hidden'), 400);
    }, duration);
}

// ============================================================================
// 1. LOGIN FORM
// ============================================================================
$('#loginForm').addEventListener('submit', async (e) => {
    e.preventDefault();
    const rollNo = $('#rollNo').value.trim();
    const name = $('#studentName').value.trim();

    if (!rollNo || !name) {
        showToast('Please enter both Roll Number and Name');
        return;
    }

    state.student = { rollNo, name };
    state.answers = {};
    state.currentIndex = 0;
    state.timerSeconds = QUIZ_TIME_SECONDS;

    try {
        await loadQuestions();
        if (state.questions.length === 0) {
            showToast('No questions available. Contact admin.');
            return;
        }
        startQuiz();
    } catch (err) {
        showToast('Error loading questions: ' + err.message);
    }
});

// ============================================================================
// 2. QUIZ ENGINE
// ============================================================================
async function loadQuestions() {
    const res = await fetch(`${API_BASE}/api/questions`);
    if (!res.ok) throw new Error('Server returned ' + res.status);
    state.questions = await res.json();
}

function startQuiz() {
    showScreen('quiz');
    $('#quizStudentLabel').textContent = `${state.student.name} (${state.student.rollNo})`;
    buildNavigator();
    renderQuestion();
    startTimer();
}

// --- QUESTION NAVIGATOR ---
function buildNavigator() {
    const nav = $('#questionNav');
    nav.innerHTML = '';
    state.questions.forEach((q, i) => {
        const pill = document.createElement('button');
        pill.className = 'q-nav-pill';
        pill.textContent = i + 1;
        pill.addEventListener('click', () => {
            state.currentIndex = i;
            renderQuestion();
        });
        nav.appendChild(pill);
    });
}

function updateNavigator() {
    const pills = $$('.q-nav-pill');
    pills.forEach((pill, i) => {
        pill.classList.remove('current', 'answered');
        if (i === state.currentIndex) {
            pill.classList.add('current');
        } else if (state.answers[state.questions[i].id] !== undefined) {
            pill.classList.add('answered');
        }
    });
}

// --- RENDER CURRENT QUESTION ---
function renderQuestion() {
    const q = state.questions[state.currentIndex];
    const total = state.questions.length;
    const idx = state.currentIndex;

    // Counter and progress
    $('#questionCounter').textContent = `Q ${idx + 1} / ${total}`;
    $('#progressBar').style.width = `${((idx + 1) / total) * 100}%`;

    // Type badge
    $('#questionTypeBadge').textContent = q.type === 'MCQ' ? 'MCQ' : 'True / False';

    // Question text
    $('#questionText').textContent = q.text;

    // Options
    const container = $('#optionsContainer');
    container.innerHTML = '';

    const letters = ['A', 'B', 'C', 'D'];
    q.options.forEach((opt, i) => {
        const btn = document.createElement('button');
        btn.className = 'option-btn';
        const letter = q.type === 'TF' ? (i === 0 ? 'T' : 'F') : letters[i];
        const value = q.type === 'TF' ? opt : letters[i];

        btn.innerHTML = `<span class="option-letter">${letter}</span><span>${opt}</span>`;

        // Check if already answered
        if (state.answers[q.id] === value) {
            btn.classList.add('selected');
        }

        btn.addEventListener('click', () => {
            state.answers[q.id] = value;
            container.querySelectorAll('.option-btn').forEach(b => b.classList.remove('selected'));
            btn.classList.add('selected');
            updateNavigator();
        });

        container.appendChild(btn);
    });

    // Navigation buttons
    $('#btnPrev').disabled = idx === 0;
    $('#btnNext').classList.toggle('hidden', idx === total - 1);
    $('#btnSubmit').classList.toggle('hidden', idx !== total - 1);

    updateNavigator();
}

// --- NAV BUTTONS ---
$('#btnPrev').addEventListener('click', () => {
    if (state.currentIndex > 0) {
        state.currentIndex--;
        renderQuestion();
    }
});

$('#btnNext').addEventListener('click', () => {
    if (state.currentIndex < state.questions.length - 1) {
        state.currentIndex++;
        renderQuestion();
    }
});

$('#btnSkip').addEventListener('click', () => {
    if (state.currentIndex < state.questions.length - 1) {
        state.currentIndex++;
        renderQuestion();
    } else {
        showToast('Last question reached. Submit when ready!');
    }
});

$('#btnSubmit').addEventListener('click', () => submitQuiz());

// --- TIMER ---
function startTimer() {
    clearInterval(state.timerInterval);
    state.timerSeconds = QUIZ_TIME_SECONDS;
    updateTimerDisplay();

    state.timerInterval = setInterval(() => {
        state.timerSeconds--;
        updateTimerDisplay();

        const timerEl = $('#timer');
        if (state.timerSeconds <= 60) {
            timerEl.className = 'timer danger';
        } else if (state.timerSeconds <= 120) {
            timerEl.className = 'timer warning';
        } else {
            timerEl.className = 'timer';
        }

        if (state.timerSeconds <= 0) {
            clearInterval(state.timerInterval);
            showToast('⏰ Time is up! Auto-submitting...');
            setTimeout(() => submitQuiz(), 500);
        }
    }, 1000);
}

function updateTimerDisplay() {
    const mins = Math.floor(state.timerSeconds / 60);
    const secs = state.timerSeconds % 60;
    $('#timerDisplay').textContent = `${String(mins).padStart(2, '0')}:${String(secs).padStart(2, '0')}`;
}

// ============================================================================
// 3. SUBMIT QUIZ
// ============================================================================
async function submitQuiz() {
    clearInterval(state.timerInterval);

    // Build answers map as { "id": "answer" }
    const answersMap = {};
    for (const [key, val] of Object.entries(state.answers)) {
        answersMap[String(key)] = val;
    }

    const payload = {
        rollNo: state.student.rollNo,
        name: state.student.name,
        answers: answersMap,
    };

    try {
        const res = await fetch(`${API_BASE}/api/submit`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(payload),
        });

        if (!res.ok) {
            const err = await res.json();
            showToast('Error: ' + (err.message || 'Submission failed'));
            return;
        }

        state.result = await res.json();
        renderResult();
    } catch (err) {
        showToast('Network error: ' + err.message);
    }
}

// ============================================================================
// 4. RESULT SCREEN
// ============================================================================
function renderResult() {
    showScreen('result');
    const r = state.result;

    // Badge
    const badge = $('#resultBadge');
    badge.textContent = r.passed ? '✅ PASSED' : '❌ FAILED';
    badge.className = 'result-badge ' + (r.passed ? 'passed' : 'failed');

    // Title
    $('#resultTitle').textContent = `Results for ${r.student.name}`;

    // Score circle
    $('#scoreNumber').textContent = `${r.student.score}/${r.student.totalMarks}`;

    // Stats grid
    const statsHtml = `
        <div class="stat-item">
            <div class="stat-value">${r.totalQuestions}</div>
            <div class="stat-label">Total Questions</div>
        </div>
        <div class="stat-item">
            <div class="stat-value">${r.correctCount}</div>
            <div class="stat-label">Correct</div>
        </div>
        <div class="stat-item">
            <div class="stat-value">${r.totalQuestions - r.correctCount}</div>
            <div class="stat-label">Wrong / Skipped</div>
        </div>
        <div class="stat-item">
            <div class="stat-value">${r.student.percentage.toFixed(1)}%</div>
            <div class="stat-label">Percentage</div>
        </div>
    `;
    $('#resultStats').innerHTML = statsHtml;

    // Reset review section
    $('#reviewSection').classList.add('hidden');
}

// View detailed review
$('#btnViewReview').addEventListener('click', () => {
    const reviewSection = $('#reviewSection');
    reviewSection.classList.toggle('hidden');

    if (!reviewSection.classList.contains('hidden') && state.result) {
        const list = $('#reviewList');
        list.innerHTML = '';

        state.result.reviews.forEach((rev, i) => {
            const item = document.createElement('div');
            item.className = `review-item ${rev.isCorrect ? 'correct' : 'incorrect'}`;
            item.innerHTML = `
                <div class="review-q">Q${i + 1}. ${rev.text}</div>
                <div class="review-answers">
                    <span>
                        <span class="answer-label">Your Answer:</span>
                        <span class="${rev.isCorrect ? 'answer-correct' : 'answer-wrong'}">${rev.userAnswer}</span>
                    </span>
                    <span>
                        <span class="answer-label">Correct:</span>
                        <span class="answer-correct">${rev.correctAnswer}</span>
                    </span>
                    <span>
                        <span class="answer-label">Marks:</span>
                        <span>${rev.marksAwarded}/${rev.maxMarks}</span>
                    </span>
                </div>
            `;
            list.appendChild(item);
        });
    }
});

// Back to home
$('#btnBackHome').addEventListener('click', () => {
    showScreen('login');
});

// ============================================================================
// 5. LEADERBOARD
// ============================================================================
async function loadLeaderboard() {
    try {
        const res = await fetch(`${API_BASE}/api/leaderboard`);
        if (!res.ok) throw new Error('Failed to fetch leaderboard');
        const data = await res.json();

        const tbody = $('#leaderboardBody');
        const emptyMsg = $('#lbEmpty');

        if (data.length === 0) {
            tbody.innerHTML = '';
            emptyMsg.classList.remove('hidden');
            return;
        }

        emptyMsg.classList.add('hidden');
        const medals = ['🥇', '🥈', '🥉'];

        tbody.innerHTML = data.map((s, i) => {
            const pctClass = s.percentage >= 80 ? 'pct-high' : s.percentage >= 50 ? 'pct-mid' : 'pct-low';
            const rankDisplay = i < 3 ? `<span class="rank-medal">${medals[i]}</span>` : (i + 1);
            return `
                <tr>
                    <td>${rankDisplay}</td>
                    <td>${s.rollNo}</td>
                    <td>${s.name}</td>
                    <td>${s.score}/${s.totalMarks}</td>
                    <td><span class="pct-badge ${pctClass}">${s.percentage.toFixed(1)}%</span></td>
                    <td>${s.timestamp || '—'}</td>
                </tr>
            `;
        }).join('');

    } catch (err) {
        showToast('Error loading leaderboard: ' + err.message);
    }
}

$('#btnRefreshLB').addEventListener('click', loadLeaderboard);

// ============================================================================
// 6. ADMIN PANEL
// ============================================================================
const adminQType = $('#adminQType');
const mcqGroup = $('#mcqOptionsGroup');
const tfGroup = $('#tfOptionsGroup');

adminQType.addEventListener('change', () => {
    if (adminQType.value === 'MCQ') {
        mcqGroup.classList.remove('hidden');
        tfGroup.classList.add('hidden');
    } else {
        mcqGroup.classList.add('hidden');
        tfGroup.classList.remove('hidden');
    }
});

$('#adminForm').addEventListener('submit', async (e) => {
    e.preventDefault();
    const msgEl = $('#adminMessage');
    msgEl.classList.add('hidden');

    const type = adminQType.value;
    const text = $('#adminQText').value.trim();
    const marks = parseInt($('#adminQMarks').value) || 5;

    if (!text) {
        showToast('Question text cannot be empty');
        return;
    }

    let payload = { type, text, marks };

    if (type === 'MCQ') {
        const options = [
            $('#adminOptA').value.trim(),
            $('#adminOptB').value.trim(),
            $('#adminOptC').value.trim(),
            $('#adminOptD').value.trim(),
        ];
        const correctOption = $('#adminCorrectOpt').value;
        if (options.some(o => !o)) {
            showToast('All 4 MCQ options are required');
            return;
        }
        payload.options = options;
        payload.correctOption = correctOption;
    } else {
        payload.correctAnswer = $('#adminCorrectTF').value;
    }

    try {
        const res = await fetch(`${API_BASE}/api/questions`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(payload),
        });

        const data = await res.json();

        if (res.ok && data.status === 'success') {
            msgEl.textContent = `✅ ${data.message} (ID: ${data.id})`;
            msgEl.className = 'admin-message success';
            msgEl.classList.remove('hidden');
            $('#adminForm').reset();
            adminQType.dispatchEvent(new Event('change'));
            showToast('Question added successfully!');
        } else {
            msgEl.textContent = `❌ ${data.message || 'Unknown error'}`;
            msgEl.className = 'admin-message error';
            msgEl.classList.remove('hidden');
        }
    } catch (err) {
        msgEl.textContent = '❌ Network error: ' + err.message;
        msgEl.className = 'admin-message error';
        msgEl.classList.remove('hidden');
    }
});

// ============================================================================
// INIT
// ============================================================================
showScreen('login');

