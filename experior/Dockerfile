#XXXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXX5
# Two-stage Dockerfile for a microservice container

# --- Stage 1: Create builder image---
FROM python:3.9-slim AS builder

# Create (if non-existent) a /dir/ in this image and make it the PWD
WORKDIR /app

# Create a virtual environment
RUN python -m venv venv

# Activate the virtual environment
ENV PATH="/app/venv/bin:$PATH"

# In "COPY x y", x relative to host's build context (by "docker build" or by "context:" in Compose),
# and y relative to PWD of the image/container (typically after most recent WORKDIR)
COPY requirements.txt .

# Install builder dependencies into the virtual environment
RUN pip install --no-cache-dir -r requirements.txt

# --- Stage 2: Create runtime image and copy from builder only what is needed for running ---
FROM python:3.9-slim AS runtime

# Put this /dir/ in any container this (runtime) image starts
WORKDIR /app

# Copy the virtual environment from the builder image
COPY --from=builder /app/venv /app/venv

# Activate the virtual environment
ENV PATH="/app/venv/bin:$PATH"

COPY . .

CMD ["python", "microMain.py"]