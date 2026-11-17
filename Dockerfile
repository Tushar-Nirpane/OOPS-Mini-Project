# ---- Build Stage ----
FROM gcc:13 AS builder
WORKDIR /app
COPY backend/ backend/
COPY data/ data/
COPY frontend/ frontend/
# Link libstdc++/libgcc statically: the gcc:13 image provides GLIBCXX_3.4.32,
# but debian:bookworm-slim only ships GLIBCXX_3.4.30, so a dynamically linked
# binary fails at startup with "version `GLIBCXX_3.4.32' not found".
RUN g++ -std=c++17 -O2 -pthread -static-libstdc++ -static-libgcc \
    backend/Question.cpp backend/QuizManager.cpp backend/server.cpp \
    -o quiz_server

# ---- Runtime Stage ----
FROM debian:bookworm-slim
WORKDIR /app
COPY --from=builder /app/quiz_server .
COPY --from=builder /app/frontend/ frontend/
COPY --from=builder /app/data/ data/

# Render/Railway inject PORT (Render defaults to 10000); the server binds to it.
EXPOSE 8080
CMD ["./quiz_server"]
