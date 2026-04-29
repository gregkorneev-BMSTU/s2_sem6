#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "Account.h"
#include "Bank.h"
#include "Client.h"
#include "CreditAccount.h"
#include "DepositAccount.h"
#include "PendingQueue.h"
#include "SavingsAccount.h"

namespace {

std::string formatMoney(double value) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(2) << value << " RUB";
    return out.str();
}

void printLine(char ch = '=', int width = 72) {
    for (int i = 0; i < width; ++i) {
        std::cout << ch;
    }
    std::cout << '\n';
}

void printSection(const std::string& title) {
    std::cout << '\n';
    printLine('=');
    std::cout << title << '\n';
    printLine('=');
}

void printDepartments(const Bank& bank) {
    const MyContainer<Department>& departments = bank.getDepartments();

    std::cout << "Отделения банка:\n";
    if (departments.empty()) {
        std::cout << "  пока нет ни одного отделения\n";
        return;
    }

    std::size_t index = 1;
    for (auto it = departments.begin(); it != departments.end(); ++it) {
        std::cout << "  " << index << ". " << it->getName()
                  << " - " << it->getAddress() << '\n';
        ++index;
    }
}

void printAccountSummary(const Account& account) {
    std::cout << "Счет " << account.getNumber()
              << " (" << account.getDisplayType() << ")\n";
    std::cout << "  Владелец: " << account.getOwnerName() << '\n';
    std::cout << "  Баланс: " << formatMoney(account.getBalance()) << '\n';
    std::cout << "  Особенность: " << account.getSpecialInfo() << '\n';
    std::cout << "  Операций в истории: " << account.getHistory().size() << '\n';
}

void printLogMessage(const std::string& text, int step, double amount) {
    std::cout << "[Обычная функция] Шаг " << step << ": " << text;
    if (amount > 0) {
        std::cout << " Сумма: " << formatMoney(amount);
    }
    std::cout << '\n';
}

void runTransfer(Bank* bank, Account* from, Account* to, double amount) {
    bool success = bank->transfer(*from, *to, amount);

    std::cout << "[Обычная функция] Перевод " << formatMoney(amount)
              << " со счета " << from->getNumber()
              << " на счет " << to->getNumber();
    if (success) {
        std::cout << " выполнен.\n";
    } else {
        std::cout << " не выполнен.\n";
    }
}

class DemoReporter {
public:
    void printState(const char* title,
                    const Bank* bank,
                    const Client* client,
                    const Account* savings,
                    const Account* credit,
                    const Account* deposit) const {
        printSection(title);
        std::cout << "Клиент: " << client->getFullName() << '\n';
        std::cout << "Всего счетов у клиента: " << client->getAccounts().size() << '\n';
        std::cout << "Суммарный баланс клиента: " << formatMoney(client->totalBalance()) << '\n';
        std::cout << "Количество отделений банка: " << bank->getDepartments().size() << '\n';
        printDepartments(*bank);
        printLine('-');
        printAccountSummary(*savings);
        printLine('-');
        printAccountSummary(*credit);
        printLine('-');
        printAccountSummary(*deposit);
    }
};

void setupDomain(Bank& bank,
                 Client& client,
                 SavingsAccount& savings,
                 CreditAccount& credit,
                 DepositAccount& deposit) {
    bank.addDepartment("Центральное", "Москва, ул. Бауманская, 5");
    bank.addDepartment("Северное", "Москва, ул. Лобненская, 12");

    bank.registerClient(client);
    client.addAccount(&savings);
    client.addAccount(&credit);
    client.addAccount(&deposit);
}

} // namespace

int main() {
    printSection("СЕМИНАР 6. ОЧЕРЕДЬ ОТЛОЖЕННЫХ ВЫЗОВОВ");

    Bank bank("BMSTU Bank");
    Client client(1, "Алиса Иванова");
    SavingsAccount savings("ACC-1001", client.getFullName(), 15000, 6);
    CreditAccount credit("ACC-2001", client.getFullName(), 2000, 10000);
    DepositAccount deposit("ACC-3001", client.getFullName(), 40000, 12);
    DemoReporter reporter;

    setupDomain(bank, client, savings, credit, deposit);

    reporter.printState("Начальное состояние предметной области",
                        &bank, &client, &savings, &credit, &deposit);

    printSection("ОСНОВНАЯ ОЧЕРЕДЬ");
    {
        PendingQueue queue;

        queue.enqueue(printLogMessage,
                      std::string("Задачи добавлены в очередь, но пока не выполняются."),
                      1,
                      2500.0);
        queue.enqueue(&Bank::addDepartment,
                      &bank,
                      std::string("Онлайн-офис"),
                      std::string("Москва, ул. Электрозаводская, 1"));
        queue.enqueue(&DemoReporter::printState,
                      &reporter,
                      "Состояние после добавления нового отделения",
                      &bank,
                      &client,
                      &savings,
                      &credit,
                      &deposit);
        queue.enqueue(&Account::deposit,
                      &savings,
                      2500.0,
                      std::string("Отложенное пополнение"));
        queue.enqueue(runTransfer,
                      &bank,
                      static_cast<Account*>(&savings),
                      static_cast<Account*>(&credit),
                      1000.0);
        queue.enqueue(&SavingsAccount::addMonthlyInterest, &savings);
        queue.enqueue(&DepositAccount::unlock, &deposit);
        queue.enqueue(&Account::withdraw,
                      &deposit,
                      500.0,
                      std::string("Снятие после разблокировки"));
        queue.enqueue(&DemoReporter::printState,
                      &reporter,
                      "Состояние после выполнения оставшихся задач",
                      &bank,
                      &client,
                      &savings,
                      &credit,
                      &deposit);

        std::cout << "После enqueue задачи только поставлены в очередь.\n";
        std::cout << "Количество задач в очереди: " << queue.size() << '\n';
        std::cout << "До запуска очереди состояние банка и счетов не изменилось.\n";
        reporter.printState("Проверка перед выполнением очереди",
                            &bank, &client, &savings, &credit, &deposit);

        printSection("ВЫЗОВ run_one()");
        queue.run_one();
        std::cout << "Количество задач после run_one(): " << queue.size() << '\n';
        std::cout << "Выполнилась только первая задача, поэтому предметная область еще не изменилась.\n";
        reporter.printState("Состояние после run_one()",
                            &bank, &client, &savings, &credit, &deposit);

        printSection("ВЫЗОВ run_all()");
        queue.run_all();
        std::cout << "Количество задач после run_all(): " << queue.size() << '\n';
        std::cout << "Все оставшиеся задачи выполнены последовательно, а из очереди удалены.\n";
        std::cout << "При выходе из блока деструктор очереди покажет, что невыполненных задач не осталось.\n";
    }

    printSection("ОЧЕРЕДЬ С НЕВЫПОЛНЕННЫМИ ЗАДАЧАМИ");
    {
        PendingQueue droppedQueue;

        droppedQueue.enqueue(printLogMessage,
                             std::string("Эта задача останется в очереди и не выполнится."),
                             99,
                             0.0);
        droppedQueue.enqueue(&Bank::addDepartment,
                             &bank,
                             std::string("Временное отделение"),
                             std::string("Этот адрес не должен появиться в списке"));

        std::cout << "Создана отдельная очередь.\n";
        std::cout << "Количество задач перед уничтожением очереди: "
                  << droppedQueue.size() << '\n';
        std::cout << "Сейчас мы выйдем из блока без run_one() и run_all().\n";
    }

    reporter.printState("Проверка после уничтожения второй очереди",
                        &bank, &client, &savings, &credit, &deposit);

    printSection("ДЕМОНСТРАЦИЯ ЗАВЕРШЕНА");
    return 0;
}
