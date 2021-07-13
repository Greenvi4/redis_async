# redis_async
Асинхронный клиент для Redis.

# Мотивация
При разработке программного обеспечения мы много работаем с внешними сетевыми ресурсами. К большинству из них можно получить доступ через асинхронный интерфейс, поэтому нам не нужна блокировка, чтобы дождаться завершения запроса. 

# Обзор
redis_async - это неофициальная асинхронная клиентская библиотека Redis, написанная на С++(std=c++14) основа на библиотеке boost::asio, используемой для асинхронного сетевого ввода-вывода. Писалась для **Redis версии 3.9**. Версия достаточно старая, но расширение до более современной версии не составит труда

## Возможности

* асинхронные операции
* пул подключений
* алиасы для подключений
* возвращаемые значения приведены к стандартным типам, но обернутым в boost::varinat
* Соединение через TCP или UNIX сокеты

## Поддерживаемые команды
К сожалению, не все команды Redis еще поддерживается. По мере необходимости и возможности список будет увеличиваться

* [x] Keys
* [x] Strings
* [x] Hashes
* [x] Lists
* [x] Sets
* [ ] Sorted sets
* [ ] Pub\Sub
* [ ] Pipeline

# Использование
```cpp
    using namespace redis_async;

    // создаем новое подключение к Redis на локальном
    // хосте с портом по умолчанию 6379
    rd_service::add_connection("main=tcp://localhost"_redis);

    // Команда PING без параметров, Redis возвращает PONG
    rd_service::execute(
        "main"_rd, cmd::ping(),
        // Функция обработки результатов(result_t - boost::variant для возвращаемых типов)
        [](const result_t &res) { std::cout << boost::get<string_t>(res) << std::endl; },
        // Обработчик ошибок, как от Redis, библиотеки так и от пользователя(например, exception)
        [](const error::rd_error &err) {
            std::cerr << err.what() << std::endl;
            rd_service::stop();
        });

    // Команда PING со строкой, Redis возвращает строку
    const std::string msg = "Hello, World!";
    rd_service::execute(
        "main"_rd, cmd::ping(msg),
        [](const result_t &res) { std::cout << boost::get<string_t>(res) << std::endl; },
        [](const error::rd_error &err) {
            std::cerr << err.what() << std::endl;
            rd_service::stop();
        });

    // Команда PING с пустой строкой, Redis возвращает пустую строку
    rd_service::execute(
        "main"_rd, cmd::ping(""),
        [](const result_t &res) {
            auto str = boost::get<string_t>(res);
            if (str.empty())
                std::cout << "{EMPTY STRING}" << std::endl;
            else
                std::cout << str << std::endl;
            rd_service::stop();
        },
        [](const error::rd_error &err) {
            std::cerr << err.what() << std::endl;
            rd_service::stop();
        });
    
    // Запускаем сервис в работу
    rd_service::run();
```
