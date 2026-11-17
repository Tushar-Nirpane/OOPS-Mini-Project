# 🚀 Deployment Guide — Getting a Live Web URL

The app is a single container: a C++17 server that serves both the REST API and the
frontend. Once deployed you get a public HTTPS URL that serves the whole quiz system.

Everything needed is already in this repo, and it has been **built and smoke-tested locally**
(UI + all four API endpoints + a full quiz submission).

---

## ✅ What is already prepared

| Piece | Why it matters |
|-------|----------------|
| `Dockerfile` | Multi-stage build: `gcc:13` compiles, `debian:bookworm-slim` runs (~90 MB final image) |
| `$PORT` support in `server.cpp` | Render/Railway inject `PORT` (Render defaults to `10000`); the server binds to it instead of a hardcoded 8080 |
| Binds `0.0.0.0` | Required by every container platform — `127.0.0.1` would be unreachable from outside |
| `window.location.origin` in `app.js` | API calls stay same-origin, so no CORS or hardcoded `localhost:8080` breaks in the cloud |
| `render.yaml` | Blueprint: free plan, Singapore region, health check on `/api/stats`, auto-deploy on push |
| `railway.json` | Railway build/deploy config with the same health check |
| `.dockerignore` | Keeps the prebuilt macOS binary and `.git` out of the build context |

**Health check endpoint:** `GET /api/stats` — returns `200` with a small JSON payload.

---

## Option A — Render (recommended, free, ~2 minutes)

### One-click blueprint deploy

Open this link while signed in to Render:

**https://render.com/deploy?repo=https://github.com/Tushar-Nirpane/OOPS-Mini-Project**

1. Sign in with GitHub (this also grants Render access to the repo).
2. Render reads `render.yaml` and shows the blueprint — click **Deploy Blueprint**.
3. Wait for the build (first build ~4–8 min: it downloads `gcc:13`).
4. Your URL appears at the top of the service page:

   ```
   https://oops-online-quiz-system.onrender.com
   ```

### Manual equivalent (if you prefer the dashboard)

1. **render.com** → sign in with GitHub → **+ New** → **Web Service**.
2. Choose **Build and deploy from a Git repository** → select `Tushar-Nirpane/OOPS-Mini-Project`.
3. Settings:
   - **Runtime / Language:** `Docker`
   - **Instance Type:** `Free`
   - **Region:** `Singapore` (closest to India)
   - **Health Check Path:** `/api/stats`
4. **Create Web Service**.

> Free instances go to sleep after 15 minutes of inactivity. The first request afterwards
> takes ~50 seconds to wake the container — this is normal, not a crash.

---

## Option B — Railway (free trial credits)

1. **railway.app** → sign in with GitHub.
2. **New Project** → **Deploy from GitHub repo** → `OOPS-Mini-Project`.
3. Railway detects the `Dockerfile` and `railway.json` automatically — no settings needed
   (it injects `PORT`; the server picks it up).
4. **Settings** → **Networking** → **Generate Domain**:

   ```
   https://oops-quiz-system.up.railway.app
   ```

---

## 🔍 Verify the live deployment

Replace `YOUR_URL` with the domain the platform gave you:

```bash
curl -s -o /dev/null -w "%{http_code}\n" YOUR_URL/            # 200  → UI loads
curl -s YOUR_URL/api/stats                                    # 200  → health check
curl -s YOUR_URL/api/questions | head -c 200                  # 200  → question bank
curl -s YOUR_URL/api/leaderboard | head -c 200                # 200  → leaderboard
```

Then open `YOUR_URL` in a browser and run through: login → quiz → result → leaderboard.

---

## 🧯 Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| Deploy fails: *"no open ports detected"* | App not bound to `0.0.0.0:$PORT` | Already handled — confirm you did not hardcode a port in the dashboard `PORT` var |
| Build fails at `g++` | Compile error | Reproduce locally: `g++ -std=c++17 -O2 -pthread backend/Question.cpp backend/QuizManager.cpp backend/server.cpp -o quiz_server` |
| `502` right after deploy | Container still starting | Wait 30–60 s, check the **Logs** tab |
| First visit after idle is slow (~50 s) | Free plan sleeps | Expected on Render free tier |
| Leaderboard resets after redeploy | Container filesystem is ephemeral — `data/results.txt` lives inside the container | Attach a persistent disk, or accept it for a lab project |
| Admin-added questions disappear | Same: `data/questions.txt` is rewritten inside the container | Re-add after deploy, or mount a disk at `/app/data` |

---

## 💾 Want data to survive redeploys?

Attach a persistent disk (Render: **Disks** → mount path `/app/data`, ~1 GB; paid plans) —
no code change needed, because `QuizManager` already reads and writes `data/questions.txt`
and `data/results.txt` relative to the working directory.
