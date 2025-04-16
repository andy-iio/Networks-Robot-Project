FROM gcc:latest

# Install build tools
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Copy all files into the container
COPY . .

# Remove any previous CMakeCache.txt and build artifacts (if any)
RUN rm -rf build CMakeCache.txt

# Build with CMake 
RUN cmake -S . -B build && cmake --build build

# Run the final binary 
CMD ["./build/Robot"]
