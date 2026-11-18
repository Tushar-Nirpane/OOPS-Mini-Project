/**
 * @file app.js
 * @brief High-Performance Frontend Engine for Online Quiz System
 * Handles state, fetch calls, Web Audio sound effects, canvas confetti,
 * live question previews, keyboard shortcuts, and OOP inspector telemetry.
 */

// ============================================================================
// CONFIGURATION & CONSTANTS
// ============================================================================
const API_BASE = window.location.origin;
const QUIZ_TIME_SECONDS = 10 * 60; // 10 minutes

// ============================================================================
// APPLICATION STATE
// ============================================================================
const state = {
    student: { rollNo: '', name: '' },
    questions: [],
    answers: {},       // { questionId: "selectedAnswer" }
    flagged: {},       // { questionId: boolean }
    currentIndex: 0,
    timerSeconds: QUIZ_TIME_SECONDS,
    timerInterval: null,
    result: null,
    soundEnabled: true,
    leaderboardData: []
};

// ============================================================================
// DOM SELECTORS
// ============================================================================
const $ = (sel) => document.querySelector(sel);
const $$ = (sel) => document.querySelectorAll(sel);

const screens = {
    login: $('#screen-login'),
    quiz: $('#screen-quiz'),
    result: $('#screen-result'),
    leaderboard: $('#screen-leaderboard'),
    'oop-inspector': $('#screen-oop-inspector'),
    admin: $('#screen-admin')
};

// ============================================================================
// WEB AUDIO API SOUND SYNTHESIZER (NO EXTERNAL AUDIO FILES)
// ============================================================================
class SoundEffects {
    constructor() {
        this.ctx = null;
    }

    init() {
        if (!this.ctx) {
            const AudioCtx = window.AudioContext || window.webkitAudioContext;
            if (AudioCtx) this.ctx = new AudioCtx();
        }
    }

    playClick() {
        if (!state.soundEnabled) return;
        this.init();
        if (!this.ctx) return;

        const osc = this.ctx.createOscillator();
        const gain = this.ctx.createGain();
        osc.type = 'sine';
        osc.frequency.setValueAtTime(480, this.ctx.currentTime);
        osc.frequency.exponentialRampToValueAtTime(800, this.ctx.currentTime + 0.05);

        gain.gain.setValueAtTime(0.08, this.ctx.currentTime);
        gain.gain.exponentialRampToValueAtTime(0.001, this.ctx.currentTime + 0.05);

        osc.connect(gain);
        gain.connect(this.ctx.destination);
        osc.start();
        osc.stop(this.ctx.currentTime + 0.05);
    }

    playFlag() {
        if (!state.soundEnabled) return;
        this.init();
        if (!this.ctx) return;

        const osc = this.ctx.createOscillator();
        const gain = this.ctx.createGain();
        osc.type = 'triangle';
        osc.frequency.setValueAtTime(320, this.ctx.currentTime);
        osc.frequency.exponentialRampToValueAtTime(640, this.ctx.currentTime + 0.08);

        gain.gain.setValueAtTime(0.1, this.ctx.currentTime);
        gain.gain.exponentialRampToValueAtTime(0.001, this.ctx.currentTime + 0.08);

        osc.connect(gain);
        gain.connect(this.ctx.destination);
        osc.start();
        osc.stop(this.ctx.currentTime + 0.08);
    }

    playSuccess() {
        if (!state.soundEnabled) return;
        this.init();
        if (!this.ctx) return;

        const now = this.ctx.currentTime;
        const notes = [523.25, 659.25, 783.99, 1046.50]; // C5, E5, G5, C6 chord
        notes.forEach((freq, idx) => {
            const osc = this.ctx.createOscillator();
            const gain = this.ctx.createGain();
            osc.type = 'triangle';
            osc.frequency.setValueAtTime(freq, now + idx * 0.09);

            gain.gain.setValueAtTime(0.12, now + idx * 0.09);
            gain.gain.exponentialRampToValueAtTime(0.001, now + idx * 0.09 + 0.35);

            osc.connect(gain);
            gain.connect(this.ctx.destination);
            osc.start(now + idx * 0.09);
            osc.stop(now + idx * 0.09 + 0.35);
        });
    }

    playAlert() {
        if (!state.soundEnabled) return;
        this.init();
        if (!this.ctx) return;

        const osc = this.ctx.createOscillator();
        const gain = this.ctx.createGain();
        osc.type = 'sawtooth';
        osc.frequency.setValueAtTime(260, this.ctx.currentTime);

        gain.gain.setValueAtTime(0.12, this.ctx.currentTime);
        gain.gain.exponentialRampToValueAtTime(0.001, this.ctx.currentTime + 0.15);

        osc.connect(gain);
        gain.connect(this.ctx.destination);
        osc.start();
        osc.stop(this.ctx.currentTime + 0.15);
    }
}
const sound = new SoundEffects();

// ============================================================================
// CANVAS CONFETTI ENGINE (PURE JS, ZERO DEPENDENCIES)
// ============================================================================
function launchConfetti() {
    const canvas = document.getElementById('confettiCanvas');
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;

    const pieces = [];
    const colors = ['#6366f1', '#06b6d4', '#10b981', '#f59e0b', '#ec4899', '#8b5cf6'];

    for (let i = 0; i < 120; i++) {
        pieces.push({
            x: Math.random() * canvas.width,
            y: Math.random() * canvas.height * 0.4,
            w: Math.random() * 8 + 5,
            h: Math.random() * 12 + 6,
            color: colors[Math.floor(Math.random() * colors.length)],
            vx: (Math.random() - 0.5) * 6,
            vy: Math.random() * 4 + 2,
            rot: Math.random() * 360,
            rotSpeed: (Math.random() - 0.5) * 10
        });
    }

    let animationFrame;
    let frames = 0;

    function render() {
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        pieces.forEach(p => {
            ctx.save();
            ctx.translate(p.x, p.y);
            ctx.rotate((p.rot * Math.PI) / 180);
            ctx.fillStyle = p.color;
            ctx.fillRect(-p.w / 2, -p.h / 2, p.w, p.h);
            ctx.restore();

            p.x += p.vx;
            p.y += p.vy;
            p.rot += p.rotSpeed;
        });

        frames++;
        if (frames < 180) {
            animationFrame = requestAnimationFrame(render);
        } else {
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            cancelAnimationFrame(animationFrame);
        }
    }
    render();
}

// ============================================================================
// TOAST NOTIFICATION SYSTEM
// ============================================================================
function showToast(message, type = 'info', duration = 3200) {
    const container = $('#toastContainer');
    if (!container) return;

    const toast = document.createElement('div');
    toast.className = `toast-bubble toast-${type}`;

    let icon = 'ℹ️';
    if (type === 'success') icon = '✅';
    if (type === 'error') icon = '⚠️';

    toast.innerHTML = `<span>${icon}</span><span>${message}</span>`;
    container.appendChild(toast);

    setTimeout(() => {
        toast.style.opacity = '0';
        toast.style.transform = 'translateX(100%)';
        setTimeout(() => toast.remove(), 260);
    }, duration);
}

// ============================================================================
// NAVIGATION & THEME CONTROLS
// ============================================================================
function showScreen(name) {
    Object.values(screens).forEach(s => s && s.classList.remove('active'));
    if (screens[name]) screens[name].classList.add('active');

    $$('.tab-btn').forEach(btn => {
        btn.classList.toggle('active', btn.dataset.screen === name);
    });

    if (name === 'leaderboard') loadLeaderboard();
    if (name === 'oop-inspector' || name === 'login') fetchTelemetryStats();
}

$$('.tab-btn').forEach(btn => {
    btn.addEventListener('click', () => {
        sound.playClick();
        showScreen(btn.dataset.screen);
    });
});

// Theme Management
const themeToggle = $('#themeToggle');
const themeIcon = $('#themeIcon');
const savedTheme = localStorage.getItem('quizTheme') || 'light';
document.documentElement.setAttribute('data-theme', savedTheme);
themeIcon.textContent = savedTheme === 'dark' ? '☀️' : '🌙';

themeToggle.addEventListener('click', () => {
    sound.playClick();
    const current = document.documentElement.getAttribute('data-theme');
    const next = current === 'dark' ? 'light' : 'dark';
    document.documentElement.setAttribute('data-theme', next);
    localStorage.setItem('quizTheme', next);
    themeIcon.textContent = next === 'dark' ? '☀️' : '🌙';
});

// Sound Toggle
const soundToggle = $('#soundToggle');
const soundIcon = $('#soundIcon');
state.soundEnabled = localStorage.getItem('quizSound') !== 'false';
soundIcon.textContent = state.soundEnabled ? '🔊' : '🔇';

soundToggle.addEventListener('click', () => {
    state.soundEnabled = !state.soundEnabled;
    localStorage.setItem('quizSound', state.soundEnabled);
    soundIcon.textContent = state.soundEnabled ? '🔊' : '🔇';
    if (state.soundEnabled) sound.playClick();
});

// ============================================================================
// TELEMETRY & LIVE C++ STATS
// ============================================================================
async function fetchTelemetryStats() {
    try {
        const res = await fetch(`${API_BASE}/api/stats`);
        if (!res.ok) return;
        const data = await res.json();

        // Update home metrics
        if ($('#metricQBank')) $('#metricQBank').textContent = data.currentQuestionsInBank || 10;
        if ($('#metricAttempts')) $('#metricAttempts').textContent = data.totalQuizzesAttempted || 0;

        // Update OOP Inspector metrics
        if ($('#teleQuestions')) $('#teleQuestions').textContent = data.totalQuestionsCreated || 10;
        if ($('#teleQuizzes')) $('#teleQuizzes').textContent = data.totalQuizzesAttempted || 0;
    } catch {
        // Fallback to defaults
    }
}

// ============================================================================
// 1. LOGIN & START EXAMINATION
// ============================================================================
$('#loginForm').addEventListener('submit', async (e) => {
    e.preventDefault();
    sound.playClick();

    const rollNo = $('#rollNo').value.trim();
    const name = $('#studentName').value.trim();

    if (!rollNo || !name) {
        showToast('Please enter both University Roll Number and Full Name.', 'error');
        return;
    }

    state.student = { rollNo, name };
    state.answers = {};
    state.flagged = {};
    state.currentIndex = 0;
    state.timerSeconds = QUIZ_TIME_SECONDS;

    try {
        showToast('Fetching randomized questions from C++ Engine...', 'info', 1500);
        await loadQuestions();
        if (state.questions.length === 0) {
            showToast('Question bank is empty. Please contact administrator.', 'error');
            return;
        }
        startQuiz();
    } catch (err) {
        showToast('Connection error: ' + err.message, 'error');
    }
});

async function loadQuestions() {
    const res = await fetch(`${API_BASE}/api/questions`);
    if (!res.ok) throw new Error('C++ Server returned status ' + res.status);
    state.questions = await res.json();
}

function startQuiz() {
    showScreen('quiz');

    // Populate student badge
    $('#quizStudentName').textContent = state.student.name;
    $('#quizStudentRoll').textContent = state.student.rollNo;
    const initials = state.student.name.split(' ').map(n => n[0]).join('').substring(0, 2).toUpperCase();
    $('#candidateAvatar').textContent = initials || 'ST';

    buildNavigator();
    renderQuestion();
    startTimer();
    showToast('Examination session active! Best of luck.', 'success');
}

// ============================================================================
// 2. QUIZ SCREEN ENGINE & PALETTE
// ============================================================================
function buildNavigator() {
    const nav = $('#questionNav');
    nav.innerHTML = '';
    state.questions.forEach((q, i) => {
        const pill = document.createElement('button');
        pill.className = 'nav-pill';
        pill.textContent = i + 1;
        pill.id = `navPill-${i}`;
        pill.addEventListener('click', () => {
            sound.playClick();
            state.currentIndex = i;
            renderQuestion();
        });
        nav.appendChild(pill);
    });
}

function updatePaletteState() {
    let answeredCount = 0;
    let flaggedCount = 0;

    state.questions.forEach((q, i) => {
        const pill = $(`#navPill-${i}`);
        if (!pill) return;

        pill.className = 'nav-pill';

        const isAnswered = state.answers[q.id] !== undefined;
        const isFlagged = !!state.flagged[q.id];

        if (isAnswered) answeredCount++;
        if (isFlagged) flaggedCount++;

        if (i === state.currentIndex) {
            pill.classList.add('current');
        } else if (isFlagged) {
            pill.classList.add('flagged');
        } else if (isAnswered) {
            pill.classList.add('answered');
        }
    });

    const total = state.questions.length;
    $('#paletteCounter').textContent = `${answeredCount}/${total} Answered`;
    $('#sumAnswered').textContent = answeredCount;
    $('#sumFlagged').textContent = flaggedCount;
    $('#sumRemaining').textContent = total - answeredCount;
}

function renderQuestion() {
    const q = state.questions[state.currentIndex];
    const total = state.questions.length;
    const idx = state.currentIndex;

    // Headers & Progress
    $('#questionCounter').textContent = `Question ${idx + 1} of ${total}`;
    const pct = Math.round(((idx + 1) / total) * 100);
    $('#progressPercentage').textContent = `${pct}% Complete`;
    $('#progressBar').style.width = `${pct}%`;

    // Metadata Badges
    $('#questionTypeBadge').textContent = q.type === 'MCQ' ? 'Multiple Choice' : 'True / False';
    $('#questionMarksBadge').textContent = `+${q.marks} Marks`;

    // Flag button state
    const isFlagged = !!state.flagged[q.id];
    const flagBtn = $('#btnFlag');
    flagBtn.classList.toggle('flagged', isFlagged);
    $('#flagBtnText').textContent = isFlagged ? 'Flagged' : 'Flag Question';

    // Question Text
    $('#questionText').textContent = q.text;

    // Render Options
    const container = $('#optionsContainer');
    container.innerHTML = '';
    const letters = ['A', 'B', 'C', 'D'];

    q.options.forEach((opt, i) => {
        const card = document.createElement('div');
        card.className = 'option-card';
        const optLetter = q.type === 'TF' ? (i === 0 ? 'T' : 'F') : letters[i];
        const optVal = q.type === 'TF' ? opt : letters[i];

        if (state.answers[q.id] === optVal) {
            card.classList.add('selected');
        }

        card.innerHTML = `
            <div class="option-badge">${optLetter}</div>
            <span class="option-label-text">${opt}</span>
        `;

        card.addEventListener('click', () => {
            sound.playClick();
            state.answers[q.id] = optVal;
            container.querySelectorAll('.option-card').forEach(c => c.classList.remove('selected'));
            card.classList.add('selected');
            updatePaletteState();
        });

        container.appendChild(card);
    });

    // Navigation Controls
    $('#btnPrev').disabled = idx === 0;
    const isLast = idx === total - 1;
    $('#btnNext').classList.toggle('hidden', isLast);
    $('#btnSubmit').classList.toggle('hidden', !isLast);

    updatePaletteState();
}

// Navigation Actions
$('#btnPrev').addEventListener('click', () => {
    sound.playClick();
    if (state.currentIndex > 0) {
        state.currentIndex--;
        renderQuestion();
    }
});

$('#btnNext').addEventListener('click', () => {
    sound.playClick();
    if (state.currentIndex < state.questions.length - 1) {
        state.currentIndex++;
        renderQuestion();
    }
});

// Flag Question Toggle
$('#btnFlag').addEventListener('click', () => {
    sound.playFlag();
    const q = state.questions[state.currentIndex];
    state.flagged[q.id] = !state.flagged[q.id];
    renderQuestion();
});

// Clear Option Choice
$('#btnClearOption').addEventListener('click', () => {
    sound.playClick();
    const q = state.questions[state.currentIndex];
    delete state.answers[q.id];
    renderQuestion();
    showToast('Selection cleared.', 'info', 1200);
});

// Countdown Timer Engine
function startTimer() {
    clearInterval(state.timerInterval);
    state.timerSeconds = QUIZ_TIME_SECONDS;
    updateTimerDisplay();

    state.timerInterval = setInterval(() => {
        state.timerSeconds--;
        updateTimerDisplay();

        const timerCard = $('#timerCard');
        if (state.timerSeconds <= 60) {
            timerCard.className = 'timer-card danger';
            if (state.timerSeconds % 10 === 0) sound.playAlert();
        } else if (state.timerSeconds <= 120) {
            timerCard.className = 'timer-card warning';
        } else {
            timerCard.className = 'timer-card';
        }

        if (state.timerSeconds <= 0) {
            clearInterval(state.timerInterval);
            sound.playAlert();
            showToast('⏰ Time has expired! Auto-evaluating examination...', 'error');
            setTimeout(() => executeSubmission(), 600);
        }
    }, 1000);
}

function updateTimerDisplay() {
    const mins = Math.floor(state.timerSeconds / 60);
    const secs = state.timerSeconds % 60;
    $('#timerDisplay').textContent = `${String(mins).padStart(2, '0')}:${String(secs).padStart(2, '0')}`;
}

// ============================================================================
// 3. SUBMISSION MODAL & API EVALUATION
// ============================================================================
function openSubmitModal() {
    sound.playClick();
    const total = state.questions.length;
    let answered = 0;
    let flagged = 0;

    state.questions.forEach(q => {
        if (state.answers[q.id] !== undefined) answered++;
        if (state.flagged[q.id]) flagged++;
    });

    const unanswered = total - answered;

    $('#mstatAnswered').textContent = answered;
    $('#mstatUnanswered').textContent = unanswered;
    $('#mstatFlagged').textContent = flagged;
    $('#modalSummaryText').textContent =
        unanswered > 0
            ? `You have ${unanswered} unanswered question(s). Ready to submit for final evaluation?`
            : `All ${total} questions answered! Ready to generate your scorecard?`;

    $('#submitModal').classList.remove('hidden');
}

$('#btnSubmit').addEventListener('click', openSubmitModal);
$('#btnPaletteSubmit').addEventListener('click', openSubmitModal);
$('#btnEarlyFinish').addEventListener('click', openSubmitModal);

$('#btnCancelModal').addEventListener('click', () => {
    sound.playClick();
    $('#submitModal').classList.add('hidden');
});

$('#btnConfirmSubmit').addEventListener('click', () => {
    $('#submitModal').classList.add('hidden');
    executeSubmission();
});

async function executeSubmission() {
    clearInterval(state.timerInterval);
    showToast('Submitting responses to C++ evaluation engine...', 'info', 2000);

    const answersMap = {};
    for (const [key, val] of Object.entries(state.answers)) {
        answersMap[String(key)] = val;
    }

    const payload = {
        rollNo: state.student.rollNo,
        name: state.student.name,
        answers: answersMap
    };

    try {
        const res = await fetch(`${API_BASE}/api/submit`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(payload)
        });

        if (!res.ok) {
            const err = await res.json();
            throw new Error(err.message || 'Submission failed');
        }

        state.result = await res.json();
        renderScorecard();
    } catch (err) {
        showToast('Submission error: ' + err.message, 'error');
    }
}

// ============================================================================
// 4. RESULT SCORECARD & PERFORMANCE REVIEW
// ============================================================================
function renderScorecard() {
    showScreen('result');
    const r = state.result;

    // Badge & Heading
    const badge = $('#resultBadge');
    badge.textContent = r.passed ? 'PASSED WITH MERIT' : 'RETAKE REQUIRED';
    badge.className = 'result-badge-pill ' + (r.passed ? 'passed' : 'failed');

    $('#resultTitle').textContent = `Scorecard: ${r.student.name}`;
    $('#resultSubtitle').textContent = `Roll No: ${r.student.rollNo} • Submitted ${r.student.timestamp}`;

    // Metrics
    $('#resScore').textContent = `${r.student.score} / ${r.student.totalMarks}`;
    $('#resCorrect').textContent = r.correctCount;
    $('#resWrong').textContent = r.totalQuestions - r.correctCount;

    const pct = r.student.percentage;
    $('#resAccuracy').textContent = `${pct.toFixed(1)}%`;
    $('#gaugePct').textContent = `${Math.round(pct)}%`;

    // Grade classification
    let grade = 'GRADE F';
    let gradeColor = 'var(--danger)';
    if (pct >= 90) { grade = 'GRADE O (Outstanding)'; gradeColor = '#f59e0b'; }
    else if (pct >= 80) { grade = 'GRADE A+ (Excellent)'; gradeColor = 'var(--success)'; }
    else if (pct >= 70) { grade = 'GRADE A (Very Good)'; gradeColor = 'var(--brand-primary)'; }
    else if (pct >= 60) { grade = 'GRADE B (Good)'; gradeColor = 'var(--brand-primary)'; }
    else if (pct >= 40) { grade = 'GRADE C (Pass)'; gradeColor = 'var(--text-secondary)'; }

    const gaugeGradeEl = $('#gaugeGrade');
    gaugeGradeEl.textContent = grade;
    gaugeGradeEl.style.color = gradeColor;

    // Animate Circular Gauge Fill
    const circumference = 2 * Math.PI * 74; // ~464.95
    const offset = circumference - (pct / 100) * circumference;
    const ringFill = $('#progressRingFill');
    ringFill.style.stroke = gradeColor;
    setTimeout(() => {
        ringFill.style.strokeDashoffset = offset;
    }, 150);

    // Audio & Confetti
    if (r.passed) {
        sound.playSuccess();
        launchConfetti();
        showToast('🎉 Congratulations! You have successfully passed the exam.', 'success');
    } else {
        sound.playAlert();
        showToast('You scored below the passing mark of 40%.', 'info');
    }

    // Populate Detailed Review List
    populateDetailedReview('all');
}

function populateDetailedReview(filter = 'all') {
    const list = $('#reviewList');
    list.innerHTML = '';
    if (!state.result || !state.result.reviews) return;

    const reviews = state.result.reviews;
    const correctReviews = reviews.filter(rev => rev.isCorrect);
    const incorrectReviews = reviews.filter(rev => !rev.isCorrect);

    $('#countAllRev').textContent = reviews.length;
    $('#countCorrectRev').textContent = correctReviews.length;
    $('#countIncorrectRev').textContent = incorrectReviews.length;

    let displayList = reviews;
    if (filter === 'correct') displayList = correctReviews;
    if (filter === 'incorrect') displayList = incorrectReviews;

    displayList.forEach((rev, idx) => {
        const item = document.createElement('div');
        item.className = `review-item-v2 ${rev.isCorrect ? 'correct' : 'incorrect'}`;

        item.innerHTML = `
            <div class="review-q-header">
                <span class="review-q-num">QUESTION #${rev.id} (${rev.type})</span>
                <span class="review-status-pill ${rev.isCorrect ? 'correct' : 'incorrect'}">
                    ${rev.isCorrect ? `+${rev.marksAwarded} Marks` : '0 Marks'}
                </span>
            </div>
            <div class="review-q-prompt">${rev.text}</div>
            <div class="review-answers-matrix">
                <div class="ans-slot">
                    <span class="ans-slot-tag">Your Selection</span>
                    <span class="ans-slot-val ${rev.isCorrect ? 'ans-correct' : 'ans-wrong'}">
                        ${rev.userAnswer || '(Not Attempted)'}
                    </span>
                </div>
                <div class="ans-slot">
                    <span class="ans-slot-tag">Correct Answer</span>
                    <span class="ans-slot-val ans-correct">${rev.correctAnswer}</span>
                </div>
            </div>
        `;
        list.appendChild(item);
    });
}

// Review Filter Tabs
$$('.filter-tab-btn').forEach(btn => {
    btn.addEventListener('click', () => {
        sound.playClick();
        $$('.filter-tab-btn').forEach(b => b.classList.remove('active'));
        btn.classList.add('active');
        populateDetailedReview(btn.dataset.filter);
    });
});

$('#btnToggleReview').addEventListener('click', () => {
    sound.playClick();
    const section = $('#reviewSection');
    section.classList.toggle('hidden');
    $('#reviewToggleText').textContent = section.classList.contains('hidden')
        ? '📋 Show Detailed Question Review'
        : '🔼 Hide Detailed Question Review';
});

$('#btnPrintResult').addEventListener('click', () => {
    sound.playClick();
    window.print();
});

$('#btnGoHome').addEventListener('click', () => {
    sound.playClick();
    showScreen('login');
});

// ============================================================================
// 5. LEADERBOARD ENGINE & PODIUM
// ============================================================================
async function loadLeaderboard() {
    try {
        const res = await fetch(`${API_BASE}/api/leaderboard`);
        if (!res.ok) throw new Error('Could not fetch leaderboard data');
        state.leaderboardData = await res.json();
        renderLeaderboardTable(state.leaderboardData);
    } catch (err) {
        showToast('Error loading leaderboard: ' + err.message, 'error');
    }
}

function renderLeaderboardTable(records) {
    const tbody = $('#leaderboardBody');
    const emptyState = $('#lbEmpty');
    const table = $('#leaderboardTable');

    if (!records || records.length === 0) {
        tbody.innerHTML = '';
        table.classList.add('hidden');
        emptyState.classList.remove('hidden');
        $('#lbMetaCount').textContent = '0 candidates';
        return;
    }

    table.classList.remove('hidden');
    emptyState.classList.add('hidden');
    $('#lbMetaCount').textContent = `Showing ${records.length} candidate(s)`;

    // Populate Top 3 Podium
    const p1 = records[0];
    const p2 = records[1];
    const p3 = records[2];

    if (p1) {
        $('#podium1Name').textContent = p1.name;
        $('#podium1Score').textContent = `${p1.score}/${p1.totalMarks} (${p1.percentage.toFixed(0)}%)`;
    }
    if (p2) {
        $('#podium2Name').textContent = p2.name;
        $('#podium2Score').textContent = `${p2.score}/${p2.totalMarks} (${p2.percentage.toFixed(0)}%)`;
    }
    if (p3) {
        $('#podium3Name').textContent = p3.name;
        $('#podium3Score').textContent = `${p3.score}/${p3.totalMarks} (${p3.percentage.toFixed(0)}%)`;
    }

    // Render Table Rows
    tbody.innerHTML = records.map((s, idx) => {
        let gradeLetter = 'C';
        if (s.percentage >= 90) gradeLetter = 'O';
        else if (s.percentage >= 80) gradeLetter = 'A';
        else if (s.percentage >= 60) gradeLetter = 'B';
        else if (s.percentage < 40) gradeLetter = 'F';

        return `
            <tr>
                <td><span class="rank-cell-tag">${idx + 1}</span></td>
                <td><span class="candidate-name-cell">${s.name}</span></td>
                <td><span class="roll-cell">${s.rollNo}</span></td>
                <td><span class="score-cell">${s.score} / ${s.totalMarks}</span></td>
                <td><strong>${s.percentage.toFixed(1)}%</strong></td>
                <td><span class="grade-badge grade-${gradeLetter}">Grade ${gradeLetter}</span></td>
                <td style="font-size:0.8rem; color:var(--text-muted);">${s.timestamp}</td>
            </tr>
        `;
    }).join('');
}

// Client-side search on leaderboard
$('#lbSearchInput').addEventListener('input', (e) => {
    const q = e.target.value.toLowerCase().trim();
    if (!q) {
        renderLeaderboardTable(state.leaderboardData);
        return;
    }
    const filtered = state.leaderboardData.filter(s =>
        s.name.toLowerCase().includes(q) || s.rollNo.toLowerCase().includes(q)
    );
    renderLeaderboardTable(filtered);
});

$('#btnRefreshLB').addEventListener('click', () => {
    sound.playClick();
    showToast('Refreshing leaderboard rankings...', 'info', 1200);
    loadLeaderboard();
});

// ============================================================================
// 6. ADMIN PANEL & REAL-TIME PREVIEW
// ============================================================================
const adminQType = $('#adminQType');
const mcqGroup = $('#mcqOptionsGroup');
const tfGroup = $('#tfOptionsGroup');

adminQType.addEventListener('change', () => {
    sound.playClick();
    if (adminQType.value === 'MCQ') {
        mcqGroup.classList.remove('hidden');
        tfGroup.classList.add('hidden');
    } else {
        mcqGroup.classList.add('hidden');
        tfGroup.classList.remove('hidden');
    }
    updateAdminPreview();
});

function updateAdminPreview() {
    const type = adminQType.value;
    const text = $('#adminQText').value.trim() || 'Your question prompt will preview here in real-time as you type...';
    const marks = $('#adminQMarks').value || 5;

    $('#previewType').textContent = `${type === 'MCQ' ? 'Multiple Choice Question' : 'True/False Question'} (${marks} Marks)`;
    $('#previewText').textContent = text;

    const optContainer = $('#previewOptions');
    optContainer.innerHTML = '';

    if (type === 'MCQ') {
        const opts = [
            $('#adminOptA').value.trim() || 'Option A',
            $('#adminOptB').value.trim() || 'Option B',
            $('#adminOptC').value.trim() || 'Option C',
            $('#adminOptD').value.trim() || 'Option D'
        ];
        const letters = ['A', 'B', 'C', 'D'];
        opts.forEach((opt, idx) => {
            const div = document.createElement('div');
            div.className = 'prev-opt';
            div.textContent = `${letters[idx]}) ${opt}`;
            optContainer.appendChild(div);
        });
    } else {
        ['True', 'False'].forEach(opt => {
            const div = document.createElement('div');
            div.className = 'prev-opt';
            div.textContent = `🔘 ${opt}`;
            optContainer.appendChild(div);
        });
    }
}

// Attach input listeners for live preview
['adminQText', 'adminQMarks', 'adminOptA', 'adminOptB', 'adminOptC', 'adminOptD'].forEach(id => {
    const el = document.getElementById(id);
    if (el) el.addEventListener('input', updateAdminPreview);
});

// Sample Question Filler
$('#btnFillSampleQ').addEventListener('click', () => {
    sound.playClick();
    adminQType.value = 'MCQ';
    adminQType.dispatchEvent(new Event('change'));

    $('#adminQText').value = 'Which C++ design pattern ensures a class has only one instance and provides a global point of access to it?';
    $('#adminQMarks').value = '5';
    $('#adminOptA').value = 'Factory Pattern';
    $('#adminOptB').value = 'Singleton Pattern';
    $('#adminOptC').value = 'Observer Pattern';
    $('#adminOptD').value = 'Adapter Pattern';
    $('#adminCorrectOpt').value = 'B';

    updateAdminPreview();
    showToast('Sample C++ question loaded into form.', 'success', 1500);
});

// Admin Form Submission
$('#adminForm').addEventListener('submit', async (e) => {
    e.preventDefault();
    sound.playClick();

    const msgEl = $('#adminMessage');
    msgEl.classList.add('hidden');

    const type = adminQType.value;
    const text = $('#adminQText').value.trim();
    const marks = parseInt($('#adminQMarks').value, 10) || 5;

    if (!text) {
        showToast('Question text cannot be empty.', 'error');
        return;
    }

    const payload = { type, text, marks };

    if (type === 'MCQ') {
        const options = [
            $('#adminOptA').value.trim(),
            $('#adminOptB').value.trim(),
            $('#adminOptC').value.trim(),
            $('#adminOptD').value.trim()
        ];
        const correctOption = $('#adminCorrectOpt').value;
        if (options.some(o => !o)) {
            showToast('All 4 MCQ option choices must be provided.', 'error');
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
            body: JSON.stringify(payload)
        });

        const data = await res.json();
        if (res.ok && data.status === 'success') {
            sound.playSuccess();
            msgEl.textContent = `✅ Successfully added question #${data.id} directly to data/questions.txt!`;
            msgEl.className = 'admin-message success';
            msgEl.classList.remove('hidden');

            $('#adminForm').reset();
            adminQType.value = 'MCQ';
            adminQType.dispatchEvent(new Event('change'));
            updateAdminPreview();
            fetchTelemetryStats();
            showToast('Question appended to bank!', 'success');
        } else {
            msgEl.textContent = `❌ ${data.message || 'Error adding question'}`;
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
// KEYBOARD SHORTCUTS CONTROLLER
// ============================================================================
window.addEventListener('keydown', (e) => {
    // Only active during quiz screen
    if (!screens.quiz.classList.contains('active')) return;
    if (['INPUT', 'TEXTAREA'].includes(document.activeElement.tagName)) return;

    const key = e.key.toUpperCase();
    const q = state.questions[state.currentIndex];
    if (!q) return;

    // Option selections: A, B, C, D or 1, 2, 3, 4
    if (q.type === 'MCQ') {
        let selectedLetter = null;
        if (['A', '1'].includes(key)) selectedLetter = 'A';
        if (['B', '2'].includes(key)) selectedLetter = 'B';
        if (['C', '3'].includes(key)) selectedLetter = 'C';
        if (['D', '4'].includes(key)) selectedLetter = 'D';

        if (selectedLetter) {
            sound.playClick();
            state.answers[q.id] = selectedLetter;
            renderQuestion();
        }
    } else if (q.type === 'TF') {
        if (['T', '1'].includes(key)) {
            sound.playClick();
            state.answers[q.id] = 'True';
            renderQuestion();
        } else if (['F', '2'].includes(key)) {
            sound.playClick();
            state.answers[q.id] = 'False';
            renderQuestion();
        }
    }

    // Next: ArrowRight or N
    if (e.key === 'ArrowRight' || key === 'N') {
        if (state.currentIndex < state.questions.length - 1) {
            sound.playClick();
            state.currentIndex++;
            renderQuestion();
        }
    }

    // Prev: ArrowLeft or P
    if (e.key === 'ArrowLeft' || key === 'P') {
        if (state.currentIndex > 0) {
            sound.playClick();
            state.currentIndex--;
            renderQuestion();
        }
    }

    // Flag: F
    if (key === 'F') {
        sound.playFlag();
        state.flagged[q.id] = !state.flagged[q.id];
        renderQuestion();
    }
});

// ============================================================================
// INITIALIZATION
// ============================================================================
showScreen('login');
fetchTelemetryStats();
updateAdminPreview();
