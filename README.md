# Морской бой

Настольная игра на C++17 и Qt Widgets: ручная или случайная расстановка флота,
игра против компьютера с двумя стратегиями, сохранение и загрузка расстановки.

## Скачать

[Готовые версии для Windows и macOS](https://github.com/Robshak/battle-ships-oop/releases/latest).
Qt входит в архивы, отдельно устанавливать его для запуска не нужно.

- **Windows 10/11, x64:** распакуйте весь архив и запустите `BattleShips.exe`.
  Соседние DLL и папки нужны для работы приложения.
- **macOS 12+, Intel и Apple Silicon:** распакуйте архив и перенесите
  `dist/BattleShips.app` в «Программы».

## Собрать

Нужны CMake 3.21+, компилятор C++17 и [Qt 6.9.3 с Widgets](https://www.qt.io/development/download-qt-installer-oss).
В командах укажите свой путь к Qt.

**macOS:** Xcode Command Line Tools и комплект Qt для macOS.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$HOME/Qt/6.9.3/macos"
cmake --build build --parallel 2
cmake --install build --prefix dist
"$HOME/Qt/6.9.3/macos/bin/macdeployqt" dist/BattleShips.app
```

Для сборки сразу под Intel и Apple Silicon добавьте к первой команде
`'-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64'`.

**Windows:** Visual Studio 2022 с Desktop development with C++ и Windows SDK,
комплект Qt MSVC 2022 64-bit. Команды для PowerShell:

```powershell
cmake -S . -B build -A x64 -DCMAKE_PREFIX_PATH=C:/Qt/6.9.3/msvc2022_64
cmake --build build --config Release --parallel 2
cmake --install build --config Release --prefix dist
C:/Qt/6.9.3/msvc2022_64/bin/windeployqt.exe --release dist/BattleShips.exe
```

Готовое приложение и библиотеки находятся в `dist`.
Уведомления о сторонних библиотеках — [THIRD_PARTY.md](THIRD_PARTY.md),
тексты лицензий — в `licenses`.
