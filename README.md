# botva
is botva
# Server
## main()
В фуннкции main() сервера создается и биндится UDP соккет, который отвечает, за подключение клиента к серверу. Клиент отправялет свой никнейм и получает в ответ два номера порта для отправки и получения сообщений, в то время как на сервере запускаются TCPRecvThread() и TCPSendThread() в качестве потоков, в которые передаются те самые порты в качестве аргументов. Используется именно UDP сокет, потому что, не смотря на ненадежность по сравнению с TCP, при использовании второго протокола, если после окончания обмена данными соккет клента по какой-либо причине не закроет соединение, сервер будет ждать вечно, а если закроет соединение сервер придется ждать TIME_WAIT, которое по умолчанию равно 2 минутам. Использование UDP позволяет этого избежать.
## map g_users
Публичный словарь users, ключами котрого яаляются никнеймы, а элементами структуры user, служит средством свзяи между TCPSendThread() и TCPRecvThread().
## struct user
В структуре user храниться std::condition_variable и std::mutex, которые в последушем используются для связи TCPSendTread() пользователя, которому эта структура принадлжит и TCPRecvThread() пользователся, который хочет отправить сообшение. str_data, для передпчи данных в формате std::string между двумя потоками. shut_down для передачи булевой информации - сигнала о завершении работы потока.
## TCPRecvThread()
Для каждого пользователмя создается поток с этой функцией. В этой функции сервер принимает данные от клиента. После подключения к клиенту и отправки пользователем сообщения в первую очередь происходит получение общего закрытого ключа между пользователями при помощи DH протокола. Сервер в этой цепи участвует лишь как передающее звено и не обрабатывает и не изменяет информацию. Алгоритм работы этой функции таков:

1)Сервер ожидает первый набор данных, состоящий из набора простых чисел и публичных ключей, от клиента;

2)Запись в поле str_data в sers, соответствующие аддресату и уведомление TCPSendThread() адресата при помощи std::condition_varible, которая была
получина при помощи users[(никнейм аддрсата)];

3)TCPRecvThread() ожидает уведомления от TCPSendThread();

4)Сервер отправляет отправителю публичные ключи аддресата, полученные от TCPSendThread(), (к этому моменту и получатель и аддресат получили общий
закрытый ключ шифрования AES);

5)Сервер ожидвет отправки отправителем зашифрованного сообщения;

6)Уведомление TCPSendThread() и передача сообщения так же, как и в шаге 2;

7)Ожидание квудомления от TCPSendThread() с информацией успешно ли дошло сообщение;

8)Отправка отправителю информации успешно ли дошло сообщение или же произошла ошибка;

На этои моменте цикл начинается заново
## TCPSendThread()
Эта функция так же создается в качестве потока для каждого пользователся, как и TCPRecvThread(). Служит для отправки сообщений клиенту.
Алгоритм работы:

1)Ожидание уведомления от TCPRecvThread();

2)Отправка аддресату данных из str_data(набор прочтых чисел, набор публичных ключей);

3)Получение данных от аддресата(набор открытых ключей);

4)Уведомление TCPRecvThread() и передача открытых ключей, как в TCPRecvThread()(шаг 2);

5)Ожидание уведомления от TCPRecvThread();

6)Отправка сообщения аддресату с зашифрованным сообщением;

7)Уведомление TCPRecvThread() и передача информации удачно ли дошло сообщение и если нет информации об ошибке;

Цикл начинается заново
## Связь между TCPRecvThread() и TCPSendThread()
Свзязь между двумя потоками осуществляется при помощи std::condition_variable, которая хранится в struct user. Для каждого потока TCPSendThread() создается уникальная std::condition_variable.
# Client
## main()
Сперва в клиенте создается UDP соккет и отправялется на сервер никней пользователя, далее от сервера приходит два порта - для получения и отправки сообщений соответственно. Отправка сообщений происходит в основном потоке, а получение в потоке listenThread().
## connectToTCP()
Создает и подключает TCP соккет. В качестве аргумента принимает номер порта и возвращает подключенный соккет.
## getPorts() 
Создает UDP соккет и получает от сервера номера портов для отправки и получения сообщений. Возвращает список, сосотоящий из двух портов.
## toFixedLength()
Используется, чтобы заполнить строку символами(@) до определенной длины. Необходимо это для того, чтобы, если длина сообщения менее определенной длины, соккет будет продолжать ожидать данные, эта функция позволяет этого избежать. В качестве аргументов принимает строку длину строки.
## getAESTo()
Получает общий с аддресатом закрытый ключ шифрования AES, путем использования DH протокола. Используется при отправке сообщений. Алгоритм работы:

1)Генерируется набор простых чисел;

2)Генерируется набор открытых ключей;

3)Набор и ключи отправляются на сервер;

4)С сервера приходят открытые ключи аддресата;

5)С помошью полученных данных генерируется ключ шифрования AES;
## getAESFrom()
Получает общий с аддресатом закрытый ключ шифрования AES, путем использования DH протокола. Используется при получении сообщений. Алгоритм работы:

1)С сервера приходят никнейм отправителя, набор простых чичел и набор открытых ключей

2)Генерируются набор открытых ключей и AES ключ шифрования

3)Набор открытых ключей отправляется на сервер
## sendMessage()
В качестве аргумента подается TCP соккет, никнейм пользователя, кому необходимо отправить сообщение, и само сообщение. В начале устанавливается ключ шифрования AES c помощью getAESTo(),  после отправялется зашифрованное сообщение и с сервера приходит ответ, удачно ли дошло сообщение.
## listenThread()
Клиент создает поток listenThread() сразу после получения соответствующего порта от сервера. В этом потоке клиент подключается к серверу и ожидает получения данных от сервера. Когда с сервера приходит первый набор данных, клиент получает ключ шифрования от getAESFrom(), дешифрует сообщение и выводит его на экран.
## toPCW()
Используется для перевода информации типа std::string в PCW, что необходимо для правильного указания ip аддреса сервера. Единственная функция, взятая из интернета, остальные сревер и клиент написаны с нуля.
# Общие функции
### fromSet() и toSet()
Используются для получения из информации типа std::string массива чисел, который будет в последущем использоваться для получени общего ключа шифрования AES и перевода массива в std::string соответсвенно
# В планах
1) Добавить регистрацию пользрвателей и БД SQL с данными пользователей - на 4
2) Добавить сохранение сообщений на устройство пользователя - на 4
3) Добавить шифроование чата, вместо щифрования кадого сообщения по отдельности - на 4
4) Добавить много поточность в main() на сервере - (опционально) на 4
5) Добавить UI (скорее всего на windowsAPI) - на 5
