#include "MainWindow.hpp"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) 
{
    setWindowTitle("Bank System Client");
    resize(400, 300);

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
    logoutBtn = new QPushButton("Logout");

    layout->addWidget(welcomeLabel);
    layout->addWidget(balanceBtn);
    layout->addWidget(withdrawBtn);
    layout->addWidget(depositBtn);
    layout->addStretch();
    layout->addWidget(logoutBtn);

    connect(balanceBtn, &QPushButton::clicked, this, &MainWindow::showBalance);
    connect(withdrawBtn, &QPushButton::clicked, this, &MainWindow::handleWithdraw);
    connect(depositBtn, &QPushButton::clicked, this, &MainWindow::handleDeposit);
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
        QMessageBox::information(this, "Success", "Registration successful! You can now log in.");
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

void MainWindow::handleLogout() 
{
    stackedWidget->setCurrentWidget(loginPage);
}