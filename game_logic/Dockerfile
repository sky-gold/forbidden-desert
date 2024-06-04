FROM ubuntu:22.04
RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        python3 \
        python3-pip \
        python3-venv \
    && rm -rf /var/lib/apt/lists/*
RUN pip3 install Conan

# Создаем профиль по умолчанию для Conan
RUN conan profile detect

# Копируем файлы проекта в контейнер
COPY . /app

# Переходим в рабочую директорию
WORKDIR /app

# Устанавливаем зависимости проекта с помощью Conan
RUN conan install . --output-folder=build --build=missing

WORKDIR /app/build

# Настраиваем проект с использованием CMake
RUN cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release

# Компилируем проект
RUN cmake --build .

# Запускаем приложение
CMD ["./game_logic"]