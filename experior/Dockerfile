#XXXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXX5
# Two-stage Dockerfile for a microservice container

# --- Stage 1: Create builder image---
FROM python:3.9-slim AS builder

# Create (if non-existent) a /dir/ in this image and make it the PWD
WORKDIR /pyapp

# Create a virtual environment
RUN python -m venv venv

# Activate the virtual environment
ENV PATH="/pyapp/venv/bin:$PATH"

# In "COPY x y", x relative to host's build context (by "docker build" or by "context:" in Compose),
# and y relative to PWD of the image/container (typically after most recent WORKDIR)
COPY requirements.txt .

# Install builder dependencies into the virtual environment
RUN pip install --no-cache-dir -r requirements.txt

# --- Stage 2: Create runtime image and copy from builder only what is needed for running ---
FROM python:3.9-slim AS runtime

# Put this /dir/ in any container this (runtime) image starts
WORKDIR /pyapp

# Copy the virtual environment from the builder image
COPY --from=builder /pyapp/venv /pyapp/venv

# Activate the copied virtual environment
ENV PATH="/pyapp/venv/bin:$PATH"

#COPY . .
# Copy application code and its pre-compiled dependenciesinto the runtime image
COPY microMain.py .
COPY microservice_pb2.py .
COPY microservice_pb2_grpc.py .

# Finish image by CMD because "*.py" is not executable unless made an argument of "python" (the runtime)
# [versus C++, which can finish image by ENTRYPOINT because a C++ build creates a runtime executable]
CMD ["python", "microMain.py"]