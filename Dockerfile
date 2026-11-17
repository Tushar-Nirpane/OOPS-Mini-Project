# ---- Build Stage ----
FROM gcc:13 AS builder
WORKDIR /app
COPY backend/ backend/
COPY data/ data/
COPY frontend/ frontend/
RUN g++ -std=c++17 -O2 -pthread \
    backend/Question.cpp backend/QuizManager.cpp backend/server.cpp \
    -o quiz_server

# ---- Runtime Stage ----
FROM debian:bookworm-slim
WORKDIR /app
RUN apt-get update && apt-get install -y --no-install-recommends libstdc++6 && \
    rm -rf /var/lib/apt/lists/*
COPY --from=builder /app/quiz_server .
COPY --from=builder /app/frontend/ frontend/
COPY --from=builder /app/data/ data/

EXPOSE 8080
CMD ["./quiz_server", "8080"]

