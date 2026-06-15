#include "MainWindow.hpp"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) 
{
    setWindowTitle("Bank System Client");
    resize(800, 600);

    bankClient = new BankClient();
    
    if (!bankClient->discoverAdmin() || !bankClient->connectToServer()) 
    {
        QMessageBox::critical(this, "Connection Error", "Could not connect to the Bank Server!");
    }

    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    setupLoginPage();
    setupDashboardPage();

    stackedWidget->setCurrentWidget(loginPage);
}

MainWindow::~MainWindow() 
{
    delete bankClient;
}

void MainWindow::setupLoginPage() 
{
    loginPage = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(loginPage);

    QLabel* title = new QLabel("<h2>Welcome to the Bank</h2>");
    title->setAlignment(Qt::AlignCenter);

    userEdit = new QLineEdit();
    userEdit->setPlaceholderText("Username");

    passEdit = new QLineEdit();
    passEdit->setPlaceholderText("Password");
    passEdit->setEchoMode(QLineEdit::Password);

    loginBtn = new QPushButton("Login");
    registerBtn = new QPushButton("Register");

    layout->addStretch();
    layout->addWidget(title);
    layout->addWidget(userEdit);
    layout->addWidget(passEdit);
    layout->addWidget(loginBtn);
    layout->addWidget(registerBtn);
    layout->addStretch();

    connect(loginBtn, &QPushButton::clicked, this, &MainWindow::handleLogin);
    connect(registerBtn, &QPushButton::clicked, this, &MainWindow::handleRegister);

    stackedWidget->addWidget(loginPage);
}

void MainWindow::setupDashboardPage() 
{
    dashboardPage = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(dashboardPage);

    welcomeLabel = new QLabel("<h3>Dashboard</h3>");
    welcomeLabel->setAlignment(Qt::AlignCenter);

    balanceBtn = new QPushButton("Check Balance");
    withdrawBtn = new QPushButton("Withdraw");
    depositBtn = new QPushButton("Deposit");
    transferBtn = new QPushButton("Transfer");
    debtsBtn = new QPushButton("Show Debts");
    getLoanBtn = new QPushButton("Get Loan");
    payLoanBtn = new QPushButton("Pay Loan");
    logoutBtn = new QPushButton("Logout");

    layout->addWidget(welcomeLabel);
    layout->addWidget(balanceBtn);
    layout->addWidget(withdrawBtn);
    layout->addWidget(depositBtn);
    layout->addWidget(transferBtn);
    layout->addWidget(debtsBtn);
    layout->addWidget(getLoanBtn);
    layout->addWidget(payLoanBtn);
    layout->addStretch();
    layout->addWidget(logoutBtn);

    connect(balanceBtn, &QPushButton::clicked, this, &MainWindow::showBalance);
    connect(withdrawBtn, &QPushButton::clicked, this, &MainWindow::handleWithdraw);
    connect(depositBtn, &QPushButton::clicked, this, &MainWindow::handleDeposit);
    connect(transferBtn, &QPushButton::clicked, this, &MainWindow::handleTransfer);
    connect(debtsBtn, &QPushButton::clicked, this, &MainWindow::handleShowDebts);
    connect(getLoanBtn, &QPushButton::clicked, this, &MainWindow::handleGetLoan);
    connect(payLoanBtn, &QPushButton::clicked, this, &MainWindow::handlePayLoan);
    connect(logoutBtn, &QPushButton::clicked, this, &MainWindow::handleLogout);

    stackedWidget->addWidget(dashboardPage);
}

void MainWindow::handleLogin() 
{
    std::string user = userEdit->text().toStdString();
    std::string pass = passEdit->text().toStdString();

    if (bankClient->login(user, pass)) 
    {
        welcomeLabel->setText("<h3>Welcome, " + QString::fromStdString(user) + "</h3>");
        stackedWidget->setCurrentWidget(dashboardPage);
        userEdit->clear();
        passEdit->clear();
    } 
    else 
    {
        QMessageBox::warning(this, "Login Failed", "Invalid username or password.");
    }
}

void MainWindow::handleRegister() 
{
    std::string user = userEdit->text().toStdString();
    std::string pass = passEdit->text().toStdString();

    if (pass.length() < 8) 
    {
        QMessageBox::warning(this, "Error", "Password must be at least 8 characters.");
        return;
    }

    if (bankClient->registerUser(user, pass)) 
    {
        QMessageBox::information(this, "Success", "Registration successful!");
        welcomeLabel->setText("<h3>Welcome, " + QString::fromStdString(user) + "</h3>");
        stackedWidget->setCurrentWidget(dashboardPage); 
        userEdit->clear();
        passEdit->clear();
    } 
    else 
    {
        QMessageBox::warning(this, "Error", "Username already exists or server error.");
    }
}

void MainWindow::showBalance() 
{
    std::string balance = bankClient->getAccountBalance();
    QMessageBox::information(this, "Account Balance", QString::fromStdString(balance));
}

void MainWindow::handleWithdraw() 
{
    bool ok;
    int amount = QInputDialog::getInt(this, "Withdraw", "Enter amount:", 0, 1, 100000, 1, &ok);
    if (ok) 
    {
        std::string response = bankClient->withdraw(amount);
        QMessageBox::information(this, "Transaction Result", QString::fromStdString(response));
    }
}

void MainWindow::handleDeposit() 
{
    bool ok;
    int amount = QInputDialog::getInt(this, "Deposit", "Enter amount:", 0, 1, 100000, 1, &ok);
    if (ok) 
    {
        std::string response = bankClient->deposit(amount);
        QMessageBox::information(this, "Transaction Result", QString::fromStdString(response));
    }
}
void MainWindow::handleShowDebts() 
{
    std::string debts = bankClient->getDebts();
    QMessageBox::information(this, "Your Debts", QString::fromStdString(debts));
}

void MainWindow::handleGetLoan() 
{
    bool okAmount;
    int amount = QInputDialog::getInt(this, "Get Loan", "Enter loan amount:", 0, 1, 10000, 1, &okAmount);
    if (okAmount) 
    {
        bool okDays;
        int days = QInputDialog::getInt(this, "Get Loan", "Days to repay:", 30, 1, 180, 1, &okDays);
        if (okDays) 
        {
            std::string response = bankClient->getLoan(amount, days);
            QMessageBox::information(this, "Loan Status", QString::fromStdString(response));
        }
    }
}

void MainWindow::handlePayLoan() 
{
    bool ok;
    int amount = QInputDialog::getInt(this, "Pay Loan", "Enter amount to pay back:", 0, 1, 100000, 1, &ok);
    if (ok) 
    {
        std::string response = bankClient->payLoan(amount);
        QMessageBox::information(this, "Payment Status", QString::fromStdString(response));
    }
}

void MainWindow::handleTransfer()
{
    bool ok;
    QString qname = QInputDialog::getText(this, "Transfer", "Enter name:", QLineEdit::Normal, "", &ok);
    if(ok && !qname.isEmpty())
    {
        std::string name = qname.toStdString();
        std::string response = bankClient->transferFindDest(name);
        if(response == "User not found") QMessageBox::information(this, "Transfer status", QString::fromStdString(response));
        else
        {
            bool okAmount;
            int amount = QInputDialog::getInt(this, "Transfer amount", "Enter amount to transfer:", 0, 1, 100000, 1, &okAmount);
            if (okAmount)
            {
                std::string response = bankClient->transferSendMoney(name,amount);
                QMessageBox::information(this, "Transfer status", QString::fromStdString(response));
            }
        }
    }
}

void MainWindow::handleLogout() 
{
    bankClient->disconnect();
    if(!bankClient->connectToServer()) 
    {
        QMessageBox::critical(this, "Connection Error", "Lost connection to the server during logout.");
    }
    userEdit->clear();
    passEdit->clear();
    stackedWidget->setCurrentWidget(loginPage);
}